// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"
#include <sstream>
#include <boost/foreach.hpp>

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
    agent_assign_[agent->endpoint_] = agent;
}

void Scheduler::RemoveAgent(const AgentEndpoint& endpoint) {
    agent_assign_.erase(endpoint);
}

ContainerGroupId Scheduler::Submit(const Requirement& require, int replica) {
    Requirement::Ptr req(new Requirement(require));
    return "";    
}

void Scheduler::Kill(const ContainerGroupId& group_id) {

}

void Scheduler::ScaleUp(const ContainerGroupId& group_id, int replica) {

}

void Scheduler::ScaleDown(const ContainerGroupId& group_id, int replica) {

}

void Scheduler::ShowAssignment(const AgentEndpoint& endpoint,
                               std::vector<Container>& containers) {
    containers.clear();
    std::map<AgentEndpoint, Agent::Ptr>::iterator it = agent_assign_.find(endpoint);
    if (it == agent_assign_.end()) {
        return;
    }
    Agent::Ptr agent = it->second;
    typedef std::map<ContainerId, Container::Ptr> ContainerMap;
    BOOST_FOREACH(const ContainerMap::value_type& pair, agent->containers_) {
        containers.push_back(*pair.second);
    }
}

void Scheduler::ShowContainerGroup(const ContainerGroupId group_id,
                                   std::vector<Container>& containers) {
    std::map<ContainerGroupId, ContainerGroup::Ptr>::iterator it;
    it = container_groups_.find(group_id);
    if (it == container_groups_.end()) {
        return;
    }
    const ContainerGroup::Ptr& ct_group = it->second;
    BOOST_FOREACH(const Container::Ptr container, ct_group->containers) {
        containers.push_back(*container);
    }
}

void Scheduler::ChangeStatus(const ContainerId& container_id, ContainerStatus status) {

}

void Scheduler::Start() {

}

} //namespace sched
} //namespace galaxy
} //namespace baidu
