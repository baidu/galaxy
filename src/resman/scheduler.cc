// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"

#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <deque>
#include <limits>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include "timer.h"

DECLARE_int64(sched_interval);
DECLARE_int64(container_group_gc_check_interval);

namespace baidu {
namespace galaxy {
namespace sched {

const int sMaxPort = 60000;
const int sMinPort = 1000;
const std::string kDynamicPort = "dynamic";

Agent::Agent(const AgentEndpoint& endpoint,
            int64_t cpu,
            int64_t memory,
            const std::map<DevicePath, VolumInfo>& volums,
            const std::set<std::string>& tags,
            const std::string& pool_name) {
    endpoint_ = endpoint;
    cpu_total_ = cpu;
    cpu_assigned_ = 0;
    memory_total_ = memory;
    memory_assigned_ = 0;
    volum_total_ = volums;
    port_total_ = sMaxPort - sMinPort + 1;
    tags_ = tags;
    pool_name_ = pool_name;
}

void Agent::SetAssignment(int64_t cpu_assigned,
                          int64_t memory_assigned,
                          const std::map<DevicePath, VolumInfo>& volum_assigned,
                          const std::set<std::string> port_assigned,
                          const std::map<ContainerId, Container::Ptr>& containers) {
    cpu_assigned_ = cpu_assigned;
    memory_assigned_ = memory_assigned;
    volum_assigned_ =  volum_assigned;
    port_assigned_ = port_assigned;
    containers_ =  containers;
    container_counts_.clear();
    BOOST_FOREACH(const ContainerMap::value_type& pair, containers) {
        const Container::Ptr& container = pair.second;
        container_counts_[container->container_group_id] += 1;
        container->allocated_agent = endpoint_;
    }
}


bool Agent::TryPut(const Container* container, ResourceError& err) {
    if (!container->require->tag.empty() &&
        tags_.find(container->require->tag) == tags_.end()) {
        err = proto::kTagMismatch;
        return false;
    }
    if (container->require->pool_names.find(pool_name_) 
        == container->require->pool_names.end()) {
        err = proto::kPoolMismatch;
        return false;
    }

    if (container->require->max_per_host > 0) {
        ContainerGroupId container_group_id = container->container_group_id;
        std::map<ContainerGroupId, int>::iterator it = container_counts_.find(container_group_id);
        if (it != container_counts_.end()) {
            int cur_counts = it->second;
            if (cur_counts >= container->require->max_per_host) {
                err = proto::kTooManyPods;
                return false;
            } 
        }
    }

    if (container->require->CpuNeed() + cpu_assigned_ > cpu_total_) {
        err = proto::kNoCpu;
        return false;
    }
    if (container->require->MemoryNeed() + memory_assigned_ > memory_total_) {
        err = proto::kNoMemory;
        return false;
    }

    int64_t size_ramdisk = 0;
    const std::vector<proto::VolumRequired>& volums = container->require->volums;
    std::vector<proto::VolumRequired> volums_no_ramdisk;

    BOOST_FOREACH(const proto::VolumRequired& v, volums) {
        if (v.medium() == proto::kTmpfs) {
            size_ramdisk += v.size();
        } else {
            volums_no_ramdisk.push_back(v);
        }
    }

    if (size_ramdisk + memory_assigned_ + container->require->MemoryNeed()> memory_total_) {
        err = proto::kNoMemoryForTmpfs;
        return false;
    }

    std::vector<DevicePath> devices;
    if (!SelectDevices(volums_no_ramdisk, devices)) {
        err = proto::kNoDevice;
        return false;
    }

    if (container->require->ports.size() + port_assigned_.size() 
        > port_total_) {
        err = proto::kNoPort;
        return false;
    }

    const std::vector<proto::PortRequired> ports = container->require->ports;
    std::vector<std::string> ports_free;
    if (!SelectFreePorts(ports, ports_free)) {
        err = proto::kPortConflict;
        return false;
    }
    return true;
}

void Agent::Put(Container::Ptr container) {
    assert(container->status == kContainerPending);
    assert(container->allocated_agent.empty());
    //cpu 
    cpu_assigned_ += container->require->CpuNeed();
    assert(cpu_assigned_ <= cpu_total_);
    //memory
    memory_assigned_ += container->require->MemoryNeed();
    int64_t size_ramdisk = 0;
    std::vector<proto::VolumRequired> volums_no_ramdisk;
    BOOST_FOREACH(const proto::VolumRequired& v, container->require->volums) {
        if (v.medium() == proto::kTmpfs) {
            size_ramdisk += v.size();
        } else {
            volums_no_ramdisk.push_back(v);
        }
    }
    memory_assigned_ += size_ramdisk;
    assert(memory_assigned_ <= memory_total_);
    //volums
    std::vector<DevicePath> devices;
    if (SelectDevices(volums_no_ramdisk, devices)) {
        for (size_t i = 0; i < devices.size(); i++) {
            const DevicePath& device_path = devices[i];
            const proto::VolumRequired& volum = volums_no_ramdisk[i];
            volum_assigned_[device_path].size += volum.size();
            VolumInfo volum_info;
            volum_info.medium = volum.medium();
            volum_info.size = volum.size();
            volum_info.exclusive = volum.exclusive();
            container->allocated_volums.push_back(
                std::make_pair(device_path, volum_info)
            );
            if (volum.exclusive()) {
                volum_assigned_[device_path].exclusive = true;
            }
        }
    }
    //ports
    std::vector<std::string> ports_free;
    if (SelectFreePorts(container->require->ports, ports_free)) {
        for (size_t i = 0; i < ports_free.size(); i++) {
            container->allocated_ports.push_back(ports_free[i]);
            port_assigned_.insert(ports_free[i]);
        }
    }
    //put on this agent succesfully
    container->allocated_agent = endpoint_;
    container->last_res_err = proto::kResOk;
    containers_[container->id] = container;
    container_counts_[container->container_group_id] += 1;
}

bool Agent::SelectFreePorts(const std::vector<proto::PortRequired>& ports_need,
                            std::vector<std::string>& ports_free) {
    bool has_determinate_port = false;
    bool has_dynamic_port = false;
    int max_port = 0;
    int min_port = std::numeric_limits<int>::max();
    int dynamic_port_count = 0;
    int determinate_port_count = 0;
    std::deque<std::string> free_random_ports;
    BOOST_FOREACH(const proto::PortRequired& port, ports_need) {
        if (port.port() != kDynamicPort) {
            has_determinate_port = true;
            std::stringstream ss;
            int n_port;
            ss << port.port();
            ss >> n_port;
            max_port = std::max(max_port, n_port);
            min_port = std::min(min_port, n_port);
            determinate_port_count++;
            if (port_assigned_.find(port.port()) != port_assigned_.end()) {
                return false;
            }
        } else {
            has_dynamic_port = true;
            dynamic_port_count++;
        }
    }
    if (has_determinate_port && (max_port - min_port + 1 > determinate_port_count) ) {
        LOG(WARNING) << "the port range is not continuous: " << min_port << max_port;
        return false;
    }
    if (has_dynamic_port && has_determinate_port) {
        int start_port = max_port + 1;
        for (int x = start_port; x < (start_port + dynamic_port_count); x++) {
            std::stringstream ss;
            ss << x;
            std::string s_port = ss.str();
            if (port_assigned_.find(s_port) != port_assigned_.end()) {
                return false;
            } else {
                free_random_ports.push_back(s_port);
            }
        }
    } else if (!has_determinate_port && has_dynamic_port) {
        for (int start_port = sMinPort; start_port <= sMaxPort; start_port += dynamic_port_count) {
            free_random_ports.clear();
            for (int x = start_port; x < (start_port + dynamic_port_count); x++) {
                std::stringstream ss;
                ss << x;
                std::string s_port = ss.str();
                if (port_assigned_.find(s_port) != port_assigned_.end()) {
                    break;
                } else {
                    free_random_ports.push_back(s_port);
                }
            }
            if ((int)free_random_ports.size() == dynamic_port_count) {
                break;
            }
        }
    }
    
    if (has_dynamic_port &&  ((int)free_random_ports.size() != dynamic_port_count) ) {
        return false;
    }

    BOOST_FOREACH(const proto::PortRequired& port, ports_need) {
        if (port.port() != kDynamicPort) {
            ports_free.push_back(port.port());
        } else {
            const std::string& dyn_port = free_random_ports.front();
            ports_free.push_back(dyn_port);
            free_random_ports.pop_front();
        }
    }
    return true;
}

void Agent::Evict(Container::Ptr container) {
    if (containers_.find(container->id) == containers_.end()) {
        LOG(WARNING) << "invalid evict, no such container:" << container->id;
        return;
    }
    //cpu 
    cpu_assigned_ -= container->require->CpuNeed();
    assert(cpu_assigned_ >= 0);
    //memory
    memory_assigned_ -= container->require->MemoryNeed();
    assert(memory_assigned_ >= 0);
    int64_t size_ramdisk = 0;
    std::vector<proto::VolumRequired> volums_no_ramdisk;
    BOOST_FOREACH(const proto::VolumRequired& v, container->require->volums) {
        if (v.medium() == proto::kTmpfs) {
            size_ramdisk += v.size();
        } else {
            volums_no_ramdisk.push_back(v);
        }
    }
    memory_assigned_ -= size_ramdisk;
    assert(memory_assigned_ >= 0);
    //volums
    for (size_t i = 0; i < container->allocated_volums.size(); i++) {
        const std::pair<DevicePath, VolumInfo>& tup = container->allocated_volums[i];
        const std::string& device_path = tup.first;
        const VolumInfo& volum_info = tup.second;
        volum_assigned_[device_path].size -= volum_info.size;
        if (volum_info.exclusive) {
            volum_assigned_[device_path].exclusive = false;
        }
    }
    BOOST_FOREACH(const std::string& port, container->allocated_ports) {
        port_assigned_.erase(port);
    }
    containers_.erase(container->id);
    container_counts_[container->container_group_id] -= 1;
    if (container_counts_[container->container_group_id] <= 0) {
        container_counts_.erase(container->container_group_id);
    }
}

bool Agent::SelectDevices(const std::vector<proto::VolumRequired>& volums,
                          std::vector<DevicePath>& devices) {
    typedef std::map<DevicePath, VolumInfo> VolumMap;
    VolumMap volum_free;
    BOOST_FOREACH(const VolumMap::value_type& pair, volum_total_) {
        const DevicePath& device_path = pair.first;
        const VolumInfo& volum_info = pair.second;
        if (volum_assigned_.find(device_path) == volum_assigned_.end()) {
            volum_free[device_path] = volum_info;
        } else {
            if (!volum_assigned_[device_path].exclusive) {
                volum_free[device_path] =  volum_info;
                volum_free[device_path].size -= volum_assigned_[device_path].size;
            }
        }
    }
    return RecurSelectDevices(0, volums, volum_free, devices);
}

bool Agent::RecurSelectDevices(size_t i, const std::vector<proto::VolumRequired>& volums,
                               std::map<DevicePath, VolumInfo>& volum_free,
                               std::vector<DevicePath>& devices) {
    if (i >= volums.size()) {
        if (devices.size() == volums.size()) {
            return true;
        } else {
            return false;
        }
    }
    const proto::VolumRequired& volum_need = volums[i];
    typedef std::map<DevicePath, VolumInfo> VolumMap;
    BOOST_FOREACH(VolumMap::value_type& pair, volum_free) {
        const DevicePath& device_path = pair.first;
        VolumInfo& volum_info = pair.second;
        if (volum_info.exclusive || volum_need.size() > volum_info.size) {
            continue;
        }
        if (volum_info.medium != volum_need.medium()) {
            continue;
        }
        volum_info.size -= volum_need.size();
        volum_info.exclusive = volum_need.exclusive();
        devices.push_back(device_path);
        if (RecurSelectDevices(i + 1, volums, volum_free, devices)) {
            return true;
        } else {
            volum_info.size += volum_need.size();
            volum_info.exclusive = false;
            devices.pop_back();
        }
    }
    return false;
}


Scheduler::Scheduler() : stop_(true) {
    srand(time(NULL));
}

void Scheduler::SetRequirement(Requirement::Ptr require, 
                               const proto::ContainerDescription& container_desc) {
    require->tag = container_desc.tag();
    for (int j = 0; j < container_desc.pool_names_size(); j++) {
        require->pool_names.insert(container_desc.pool_names(j));
    }
    require->max_per_host = container_desc.max_per_host();
    for (int j = 0; j < container_desc.cgroups_size(); j++) {
        const proto::Cgroup& cgroup = container_desc.cgroups(j);
        require->cpu.push_back(cgroup.cpu());
        require->memory.push_back(cgroup.memory());
        for (int k = 0; k < cgroup.ports_size(); k++) {
            require->ports.push_back(cgroup.ports(k));
        }
    }
    require->volums.push_back(container_desc.workspace_volum());
    for (int j = 0; j < container_desc.data_volums_size(); j++) {
        require->volums.push_back(container_desc.data_volums(j));
    }
    require->version = container_desc.version();
}

void Scheduler::AddAgent(Agent::Ptr agent, const proto::AgentInfo& agent_info) {
    MutexLock locker(&mu_);
    
    int64_t cpu_assigned = 0;
    int64_t memory_assigned = 0;
    std::map<DevicePath, VolumInfo> volum_assigned;
    std::set<std::string> port_assigned;
    std::map<ContainerId, Container::Ptr> containers;

    for (int i = 0; i < agent_info.container_info_size(); i++) {
        const proto::ContainerInfo& container_info = agent_info.container_info(i);
        if (container_info.status() != kContainerReady) {
            continue;
        }
        if (container_groups_.find(container_info.group_id()) == container_groups_.end()) {
            LOG(WARNING) << "add agent exception, no such container group:" << container_info.group_id();
            continue;
        }
        ContainerGroup::Ptr& container_group = container_groups_[container_info.group_id()];
        if (container_group->terminated) {
            LOG(WARNING) << "ignore killed container group:" << container_info.group_id();
            continue;
        }
        Container::Ptr container(new Container());

        if (container_group->containers.find(container_info.id())
            != container_group->containers.end()) {
            Container::Ptr& exist_container = container_group->containers[container_info.id()];
            if (exist_container->status != kContainerReady) {
                ChangeStatus(exist_container, kContainerTerminated);
                container = exist_container;
            } else {
                LOG(WARNING) << "this container already exist:" << container_info.id();
                continue;
            }
        }
        
        Requirement::Ptr require(new Requirement());    
        const proto::ContainerDescription& container_desc = container_info.container_desc();    
        SetRequirement(require, container_desc);

        container->id = container_info.id();
        container->container_group_id = container_info.group_id();
        container->priority = container_desc.priority();
        container->status = container_info.status();
        container->require = require;
        cpu_assigned += require->CpuNeed();
        memory_assigned += require->MemoryNeed();
        for (int j = 0; j < container_desc.cgroups_size(); j++) {
            const proto::Cgroup& cgroup = container_desc.cgroups(j);
            for (int k = 0; k < cgroup.ports_size(); k++) {
                container->allocated_ports.push_back(cgroup.ports(k).real_port());
                port_assigned.insert(cgroup.ports(k).real_port());
            }
        }

        VolumInfo workspace_volum;
        workspace_volum.medium = container_desc.workspace_volum().medium();
        workspace_volum.size = container_desc.workspace_volum().size();
        workspace_volum.exclusive = container_desc.workspace_volum().exclusive();
        std::string work_path = container_desc.workspace_volum().source_path();
        if (workspace_volum.medium != proto::kTmpfs) {
            container->allocated_volums.push_back(
                std::make_pair(work_path, workspace_volum)
            );
            volum_assigned[work_path].size += workspace_volum.size;
            volum_assigned[work_path].medium = workspace_volum.medium;
        } else {
            memory_assigned +=  workspace_volum.size;
        }
        if (workspace_volum.exclusive) {
            volum_assigned[work_path].exclusive = true;
        }
        for (int j = 0; j < container_desc.data_volums_size(); j++) {
            VolumInfo data_volum;
            data_volum.medium = container_desc.data_volums(j).medium();
            data_volum.size = container_desc.data_volums(j).size();
            if (data_volum.medium == proto::kTmpfs) {
                memory_assigned += data_volum.size;
                continue;
            }
            data_volum.exclusive = container_desc.data_volums(j).exclusive();
            std::string device_path = container_desc.data_volums(j).source_path();
            container->allocated_volums.push_back(
                std::make_pair(device_path, data_volum)
            );
            volum_assigned[device_path].size += data_volum.size;
            volum_assigned[device_path].medium = data_volum.medium;
            if (data_volum.exclusive) {
                volum_assigned[device_path].exclusive = true;
            }
        }
        containers[container->id] = container;
        container_groups_[container->container_group_id]->containers[container->id] = container;
        container->allocated_agent = agent->endpoint_;
        ChangeStatus(container, container->status);
    }
    agent->SetAssignment(cpu_assigned, memory_assigned, volum_assigned,
                         port_assigned, containers);
    agents_[agent->endpoint_] = agent;
}

void Scheduler::RemoveAgent(const AgentEndpoint& endpoint) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it;
    it = agents_.find(endpoint);
    if (it == agents_.end()) {
        return;
    }
    const Agent::Ptr& agent = it->second;
    ContainerMap containers = agent->containers_; //copy
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        ChangeStatus(container, kContainerPending);        
    }
    agents_.erase(endpoint);
}

void Scheduler::AddTag(const AgentEndpoint& endpoint, const std::string& tag) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "add tag fail, no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->tags_.insert(tag);
}

void Scheduler::RemoveTag(const AgentEndpoint& endpoint, const std::string& tag) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "remove tag fail, no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->tags_.erase(tag);
}

void Scheduler::SetPool(const AgentEndpoint& endpoint, const std::string& pool_name) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "set pool fail, no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->pool_name_ = pool_name;
}

ContainerGroupId Scheduler::GenerateContainerGroupId(const std::string& container_group_name) {
    std::string suffix = "";
    BOOST_FOREACH(char c, container_group_name) {
        if (!isalnum(c)) {
            suffix += "_";
        } else {
            suffix += c;
        }
        if (suffix.length() >= 16) {//truncate
            break;
        }
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const time_t seconds = tv.tv_sec;
    struct tm t;
    localtime_r(&seconds, &t);
    std::stringstream ss;
    char time_buf[32] = { 0 };
    ::strftime(time_buf, 32, "%Y%m%d_%H%M%S", &t);
    ss << "job_" << time_buf << "_"
       << random() << "_" << suffix;
    return ss.str();
}

ContainerId Scheduler::GenerateContainerId(const ContainerGroupId& container_group_id, int offset) {
    std::stringstream ss;
    ss << container_group_id << ".pod_" << offset;
    return ss.str();
}

ContainerGroupId Scheduler::Submit(const std::string& container_group_name,
                                   const proto::ContainerDescription& container_desc,
                                   int replica, int priority,
                                   const std::string& user_name) {
    MutexLock locker(&mu_);
    ContainerGroupId container_group_id = GenerateContainerGroupId(container_group_name);
    if (container_groups_.find(container_group_id) != container_groups_.end()) {
        LOG(WARNING) << "container_group id conflict:" << container_group_id;
        return "";
    }
    Requirement::Ptr req(new Requirement());
    SetRequirement(req, container_desc);
    ContainerGroup::Ptr container_group(new ContainerGroup());
    container_group->require = req;
    container_group->id = container_group_id;
    container_group->priority = priority;
    container_group->container_desc = container_desc;
    container_group->replica = replica;
    container_group->name = container_group_name;
    container_group->user_name = user_name;
    container_group->submit_time = common::timer::get_micros();
    for (int i = 0 ; i < replica; i++) {
        Container::Ptr container(new Container());
        container->container_group_id = container_group->id;
        container->id = GenerateContainerId(container_group_id, i);
        container->require = req;
        container->priority = priority;
        container_group->containers[container->id] = container;
        ChangeStatus(container_group, container, kContainerPending);
    }
    container_groups_[container_group_id] = container_group;
    container_group_queue_.insert(container_group);
    return container_group->id;
}

void Scheduler::Reload(const proto::ContainerGroupMeta& container_group_meta) {
    MutexLock lock(&mu_);
    Requirement::Ptr req(new Requirement());
    ContainerGroup::Ptr container_group(new ContainerGroup());

    SetRequirement(req, container_group_meta.desc());
    container_group->require = req;
    container_group->id = container_group_meta.id();
    container_group->priority = container_group_meta.desc().priority();
    container_group->replica = container_group_meta.replica();
    container_group->update_interval = container_group_meta.update_interval();
    container_group->container_desc = container_group_meta.desc();
    container_group->name = container_group_meta.name();
    container_group->user_name = container_group_meta.user_name();
    container_group->submit_time = container_group_meta.submit_time();
    container_group->update_time = container_group_meta.update_time();
    if (container_group_meta.status() == proto::kContainerGroupTerminated) {
        container_group->terminated = true;
    } else {
        container_group->terminated = false;
    }

    container_groups_[container_group->id] = container_group;
    container_group_queue_.insert(container_group);
}

bool Scheduler::Kill(const ContainerGroupId& container_group_id) {
    MutexLock locker(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "unkonw container_group id: " << container_group_id;
        return false;
    }
    ContainerGroup::Ptr container_group = it->second;
    BOOST_FOREACH(ContainerMap::value_type& pair, container_group->containers) {
        Container::Ptr container = pair.second;
        if (container->status == kContainerPending) {
            ChangeStatus(container_group, container, kContainerTerminated);
        } else if (container->status != kContainerTerminated){
            ChangeStatus(container_group, container, kContainerDestroying);
        }
    }
    container_group->terminated = true;
    gc_pool_.AddTask(boost::bind(&Scheduler::CheckContainerGroupGC, this, container_group));
    return true;
}

void Scheduler::CheckContainerGroupGC(ContainerGroup::Ptr container_group) {
    MutexLock locker(&mu_);
    assert(container_group->terminated);
    bool all_container_terminated = true;
    BOOST_FOREACH(ContainerMap::value_type& pair, container_group->containers) {
        Container::Ptr container = pair.second;
        if (container->status != kContainerTerminated) {
            all_container_terminated  = false;
            break;
        }
    }
    if (all_container_terminated) {
        container_groups_.erase(container_group->id);
        container_group_queue_.erase(container_group);
        //after this, all containers wish to be deleted
    } else {
        gc_pool_.DelayTask(FLAGS_container_group_gc_check_interval,
                           boost::bind(&Scheduler::CheckContainerGroupGC, this, container_group));
    }
}

bool Scheduler::ChangeReplica(const ContainerGroupId& container_group_id, int replica) {
    MutexLock locker(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "unkonw container_group id: " << container_group_id;
        return false;
    }
    if (replica < 0) {
        LOG(WARNING) << "ignore invalid replica: " << replica;
        return false;
    }
    ContainerGroup::Ptr container_group = it->second;
    if (container_group->terminated) {
        LOG(WARNING) << "terminated container_group can not be scale up/down";
        return false;
    }
    int current_replica = container_group->Replica();
    if (replica == current_replica) {
        LOG(INFO) << "replica not change, do nothing" ;
    } else if (replica < current_replica) {
        ScaleDown(container_group, replica);
    } else {
        ScaleUp(container_group, replica);
    }
    container_group->replica = replica;
    return true;
}

void Scheduler::ScaleDown(ContainerGroup::Ptr container_group, int replica) {
    mu_.AssertHeld();
    int delta = container_group->Replica() - replica;
    //remove from pending first
    ContainerMap pending_containers = container_group->states[kContainerPending];
    BOOST_FOREACH(ContainerMap::value_type& pair, pending_containers) {
        Container::Ptr container = pair.second;
        ChangeStatus(container_group, container, kContainerTerminated);
        --delta;
        if (delta <= 0) {
            break;
        }
    }
    ContainerStatus all_status[] = {kContainerAllocating, kContainerReady};
    for (size_t i = 0; i < sizeof(all_status) && delta > 0; i++) {
        ContainerStatus st = all_status[i];
        ContainerMap working_containers = container_group->states[st];
        BOOST_FOREACH(ContainerMap::value_type& pair, working_containers) {
            Container::Ptr container = pair.second;
            ChangeStatus(container, kContainerDestroying);
            --delta;
            if (delta <= 0) {
                break;
            }
        }
    }
}

void Scheduler::ScaleUp(ContainerGroup::Ptr container_group, int replica) {
    mu_.AssertHeld();
    for (int i = 0; i < replica; i++) {
        if (container_group->Replica() >= replica) {
            break;
        }
        ContainerId container_id = GenerateContainerId(container_group->id, i);
        Container::Ptr container;
        ContainerMap::iterator it = container_group->containers.find(container_id);
        if (it == container_group->containers.end()) {
            container.reset(new Container());
            container->container_group_id = container_group->id;
            container->id = container_id;
            container->require = container_group->require;
            container_group->containers[container_id] = container;
        } else {
            container = it->second;
        }
        if (container->status != kContainerReady && container->status != kContainerAllocating) {
            ChangeStatus(container_group, container, kContainerPending);            
        }
    }
}

bool Scheduler::ChangeStatus(const ContainerGroupId& container_group_id,
                             const ContainerId& container_id,
                             ContainerStatus new_status) {
    MutexLock lock(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "change status fail, no such container_group:" << container_group_id;
        return false;
    }
    ContainerGroup::Ptr container_group = it->second;
    ContainerMap::iterator container_it = container_group->containers.find(container_id);
    if (container_it == container_group->containers.end()) {
        LOG(WARNING) << "change status fail, no such container: " << container_id;
        return false;
    }
    Container::Ptr container = container_it->second;
    ChangeStatus(container_group, container, new_status);
    return true;
}

void Scheduler::ChangeStatus(Container::Ptr container,
                             ContainerStatus new_status) {
    mu_.AssertHeld();
    ContainerGroupId container_group_id = container->container_group_id;
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "change status fail, no such container_group:" << container_group_id;
        return;
    }
    ContainerGroup::Ptr container_group = it->second;
    return ChangeStatus(container_group, container, new_status);
}

void Scheduler::ChangeStatus(ContainerGroup::Ptr container_group,
                             Container::Ptr container,
                             ContainerStatus new_status) {
    mu_.AssertHeld();
    ContainerId container_id = container->id;
    if (container_group->containers.find(container_id) == container_group->containers.end()) {
        LOG(WARNING) << "change status fail, no such container id: " << container_id;
        return;
    }
    ContainerStatus old_status = container->status;
    container_group->states[old_status].erase(container_id);
    container_group->states[new_status][container_id] = container;
    if (new_status == kContainerPending || new_status == kContainerTerminated) {
        std::map<AgentEndpoint, Agent::Ptr>::iterator it;
        it = agents_.find(container->allocated_agent);
        if (it != agents_.end()) {
            Agent::Ptr agent = it->second;
            agent->Evict(container);
        }
        container->allocated_volums.clear();
        container->allocated_ports.clear();
        container->allocated_agent = "";
        container->require = container_group->require;
        container->remote_info.Clear();
    }
    container->status = new_status;
}

void Scheduler::CheckTagAndPool(Agent::Ptr agent) {
    mu_.AssertHeld();
    ContainerMap containers = agent->containers_;
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        bool check_passed = CheckTagAndPoolOnce(agent, container);
        if (!check_passed) { //evit the container to pendings
            ChangeStatus(container, kContainerPending);
        }
    }
}

void Scheduler::Start() {
    LOG(INFO) << "scheduler started";
    std::vector<std::pair<ContainerGroupId, int> > replicas;
    std::set<ContainerGroupId> need_kill;
    {
        MutexLock lock(&mu_);
        stop_ = false;
        std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it;
        for (it = container_groups_.begin(); it != container_groups_.end(); it++) {
            replicas.push_back(std::make_pair(it->first, it->second->replica));
            if (it->second->terminated) {
                need_kill.insert(it->first);
            }
        }
    }

    for (size_t i = 0; i < replicas.size(); i++) {
        const ContainerGroupId& group_id = replicas[i].first;
        int replica = replicas[i].second;
        ChangeReplica(group_id, replica);
        if (need_kill.find(group_id) != need_kill.end()) {
            Kill(group_id);
        }
    }
    AgentEndpoint fake_endpoint = "";
    ScheduleNextAgent(fake_endpoint);
}

void Scheduler::Stop() {
    LOG(INFO) << "scheduler stopped.";
    MutexLock lock(&mu_);
    stop_ = true;
}

bool Scheduler::CheckTagAndPoolOnce(Agent::Ptr agent, Container::Ptr container) {
    mu_.AssertHeld();
    bool check_passed = true;
    if (!container->require->tag.empty()
        && agent->tags_.find(container->require->tag) == agent->tags_.end()) {
        container->last_res_err = proto::kTagMismatch;
        check_passed = false;
    }
    if (container->require->pool_names.find(agent->pool_name_) 
        == container->require->pool_names.end()) {
        container->last_res_err = proto::kPoolMismatch;
        check_passed = false;
    }
    return check_passed;
}

void Scheduler::CheckVersion(Agent::Ptr agent) {
    mu_.AssertHeld();
    ContainerMap containers = agent->containers_;
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        ContainerGroupId container_group_id = container->container_group_id;
        std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
        if (it == container_groups_.end()) {
            LOG(WARNING) << "check version exception, no such container_group, so evict it" << container_group_id;
            agent->Evict(container);
            continue;
        }
        ContainerGroup::Ptr container_group = it->second;
        if (container->require->version == container_group->require->version) {
            container->require = container_group->require;
            continue;
        }
        int32_t now = common::timer::now_time();
        if (now - container_group->last_update_time < container_group->update_interval) {
            continue;
        }
        //update container require, and re-schedule it.
        ChangeStatus(container, kContainerPending);
        container->require = container_group->require;
        container_group->last_update_time = now;
    }
}

void Scheduler::ScheduleNextAgent(AgentEndpoint pre_endpoint) {
    VLOG(20) << "scheduling the agent after: " << pre_endpoint;
    Agent::Ptr agent;
    AgentEndpoint endpoint;
    MutexLock lock(&mu_);
    if (stop_ || agents_.empty()) {
        if (stop_) {
            VLOG(16) << "no scheduling, because scheduler is stoped.";
        }
        if (agents_.empty()) {
            VLOG(16) << "no alive agents for scheduler.";
        }
        sched_pool_.DelayTask(FLAGS_sched_interval, 
                    boost::bind(&Scheduler::ScheduleNextAgent, this, pre_endpoint));
        return;
    }    
    std::map<AgentEndpoint, Agent::Ptr>::iterator it;
    it = agents_.upper_bound(pre_endpoint);
    if (it != agents_.end()) {
        agent = it->second;
        endpoint = it->first;
    } else {
        // turn to the start
        sched_pool_.AddTask(boost::bind(&Scheduler::ScheduleNextAgent, this, ""));
        return;
    }

    CheckVersion(agent); //check containers version
    CheckTagAndPool(agent); //may evict some containers
    //for each container_group checking pending containers, try to put on...
    std::set<ContainerGroup::Ptr, ContainerGroupQueueLess>::iterator jt;
    for (jt = container_group_queue_.begin(); jt != container_group_queue_.end(); jt++) {
        ContainerGroup::Ptr container_group = *jt;
        if (container_group->states[kContainerPending].size() == 0) {
            continue; // no pending pods
        }
        Container::Ptr container = container_group->states[kContainerPending].begin()->second;
        ResourceError res_err;
        if (!agent->TryPut(container.get(), res_err)) {
            container->last_res_err = res_err;
            VLOG(10) << "try put fail: " << container->id 
                     << ", err:" << proto::ResourceError_Name(res_err); 
            continue; //no feasiable
        }
        agent->Put(container);
        ChangeStatus(container, kContainerAllocating);
    }
    //scheduling round for the next agent
    sched_pool_.DelayTask(FLAGS_sched_interval, 
                    boost::bind(&Scheduler::ScheduleNextAgent, this, endpoint));
}

bool Scheduler::ManualSchedule(const AgentEndpoint& endpoint,
                               const ContainerGroupId& container_group_id) {
    LOG(INFO) << "manul scheduling: " << container_group_id << " @ " << endpoint;
    MutexLock lock(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator agent_it;
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator container_group_it;
    agent_it = agents_.find(endpoint);
    if (agent_it == agents_.end()) {
        LOG(WARNING) << "manual scheduling fail, no such agent:" << endpoint;
        return false;
    }
    Agent::Ptr agent = agent_it->second;
    container_group_it = container_groups_.find(container_group_id);
    if (container_group_it == container_groups_.end()) {
        LOG(WARNING) << "manual scheduling fail, no such container_group:" << container_group_id;
        return false;
    }
    ContainerGroup::Ptr container_group = container_group_it->second;
    if (container_group->states[kContainerPending].size() == 0) {
        LOG(WARNING) << "manual scheduling exception, no pending containers to put, " << container_group_id;
        return false;
    }
    Container::Ptr container_manual = container_group->states[kContainerPending].begin()->second;
    if (!CheckTagAndPoolOnce(agent, container_manual)) {
        LOG(WARNING) << "manual scheduling fail, because of mismatching tag or pools";
        return false;
    }

    std::vector<Container::Ptr> agent_containers;
    BOOST_FOREACH(ContainerMap::value_type& pair, agent->containers_) {
        agent_containers.push_back(pair.second);
    }
    std::sort(agent_containers.begin(), 
              agent_containers.end(), ContainerPriorityLess());
    std::vector<Container::Ptr>::reverse_iterator it;
    ResourceError res_err;
    bool preempt_succ = false;
    for (it = agent_containers.rbegin();
         it != agent_containers.rend(); it++) {
        Container::Ptr poor_container = *it;
        if (!agent->TryPut(container_manual.get(), res_err)) {
            ChangeStatus(poor_container, kContainerPending); //evict one
        }
        //try again after evicting
        if (agent->TryPut(container_manual.get(), res_err)) {
            agent->Put(container_manual);
            ChangeStatus(container_manual, kContainerAllocating);
            preempt_succ = true;
            break;
        } else {
            container_manual->last_res_err = res_err;
            continue;
        }
    }
    return preempt_succ;
}

bool Scheduler::Update(const ContainerGroupId& container_group_id,
                       const proto::ContainerDescription& container_desc,
                       int update_interval,
                       std::string& new_version) {
    MutexLock locker(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "update fail, no such container_group: " << container_group_id;
        return false;
    }
    ContainerGroup::Ptr container_group = it->second;
    Requirement::Ptr require(new Requirement());
    SetRequirement(require, container_desc);
    if (!RequireHasDiff(require.get(), container_group->require.get())) {
        LOG(WARNING) << "version same, ignore updating";
        return false;
    }
    new_version = GetNewVersion();
    require->version = new_version;
    container_group->update_interval = update_interval;
    container_group->last_update_time = common::timer::now_time();
    container_group->require = require;
    container_group->container_desc = container_desc;
    container_group->container_desc.set_version(new_version);
    container_group->update_time = common::timer::get_micros();
    BOOST_FOREACH(ContainerMap::value_type& pair, container_group->states[kContainerPending]) {
        Container::Ptr pending_container = pair.second;
        pending_container->require = container_group->require;
    }
    return true;
}

void Scheduler::MakeCommand(const std::string& agent_endpoint,
                            const proto::AgentInfo& agent_info, 
                            std::vector<AgentCommand>& commands) {
    MutexLock locker(&mu_);
    if (stop_) {
        LOG(INFO) << "no command to agent, when scheduler stopped.";
        return;
    }
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(agent_endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "no such agent, will kill all containers, " << agent_endpoint;
        for (int i = 0; i < agent_info.container_info_size(); i++) {
            const proto::ContainerInfo& container_remote = agent_info.container_info(i);
            AgentCommand cmd;
            LOG(INFO) << "unexpected remote containers: " << container_remote.id();
            cmd.container_id = container_remote.id();
            cmd.container_group_id = container_remote.group_id();
            cmd.action = kDestroyContainer;
            commands.push_back(cmd);
        }
        return;
    }
    Agent::Ptr agent = it->second;
    ContainerMap containers_local = agent->containers_;
    std::map<ContainerId, ContainerStatus> remote_status;
    for (int i = 0; i < agent_info.container_info_size(); i++) {
        const proto::ContainerInfo& container_remote = agent_info.container_info(i);
        ContainerMap::iterator it_local = containers_local.find(container_remote.id());
        AgentCommand cmd;
        if (it_local == containers_local.end()) {
            LOG(INFO) << "expired remote containers: " << container_remote.id();
            cmd.container_id = container_remote.id();
            cmd.container_group_id = container_remote.group_id();
            cmd.action = kDestroyContainer;
            commands.push_back(cmd);
            continue;
        } 
        const std::string& local_version = it_local->second->require->version;
        const std::string& remote_version = container_remote.container_desc().version();
        if (local_version != remote_version) {
            LOG(INFO) << "version expired:" << local_version 
                      << " , " << remote_version << ", " << container_remote.id();
            cmd.container_id = container_remote.id();
            cmd.container_group_id = container_remote.group_id();
            cmd.action = kDestroyContainer;
            commands.push_back(cmd);
            continue;
        } 
        remote_status[container_remote.id()] = container_remote.status();
        Container::Ptr container_local = it_local->second;
        container_local->remote_info.set_cpu_used(container_remote.cpu_used());
        container_local->remote_info.set_memory_used(container_remote.memory_used());
        container_local->remote_info.mutable_volum_used()->CopyFrom(container_remote.volum_used());
        container_local->remote_info.mutable_port_used()->CopyFrom(container_remote.port_used());
    }
    BOOST_FOREACH(ContainerMap::value_type& pair, containers_local) {
        Container::Ptr container_local = pair.second;
        AgentCommand cmd;
        cmd.container_id = container_local->id;
        cmd.container_group_id = container_local->container_group_id;
        ContainerStatus remote_st;
        remote_st = remote_status[container_local->id];
        ContainerGroup::Ptr container_group;
        if (container_groups_.find(container_local->container_group_id) != container_groups_.end()) {
            container_group = container_groups_[container_local->container_group_id];
        } else {
            LOG(WARNING) << "make commands exception, no such container group: " << container_local->container_group_id;
            agent->Evict(container_local);
            continue;
        }
        switch (container_local->status) {
            case kContainerAllocating:
                if (remote_st == kContainerReady) {
                    ChangeStatus(container_local, kContainerReady);
                } else if (remote_st == kContainerFinish) {
                    ChangeStatus(container_local, kContainerTerminated);
                } else if (remote_st == kContainerError) {
                    ChangeStatus(container_local, kContainerPending);
                } else {
                    cmd.action = kCreateContainer;
                    cmd.desc = container_group->container_desc;
                    SetVolumsAndPorts(container_local, cmd.desc);
                    commands.push_back(cmd);
                }
                break;
            case kContainerReady:
                if (remote_st == kContainerFinish) {
                    ChangeStatus(container_local, kContainerTerminated);
                } else if (remote_st != kContainerReady) {
                    ChangeStatus(container_local, kContainerPending);
                }
                break;
            case kContainerDestroying:
                if (remote_st == 0) {//not exit on remote
                    ChangeStatus(container_local, kContainerTerminated);
                } else if (remote_st != kContainerTerminated) {
                    cmd.action = kDestroyContainer;
                    commands.push_back(cmd);
                }
                break;
            default:
                LOG(WARNING) << "invalid status:" << container_local->id
                             << container_local->status;
        }
    }
}

bool Scheduler::RequireHasDiff(const Requirement* v1, const Requirement* v2) {
    mu_.AssertHeld();
    if (v1 == v2) {//same object 
        return false;
    }
    if (v1->tag != v2->tag) {
        return true;
    }
    if (v1->pool_names.size() != v2->pool_names.size()) {
        return true;
    }
    std::set<std::string>::iterator it = v1->pool_names.begin();
    std::set<std::string>::iterator jt = v2->pool_names.begin();
    for (; it != v1->pool_names.end() && jt != v2->pool_names.end() ; it++, jt++) {
        if (*it != *jt) {
            return true;
        }
    }
    if (v1->max_per_host != v2->max_per_host) {
        return true;
    }
    if (v1->cpu.size() != v2->cpu.size()) {
        return true;
    }
    for (size_t i = 0; i < v1->cpu.size(); i++) {
        if (v1->cpu[i].milli_core() != v2->cpu[i].milli_core() ||
            v1->cpu[i].excess() != v2->cpu[i].excess()) {
            return true;
        }
    }
    if (v1->memory.size() != v2->memory.size()) {
        return true;
    }
    for (size_t i = 0; i < v1->memory.size(); i++) {
        if (v1->memory[i].size() != v2->memory[i].size() ||
            v1->memory[i].excess() != v2->memory[i].excess()) {
            return true;
        }
    }
    if (v1->volums.size() != v2->volums.size()) {
        return true;
    }
    if (v1->ports.size() != v2->ports.size()) {
        return true;
    }
    for (size_t i = 0; i < v1->volums.size(); i++) {
        const proto::VolumRequired& vr_1 = v1->volums[i];
        const proto::VolumRequired& vr_2 = v2->volums[i];
        if (vr_1.size() != vr_2.size() || vr_1.type() != vr_2.type()
            || vr_1.medium() != vr_2.medium() 
            || vr_1.dest_path() != vr_2.dest_path()
            || vr_1.readonly() != vr_2.readonly()
            || vr_1.exclusive() != vr_2.exclusive()) {
            return true;
        }
    }
    for (size_t i = 0; i < v1->ports.size(); i++) {
        const proto::PortRequired pt_1 = v1->ports[i];
        const proto::PortRequired pt_2 = v2->ports[i];
        if (pt_1.port() != pt_2.port() || 
            pt_1.port_name() != pt_2.port_name()) {
            return true;
        }   
    }
    return false;
}

void Scheduler::SetVolumsAndPorts(const Container::Ptr& container, 
                                  proto::ContainerDescription& container_desc) {
    mu_.AssertHeld();
    size_t idx = 0;
    if (container_desc.workspace_volum().medium() != proto::kTmpfs) {
        if (idx >= container->allocated_volums.size()) {
            LOG(WARNING) << "fail to set allocated volums device path";
            return;
        }
        container_desc.mutable_workspace_volum()->set_source_path(
            container->allocated_volums[idx++].first
        );
    }
    for (int i = 0; i < container_desc.data_volums_size(); i++) {
        if (idx >= container->allocated_volums.size()) {
            break;
        }
        proto::VolumRequired* vol = container_desc.mutable_data_volums(i);
        if (vol->medium() != proto::kTmpfs) {
            vol->set_source_path(container->allocated_volums[idx++].first);
        }
    }
    idx = 0;
    for (int i = 0; i < container_desc.cgroups_size(); i++) {
        int one_cgropu_ports = container_desc.cgroups(i).ports_size();
        for (int j = 0; j < one_cgropu_ports; j++) {
            if (idx >= container->allocated_ports.size()) {
                LOG(WARNING) << "fail to set real port";
                return;
            }
            container_desc.mutable_cgroups(i)->mutable_ports(j)->set_real_port(
                container->allocated_ports[idx++]
            );
        }
    }
}

bool Scheduler::ListContainerGroups(std::vector<proto::ContainerGroupStatistics>& container_groups) {
    MutexLock lock(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it;
    for (it = container_groups_.begin(); it != container_groups_.end(); it++) {
        int64_t cpu_assigned = 0; //for one container group
        int64_t cpu_used = 0;
        int64_t memory_assigned = 0;
        int64_t memory_used = 0;
        std::map<proto::VolumMedium, int64_t> volum_assigned;
        std::map<proto::VolumMedium, int64_t> volum_used;

        ContainerGroup::Ptr& container_group = it->second;
        proto::ContainerGroupStatistics group_stat;
        group_stat.set_id(container_group->id);
        group_stat.set_name(container_group->name);
        group_stat.set_replica(container_group->Replica());
        group_stat.set_ready(container_group->states[kContainerReady].size());
        group_stat.set_pending(container_group->states[kContainerPending].size());
        group_stat.set_allocating(container_group->states[kContainerAllocating].size());
        group_stat.set_destroying(container_group->states[kContainerDestroying].size());
        group_stat.set_user_name(container_group->user_name);
        group_stat.set_submit_time(container_group->submit_time);
        group_stat.set_update_time(container_group->update_time);
        if (container_group->terminated) {
            group_stat.set_status(proto::kContainerGroupTerminated);
        } else {
            group_stat.set_status(proto::kContainerGroupNormal);
        }
        std::map<ContainerId, Container::Ptr>::iterator jt;
        for (jt = container_group->containers.begin(); 
             jt != container_group->containers.end(); jt++) {
            //sum up all containers
            Container::Ptr container = jt->second;
            if (container->status != kContainerReady) {
                continue;
            }
            for (size_t i = 0; i < container->require->volums.size(); i++) {
                proto::VolumMedium medium = container->require->volums[i].medium();
                int64_t as = container->require->volums[i].size();
                int64_t us = 0;
                if ((int)i < container->remote_info.volum_used_size()) {
                    us = container->remote_info.volum_used(i).used_size();
                }
                volum_assigned[medium] += as;
                volum_used[medium] += us;
            }
            cpu_assigned += container->require->CpuNeed();
            cpu_used += container->remote_info.cpu_used();
            memory_assigned += container->require->MemoryNeed();
            memory_used += container->remote_info.memory_used();
        }
        group_stat.mutable_cpu()->set_assigned(cpu_assigned);
        group_stat.mutable_cpu()->set_used(cpu_used);
        group_stat.mutable_memory()->set_assigned(memory_assigned);
        group_stat.mutable_memory()->set_used(memory_used);
        
        std::map<proto::VolumMedium, int64_t>::iterator v_it;
        for (v_it = volum_assigned.begin(); v_it != volum_assigned.end(); v_it++) {
            proto::VolumResource* volum_stat = group_stat.add_volums();
            proto::VolumMedium medium = v_it->first;
            int64_t assigned_size = v_it->second;
            int64_t used_size = volum_used[medium];
            volum_stat->set_medium(medium);
            volum_stat->mutable_volum()->set_assigned(assigned_size);
            volum_stat->mutable_volum()->set_used(used_size);
        }  
        container_groups.push_back(group_stat);
    }
    return true;
}

bool Scheduler::ShowContainerGroup(const ContainerGroupId& container_group_id,
                                   std::vector<proto::ContainerStatistics>& containers) {
    MutexLock lock(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it;
    it = container_groups_.find(container_group_id);
    if (it == container_groups_.end()) {
        LOG(WARNING) << "show container-group fail, no such container group: " << container_group_id;
        return false;
    }
    ContainerGroup::Ptr container_group = it->second;
    GetContainersStatistics(container_group->containers, containers);
    return true;
}

void Scheduler::GetContainersStatistics(const ContainerMap& containers_map,
                                        std::vector<proto::ContainerStatistics>& containers) {
    mu_.AssertHeld();
    BOOST_FOREACH(const ContainerMap::value_type& pair, containers_map) {
        Container::Ptr container = pair.second;
        proto::ContainerStatistics container_stat;
        container_stat.set_id(container->id);
        container_stat.set_status(container->status);
        container_stat.set_endpoint(container->allocated_agent);
        container_stat.set_last_res_err(container->last_res_err);
        std::map<proto::VolumMedium, int64_t> volum_assigned;
        std::map<proto::VolumMedium, int64_t> volum_used;
        int64_t cpu_assigned = container->require->CpuNeed();
        int64_t cpu_used = container->remote_info.cpu_used();
        int64_t memory_assigned = container->require->MemoryNeed();
        int64_t memory_used = container->remote_info.memory_used();
        for (size_t i = 0; i < container->require->volums.size(); i++) {
            proto::VolumMedium medium = container->require->volums[i].medium();
            int64_t as = container->require->volums[i].size();
            int64_t us = 0;
            if ((int)i < container->remote_info.volum_used_size()) {
                us = container->remote_info.volum_used(i).used_size();
            }
            volum_assigned[medium] += as;
            volum_used[medium] += us;
        }
        std::map<proto::VolumMedium, int64_t>::iterator v_it;
        for (v_it = volum_assigned.begin(); v_it != volum_assigned.end(); v_it++) {
            proto::VolumResource* volum_stat = container_stat.add_volums();
            proto::VolumMedium medium = v_it->first;
            int64_t assigned_size = v_it->second;
            int64_t used_size = volum_used[medium];
            volum_stat->set_medium(medium);
            volum_stat->mutable_volum()->set_assigned(assigned_size);
            volum_stat->mutable_volum()->set_used(used_size);
        }
        container_stat.mutable_cpu()->set_assigned(cpu_assigned);
        container_stat.mutable_cpu()->set_used(cpu_used);
        container_stat.mutable_memory()->set_assigned(memory_assigned);
        container_stat.mutable_memory()->set_used(memory_used);  
        containers.push_back(container_stat);
    }
}

bool Scheduler::ShowAgent(const AgentEndpoint& endpoint,
                          std::vector<proto::ContainerStatistics>& containers) {
    MutexLock lock(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator agent_it;
    agent_it = agents_.find(endpoint);
    if (agent_it == agents_.end()) {
        LOG(WARNING) << "fail to show agent, not exist: " << endpoint;
        return false;
    }
    Agent::Ptr agent = agent_it->second;
    GetContainersStatistics(agent->containers_, containers);
    return true;
}

void Scheduler::ShowUserAlloc(const std::string& user_name, proto::Quota& alloc) {
    MutexLock lock(&mu_);
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it;
    int64_t cpu_alloc = 0; //for one user
    int64_t memory_alloc = 0;
    int64_t ssd_alloc = 0;
    int64_t disk_alloc = 0;
    int64_t replica_alloc = 0;
    for (it = container_groups_.begin(); it != container_groups_.end(); it++) {
        const ContainerGroup::Ptr& container_group = it->second;
        if (container_group->user_name != user_name) {
            continue;
        }
        int64_t replica = container_group->replica;
        replica_alloc +=  replica;
        cpu_alloc += container_group->require->CpuNeed() * replica;
        memory_alloc += container_group->require->MemoryNeed() * replica;
        disk_alloc += container_group->require->DiskNeed() * replica;
        ssd_alloc += container_group->require->SsdNeed() * replica;  
    }
    alloc.set_millicore(cpu_alloc);
    alloc.set_memory(memory_alloc);
    alloc.set_replica(replica_alloc);
    alloc.set_disk(disk_alloc);
    alloc.set_ssd(ssd_alloc);
}

std::string Scheduler::GetNewVersion() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const time_t seconds = tv.tv_sec;
    struct tm t;
    localtime_r(&seconds, &t);
    std::stringstream ss;
    char time_buf[32] = { 0 };
    ::strftime(time_buf, 32, "%Y%m%d_%H:%M:%S", &t);
    ss << "ver_" << time_buf << "_" << random();
    return ss.str();
}

} //namespace sched
} //namespace galaxy
} //namespace baidu
