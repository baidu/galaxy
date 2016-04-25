// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"

#include <sys/time.h>
#include <time.h>
#include <sstream>
#include <boost/foreach.hpp>
#include <glog/logging.h>

namespace baidu {
namespace galaxy {
namespace sched {

const size_t kMaxPorts = 60000;
const size_t kMinPort = 1000;
const std::string kDynamicPort = "dynamic";

Agent::Agent(const AgentEndpoint& endpoint,
            int64_t cpu,
            int64_t memory,
            const std::map<DevicePath, VolumInfo>& volums,
            const std::set<std::string>& labels) {
    cpu_total_ = cpu;
    cpu_assigned_ = 0;
    memory_total_ = memory;
    memory_assigned_ = 0;
    volum_total_ = volums;
    port_total_ = kMaxPorts;
    labels_ = labels;
}

void Agent::SetAssignment(int64_t cpu_assigned,
                          int64_t memory_assigned,
                          const std::map<DevicePath, VolumInfo>& volum_assigned,
                          const std::set<std::string> port_assigned) {
    cpu_assigned_ = cpu_assigned;
    memory_assigned_ = memory_assigned;
    volum_assigned_ =  volum_assigned;
    port_assigned_ = port_assigned;
}

bool Agent::TryPut(const Container* container, ResourceError& err) {
    if (!container->require->label.empty() &&
        labels_.find(container->require->label) == labels_.end()) {
        err = kLabelMismatch;
        return false;
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
    //cpu 
    cpu_assigned_ += container->require->cpu.milli_core();
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
            double rn = rand() / (RAND_MAX+0.0);
            int random_port = kMinPort + static_cast<int>(rn * kMaxPorts);
            std::stringstream ss;
            ss << random_port;
            s_port = ss.str();
        }
        port_assigned_.insert(s_port);
        container->allocated_port.insert(s_port);
    }
    //put on this agent succesfully
    container->allocated_agent = endpoint_;
    containers_[container->id] = container;
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
    container->allocated_volums.clear();
    //port
    BOOST_FOREACH(const std::string& port, container->allocated_port) {
        port_assigned_.erase(port);
    }
    container->allocated_port.clear();
    //erase from this agent
    container->allocated_agent = "";
    containers_.erase(container->id);
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
    agents_[agent->endpoint_] = agent;
}

void Scheduler::RemoveAgent(const AgentEndpoint& endpoint) {
    std::map<AgentEndpoint, Agent::Ptr>::iterator it;
    it = agents_.find(endpoint);
    if (it == agents_.end()) {
        return;
    }
    const Agent::Ptr& agent = it->second;
    ContainerMap containers = agent->containers_; //copy
    BOOST_FOREACH(ContainerMap::value_type& pair, containers) {
        Container::Ptr container = pair.second;
        agent->Evict(container);
        ChangeStatus(container, kPending);        
    }
    agents_.erase(endpoint);
}

JobId Scheduler::GenerateJobId(const std::string& job_name) {
    std::string prefix = "";
    BOOST_FOREACH(char c, job_name) {
        if (!isalnum(c)) {
            prefix += "_";
        } else {
            prefix += c;
        }
        if (prefix.length() >= 16) {//truncate
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
    ss << prefix << "_" << time_buf << "_"
       << random();
    return ss.str();
}

JobId Scheduler::Submit(const std::string& job_name,
                        const Requirement& require, 
                        int replica) {
    JobId jobid = GenerateJobId(job_name);
    if (jobs_.find(jobid) != jobs_.end()) {
        LOG(WARNING) << "job id conflict:" << jobid;
        return "";
    }
    Requirement::Ptr req(new Requirement(require));
    Job::Ptr job(new Job());
    job->require = req;
    job->id = jobid;
    job->replica = replica;
    for (int i = 0 ; i < replica; i++) {
        Container::Ptr container(new Container());
        container->job_id = job->id;
        std::stringstream ss;
        ss << job->id << ".vm_" << i;
        container->id = ss.str();
        container->require = req;
        job->containers[container->id] = container;
        ChangeStatus(job, container, kPending);
    }
    jobs_[jobid] = job;
    return job->id;
}

void Scheduler::Kill(const JobId& job_id) {
    std::map<JobId, Job::Ptr>::iterator it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "unkonw job id: " << job_id;
        return;
    }
    Job::Ptr& job = it->second;
    BOOST_FOREACH(ContainerMap::value_type& pair, job->containers) {
        Container::Ptr container = pair.second;
        if (container->status != kRunning && container->status != kError) {
            ChangeStatus(job, container, kTerminated);
        } else {
            ChangeStatus(job, container, kDestroying);
        }
    }
}

void Scheduler::ScaleUpDown(const JobId& job_id, int replica) {
    std::map<JobId, Job::Ptr>::iterator it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "unkonw job id: " << job_id;
        return;
    }
    Job::Ptr job = it->second;
    if (replica == job->replica) {
        LOG(INFO) << "replica not change, do nothing" ;
        return;
    } else if (replica < job->replica) {
        //scale down
        int delta = job->replica - replica;
        //remove from pending first
        BOOST_FOREACH(ContainerMap::value_type& pair, job->states[kPending]) {
            Container::Ptr container = pair.second;
            ChangeStatus(job, container, kTerminated);
            --delta;
            if (delta <= 0) {
                break;
            }
        }
        ContainerStatus all_status[] = {kError, kAllocating, kRunning};
        for (int i = 0; i < 3 && delta > 0; i++) {
            ContainerStatus st = all_status[i];
            BOOST_FOREACH(ContainerMap::value_type& pair, job->states[st]) {
                Container::Ptr container = pair.second;
                ChangeStatus(container, kDestroying);
                --delta;
                if (delta <= 0) {
                    break;
                }
            }
        }
        job->replica = replica;
    } else {
        //scale up
        int delta = replica - job->replica;
        int from_no = job->containers.size();
        int to_no = job->containers.size() + delta;
        for (int i = from_no; i <= to_no; i++) {
            Container::Ptr container(new Container());
            container->job_id = job->id;
            std::stringstream ss;
            ss << job->id << ".vm_" << i;
            container->id = ss.str();
            container->require = job->require;
            job->containers[container->id] = container;
            ChangeStatus(job, container, kPending);            
        }
        job->replica = replica; 
    }
}

void Scheduler::ShowAssignment(const AgentEndpoint& endpoint,
                               std::vector<Container>& containers) {
    containers.clear();
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        return;
    }
    Agent::Ptr agent = it->second;
    BOOST_FOREACH(const ContainerMap::value_type& pair, agent->containers_) {
        containers.push_back(*pair.second);
    }
}

void Scheduler::ShowJob(const JobId job_id,
                        std::vector<Container>& containers) {
    std::map<JobId, Job::Ptr>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return;
    }
    const Job::Ptr& job = it->second;
    BOOST_FOREACH(ContainerMap::value_type& pair, job->containers) {
        containers.push_back(*pair.second);
    }
}

void Scheduler::ChangeStatus(const JobId& job_id,
                             const ContainerId& container_id,
                             ContainerStatus new_status) {
    
}

void Scheduler::ChangeStatus(Container::Ptr container,
                             ContainerStatus new_status) {
    JobId job_id = container->job_id;
    std::map<JobId, Job::Ptr>::iterator it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return;
    }
    Job::Ptr job = it->second;
    return ChangeStatus(job, container, new_status);
}

void Scheduler::ChangeStatus(Job::Ptr job,
                             Container::Ptr container,
                             ContainerStatus new_status) {
    ContainerId container_id = container->id;
    if (job->containers.find(container_id) == job->containers.end()) {
        return;
    }
    ContainerStatus old_status = container->status;
    job->states[old_status].erase(container_id);
    job->states[new_status][container_id] = container;
}

void Scheduler::Start() {

}

} //namespace sched
} //namespace galaxy
} //namespace baidu
