// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"

#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include "timer.h"

DECLARE_int64(sched_interval);
DECLARE_int64(group_gc_check_interval);

namespace baidu {
namespace galaxy {
namespace sched {

const uint32_t kMaxPort = 60000;
const uint32_t kMinPort = 1000;
const std::string kDynamicPort = "dynamic";

Agent::Agent(const AgentEndpoint& endpoint,
            int64_t cpu,
            int64_t memory,
            const std::map<DevicePath, VolumInfo>& volums,
            const std::set<std::string>& labels,
            const std::string& pool_name) {
    cpu_total_ = cpu;
    cpu_assigned_ = 0;
    memory_total_ = memory;
    memory_assigned_ = 0;
    volum_total_ = volums;
    port_total_ = kMaxPort - kMinPort + 1;
    labels_ = labels;
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
        container_counts_[container->group_id] += 1;
    }
}

bool Agent::TryPut(const Container* container, ResourceError& err) {
    if (!container->require->label.empty() &&
        labels_.find(container->require->label) == labels_.end()) {
        err = kLabelMismatch;
        return false;
    }
    if (container->require->pool_names.find(pool_name_) 
        == container->require->pool_names.end()) {
        err = kPoolMismatch;
        return false;
    }

    if (container->require->max_per_host > 0) {
        GroupId group_id = container->group_id;
        std::map<GroupId, int>::iterator it = container_counts_.find(group_id);
        if (it != container_counts_.end()) {
            int cur_counts = it->second;
            if (cur_counts >= container->require->max_per_host) {
                err = kTooManyPods;
                return false;
            } 
        }
    }

    if (container->require->cpu.milli_core() + cpu_assigned_ > cpu_total_) {
        err = kNoCpu;
        return false;
    }
    if (container->require->memory.size() + memory_assigned_ > memory_total_) {
        err = kNoMemory;
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

    if (size_ramdisk + memory_assigned_ > memory_total_) {
        err = kNoMemoryForTmpfs;
        return false;
    }

    std::vector<DevicePath> devices;
    if (!SelectDevices(volums_no_ramdisk, devices)) {
        err = kNoDevice;
        return false;
    }

    if (container->require->ports.size() + port_assigned_.size() 
        > port_total_) {
        err = kNoPort;
        return false;
    }

    const std::vector<proto::PortRequired> ports = container->require->ports;
    BOOST_FOREACH(const proto::PortRequired& port, ports) {
        if (port.port() != kDynamicPort
            && port_assigned_.find(port.port()) != port_assigned_.end()) {
            err = kPortConflict;
            return false;
        } 
    }
    return true;
}

void Agent::Put(Container::Ptr container) {
    assert(container->status == kPending);
    assert(container->allocated_agent.empty());
    //cpu 
    cpu_assigned_ += container->require->cpu.milli_core();
    assert(cpu_assigned_ <= cpu_total_);
    //memory
    memory_assigned_ += container->require->memory.size();
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
            container->allocated_volums.push_back(device_path);
            const proto::VolumRequired& volum = volums_no_ramdisk[i];
            volum_assigned_[device_path].size += volum.size();
            if (volum.exclusive()) {
                volum_assigned_[device_path].exclusive = true;
            }
        }
    }  
    //ports
    BOOST_FOREACH(const proto::PortRequired& port, container->require->ports) {
        std::string s_port;
        if (port.port() != kDynamicPort) {
            s_port = port.port();
        } else {
            uint32_t max_tries = (kMaxPort - kMinPort + 1);
            double rand_scale = (double)rand() / (RAND_MAX + 1.0);
            uint32_t start_port = kMinPort + (uint32_t)(max_tries * rand_scale);
            for (uint32_t i = 0; i < max_tries; i++) {
                std::stringstream ss;
                ss << start_port;
                const std::string& random_port = ss.str();
                if (port_assigned_.find(random_port) == port_assigned_.end()) {
                    s_port = random_port;
                    break;
                }
                start_port ++;
                if (start_port > kMaxPort) {
                    start_port = kMinPort;
                }
            }
        }
        if (!s_port.empty()) {
            port_assigned_.insert(s_port);
            container->allocated_port.insert(s_port);
        } else {
            LOG(WARNING) << "no free port.";
        }
    }
    //put on this agent succesfully
    container->allocated_agent = endpoint_;
    container->last_res_err = kOk;
    containers_[container->id] = container;
    container_counts_[container->group_id] += 1;
}

void Agent::Evict(Container::Ptr container) {
    //cpu 
    cpu_assigned_ -= container->require->cpu.milli_core();
    assert(cpu_assigned_ >= 0);
    //memory
    memory_assigned_ -= container->require->memory.size();
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
        const DevicePath& device_path = container->allocated_volums[i];
        const proto::VolumRequired& volum = volums_no_ramdisk[i];
        volum_assigned_[device_path].size -= volum.size();
        if (volum.exclusive()) {
            volum_assigned_[device_path].exclusive = false;
        }
    }
    containers_.erase(container->id);
    container_counts_[container->group_id] -= 1;
    if (container_counts_[container->group_id] <= 0) {
        container_counts_.erase(container->group_id);
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


Scheduler::Scheduler() {

}

void Scheduler::AddAgent(Agent::Ptr agent) {
    MutexLock locker(&mu_);
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
        ChangeStatus(container, kPending);        
    }
    agents_.erase(endpoint);
}

void Scheduler::AddLabel(const AgentEndpoint& endpoint, const std::string& label) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->labels_.insert(label);
}

void Scheduler::RemoveLabel(const AgentEndpoint& endpoint, const std::string& label) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->labels_.erase(label);
}

void Scheduler::SetPool(const AgentEndpoint& endpoint, const std::string& pool_name) {
    MutexLock locker(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "no such agent:" << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    agent->pool_name_ = pool_name;
}

GroupId Scheduler::GenerateGroupId(const std::string& group_name) {
    std::string suffix = "";
    BOOST_FOREACH(char c, group_name) {
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
    ss << "group_" << time_buf << "_"
       << random() << "_" << suffix;
    return ss.str();
}

ContainerId Scheduler::GenerateContainerId(const GroupId& group_id, int offset) {
    std::stringstream ss;
    ss << group_id << ".vm_" << offset;
    return ss.str();
}

GroupId Scheduler::Submit(const std::string& group_name,
                          const Requirement& require, 
                          int replica, int priority) {
    MutexLock locker(&mu_);
    GroupId group_id = GenerateGroupId(group_name);
    if (groups_.find(group_id) != groups_.end()) {
        LOG(WARNING) << "group id conflict:" << group_id;
        return "";
    }
    Requirement::Ptr req(new Requirement(require));
    Group::Ptr group(new Group());
    group->require = req;
    group->id = group_id;
    group->priority = priority;
    for (int i = 0 ; i < replica; i++) {
        Container::Ptr container(new Container());
        container->group_id = group->id;
        container->id = GenerateContainerId(group_id, i);
        container->require = req;
        container->priority = priority;
        group->containers[container->id] = container;
        ChangeStatus(group, container, kPending);
    }
    groups_[group_id] = group;
    group_queue_.insert(group);
    return group->id;
}

bool Scheduler::Kill(const GroupId& group_id) {
    MutexLock locker(&mu_);
    std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "unkonw group id: " << group_id;
        return false;
    }
    Group::Ptr group = it->second;
    if (group->terminated) {
        LOG(WARNING) << "ignore the killing, "
                     << group_id << " already killed";
        return false;
    }
    BOOST_FOREACH(ContainerMap::value_type& pair, group->containers) {
        Container::Ptr container = pair.second;
        if (container->status == kPending) {
            ChangeStatus(group, container, kTerminated);
        } else if (container->status != kTerminated){
            ChangeStatus(group, container, kDestroying);
        }
    }
    group->terminated = true;
    gc_pool_.AddTask(boost::bind(&Scheduler::CheckGroupGC, this, group));
    return true;
}

void Scheduler::CheckGroupGC(Group::Ptr group) {
    MutexLock locker(&mu_);
    assert(group->terminated);
    bool all_container_terminated = true;
    BOOST_FOREACH(ContainerMap::value_type& pair, group->containers) {
        Container::Ptr container = pair.second;
        if (container->status != kTerminated) {
            all_container_terminated  = false;
            break;
        }
    }
    if (all_container_terminated) {
        groups_.erase(group->id);
        group_queue_.erase(group);
        //after this, all containers wish to be deleted
    } else {
        gc_pool_.DelayTask(FLAGS_group_gc_check_interval,
                           boost::bind(&Scheduler::CheckGroupGC, this, group));
    }
}

bool Scheduler::ChangeReplica(const GroupId& group_id, int replica) {
    MutexLock locker(&mu_);
    std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "unkonw group id: " << group_id;
        return false;
    }
    if (replica < 0) {
        LOG(WARNING) << "ignore invalid replica: " << replica;
        return false;
    }
    Group::Ptr group = it->second;
    if (group->terminated) {
        LOG(WARNING) << "terminated group can not be scale up/down";
        return false;
    }
    int current_replica = group->Replica();
    if (replica == current_replica) {
        LOG(INFO) << "replica not change, do nothing" ;
    } else if (replica < current_replica) {
        ScaleDown(group, replica);
    } else {
        ScaleUp(group, replica);
    }
    return true;
}

void Scheduler::ScaleDown(Group::Ptr group, int replica) {
    mu_.AssertHeld();
    int delta = group->Replica() - replica;
    //remove from pending first
    ContainerMap pending_containers = group->states[kPending];
    BOOST_FOREACH(ContainerMap::value_type& pair, pending_containers) {
        Container::Ptr container = pair.second;
        ChangeStatus(group, container, kTerminated);
        --delta;
        if (delta <= 0) {
            break;
        }
    }
    ContainerStatus all_status[] = {kAllocating, kRunning};
    for (size_t i = 0; i < sizeof(all_status) && delta > 0; i++) {
        ContainerStatus st = all_status[i];
        ContainerMap working_containers = group->states[st];
        BOOST_FOREACH(ContainerMap::value_type& pair, working_containers) {
            Container::Ptr container = pair.second;
            ChangeStatus(container, kDestroying);
            --delta;
            if (delta <= 0) {
                break;
            }
        }
    }
}

void Scheduler::ScaleUp(Group::Ptr group, int replica) {
    mu_.AssertHeld();
    for (int i = 0; i < replica; i++) {
        if (group->Replica() >= replica) {
            break;
        }
        ContainerId container_id = GenerateContainerId(group->id, i);
        Container::Ptr container;
        ContainerMap::iterator it = group->containers.find(container_id);
        if (it == group->containers.end()) {
            container.reset(new Container());
            container->group_id = group->id;
            container->id = container_id;
            container->require = group->require;
            group->containers[container_id] = container;
        } else {
            container = it->second;
        }
        if (container->status != kRunning && container->status != kAllocating) {
            ChangeStatus(group, container, kPending);            
        }
    }
}

void Scheduler::ShowAssignment(const AgentEndpoint& endpoint,
                               std::vector<Container>& containers) {
    MutexLock locker(&mu_);
    containers.clear();
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(WARNING) << "no such agent: " << endpoint;
        return;
    }
    Agent::Ptr agent = it->second;
    BOOST_FOREACH(const ContainerMap::value_type& pair, agent->containers_) {
        containers.push_back(*pair.second);
    }
}

void Scheduler::ShowGroup(const GroupId group_id,
                        std::vector<Container>& containers) {
    MutexLock locker(&mu_);
    std::map<GroupId, Group::Ptr>::iterator it;
    it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "no such group id: " << group_id;
        return;
    }
    const Group::Ptr& group = it->second;
    BOOST_FOREACH(ContainerMap::value_type& pair, group->containers) {
        containers.push_back(*pair.second);
    }
}

void Scheduler::ChangeStatus(const GroupId& group_id,
                             const ContainerId& container_id,
                             ContainerStatus new_status) {
    MutexLock locker(&mu_);
    std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "no such group: " << group_id;
        return;
    }
    Group::Ptr group = it->second;
    ContainerMap::iterator ct = group->containers.find(container_id);
    if (ct == group->containers.end()) {
        LOG(WARNING) << "no such container: " << container_id;
        return;
    }
    Container::Ptr container = ct->second;
    return ChangeStatus(container, new_status);
}

void Scheduler::ChangeStatus(Container::Ptr container,
                             ContainerStatus new_status) {
    mu_.AssertHeld();
    GroupId group_id = container->group_id;
    std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "no such group:" << group_id;
        return;
    }
    Group::Ptr group = it->second;
    return ChangeStatus(group, container, new_status);
}

void Scheduler::ChangeStatus(Group::Ptr group,
                             Container::Ptr container,
                             ContainerStatus new_status) {
    mu_.AssertHeld();
    ContainerId container_id = container->id;
    if (group->containers.find(container_id) == group->containers.end()) {
        LOG(WARNING) << "no such container id: " << container_id;
        return;
    }
    ContainerStatus old_status = container->status;
    group->states[old_status].erase(container_id);
    group->states[new_status][container_id] = container;
    if ((new_status == kPending || new_status == kTerminated)
        && container->allocated_agent != "") {
        std::map<AgentEndpoint, Agent::Ptr>::iterator it;
        it = agents_.find(container->allocated_agent);
        if (it != agents_.end()) {
            Agent::Ptr agent = it->second;
            agent->Evict(container);
        }
        container->allocated_volums.clear();
        container->allocated_port.clear();
        container->allocated_agent = "";
        container->require = group->require;
    }
    container->status = new_status;
}

void Scheduler::CheckLabelAndPool(Agent::Ptr agent) {
    mu_.AssertHeld();
    ContainerMap containers = agent->containers_;
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        bool check_passed = CheckLabelAndPoolOnce(agent, container);
        if (!check_passed) { //evit the container to pendings
            ChangeStatus(container, kPending);
        }
    }
}

void Scheduler::Start() {
    AgentEndpoint fake_endpoint = "";
    ScheduleNextAgent(fake_endpoint);
}

bool Scheduler::CheckLabelAndPoolOnce(Agent::Ptr agent, Container::Ptr container) {
    mu_.AssertHeld();
    bool check_passed = true;
    if (!container->require->label.empty()
        && agent->labels_.find(container->require->label) == agent->labels_.end()) {
        container->last_res_err = kLabelMismatch;
        check_passed = false;
    }
    if (container->require->pool_names.find(agent->pool_name_) 
        == container->require->pool_names.end()) {
        container->last_res_err = kPoolMismatch;
        check_passed = false;
    }
    return check_passed;
}

void Scheduler::CheckVersion(Agent::Ptr agent) {
    mu_.AssertHeld();
    ContainerMap containers = agent->containers_;
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        GroupId group_id = container->group_id;
        std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
        if (it == groups_.end()) {
            LOG(WARNING) << "no such group, so evict it" << group_id;
            agent->Evict(container);
            continue;
        }
        Group::Ptr group = it->second;
        if (!RequireHasDiff(container->require.get(), group->require.get())) {
            container->require = group->require;
            continue;
        }
        int32_t now = common::timer::now_time();
        if (now - group->last_update_time < group->update_interval) {
            continue;
        }
        //update container require, and re-schedule it.
        ChangeStatus(container, kPending);
        container->require = group->require;
        group->last_update_time = now;
    }
}

void Scheduler::ScheduleNextAgent(AgentEndpoint pre_endpoint) {
    VLOG(16) << "scheduling the agent after: " << pre_endpoint;
    Agent::Ptr agent;
    AgentEndpoint endpoint;
    MutexLock lock(&mu_);
        
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

    CheckLabelAndPool(agent); //may evict some containers
    CheckVersion(agent); //check containers version

    //for each group checking pending containers, try to put on...
    std::set<Group::Ptr, GroupQueueLess>::iterator jt;
    for (jt = group_queue_.begin(); jt != group_queue_.end(); jt++) {
        Group::Ptr group = *jt;
        if (group->states[kPending].size() == 0) {
            continue; // no pending pods
        }
        Container::Ptr container = group->states[kPending].begin()->second;
        ResourceError res_err;
        if (!agent->TryPut(container.get(), res_err)) {
            container->last_res_err = res_err;
            continue; //no feasiable
        }
        agent->Put(container);
        ChangeStatus(container, kAllocating);
    }
    //scheduling round for the next agent
    sched_pool_.DelayTask(FLAGS_sched_interval, 
                    boost::bind(&Scheduler::ScheduleNextAgent, this, endpoint));
}

bool Scheduler::ManualSchedule(const AgentEndpoint& endpoint,
                               const GroupId& group_id) {
    LOG(INFO) << "manul scheduling: " << group_id << " @ " << endpoint;
    MutexLock lock(&mu_);
    std::map<AgentEndpoint, Agent::Ptr>::iterator agent_it;
    std::map<GroupId, Group::Ptr>::iterator group_it;
    agent_it = agents_.find(endpoint);
    if (agent_it == agents_.end()) {
        LOG(WARNING) << "no such agent:" << endpoint;
        return false;
    }
    Agent::Ptr agent = agent_it->second;
    group_it = groups_.find(group_id);
    if (group_it == groups_.end()) {
        LOG(WARNING) << "no such group:" << group_id;
        return false;
    }
    Group::Ptr group = group_it->second;
    if (group->states[kPending].size() == 0) {
        LOG(WARNING) << "no pending containers to put, " << group_id;
        return false;
    }
    Container::Ptr container_manual = group->states[kPending].begin()->second;
    if (!CheckLabelAndPoolOnce(agent, container_manual)) {
        LOG(WARNING) << "manual scheduling fail, because of mismatching label or pools";
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
            ChangeStatus(poor_container, kPending); //evict one
        }
        //try again after evicting
        if (agent->TryPut(container_manual.get(), res_err)) {
            agent->Put(container_manual);
            ChangeStatus(container_manual, kAllocating);
            preempt_succ = true;
            break;
        } else {
            container_manual->last_res_err = res_err;
            continue;
        }
    }
    return preempt_succ;
}

bool Scheduler::Update(const GroupId& group_id,
                       const Requirement& require,
                       int update_interval) {
    MutexLock locker(&mu_);
    std::map<GroupId, Group::Ptr>::iterator it = groups_.find(group_id);
    if (it == groups_.end()) {
        LOG(WARNING) << "no such group: " << group_id;
        return false;
    }
    Group::Ptr group = it->second;
    if (!RequireHasDiff(&require, group->require.get())) {
        LOG(WARNING) << "version same, ignore updating";
        return false;
    }
    group->update_interval = update_interval;
    group->last_update_time = common::timer::now_time();
    group->require.reset(new Requirement(require));
    BOOST_FOREACH(ContainerMap::value_type& pair, group->states[kPending]) {
        Container::Ptr pending_container = pair.second;
        pending_container->require = group->require;
    }
    return true;
}

bool Scheduler::RequireHasDiff(const Requirement* v1, const Requirement* v2) {
    if (v1 == v2) {//same object 
        return false;
    }
    if (v1->label != v2->label) {
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
    if (v1->cpu.milli_core() != v2->cpu.milli_core() ||
        v1->cpu.excess() != v2->cpu.excess()) {
        return true;
    }
    if (v1->memory.size() != v2->memory.size() ||
        v1->memory.excess() != v2->memory.excess()) {
        return true;
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
            || vr_1.source_path() != vr_2.source_path()
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

} //namespace sched
} //namespace galaxy
} //namespace baidu
