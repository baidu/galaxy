// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"
#include <boost/foreach.hpp>

namespace baidu {
namespace galaxy {
namespace sched {

Agent::Agent(const AgentEndpoint endpoint,
             const Resource& res_total) {

}

bool Agent::CanPut(const Container* container, ResourceError& err) {
    if (labels_.find(container->require.label) == labels_.end()) {
        err = kLabelMismatch;
        return false;
    }
    if (container->require.res.cpu.milli_core() + res_assigned_.cpu.milli_core()
        > res_total_.cpu.milli_core()) {
        err = kNoCpu;
        return false;
    }
    if (container->require.res.memory.size() + res_assigned_.memory.size()
        > res_total_.memory.size()) {
        err = kNoMemory;
        return false;
    }
    //volums check
    std::map<proto::VolumMedium, std::map<std::string, proto::VolumRequired> > free_volums;
    BOOST_FOREACH(proto::VolumRequired& v, res_total_.volums) {
        free_volums[v.medium()][v.source_path()] = v;
    }

    BOOST_FOREACH(proto::VolumRequired& v, res_assigned_.volums) {
        int64_t remain_size = free_volums[v.medium()][v.source_path()].size();
        free_volums[v.medium()][v.source_path()].set_size(remain_size - v.size());
    }
    
    const std::vector<proto::VolumRequired>& volums_req = container->require.res.volums;
    int64_t sum_tmpfs_size = 0;
    BOOST_FOREACH(const proto::VolumRequired& volum, volums_req) {
        if (volum.medium() == proto::kTmpfs) { //ramdisk
            sum_tmpfs_size += volum.size();
            continue;
        }
        if (free_volums.find(volum.medium()) == free_volums.end()) {
            err = kNoMedium;
            return false; // no shuch medium on this agent
        }
        bool found_device = false; // try find at least one device
        typedef std::map<std::string, proto::VolumRequired> Path2Volum;
        const Path2Volum& free_v = free_volums[volum.medium()];
        BOOST_FOREACH(const Path2Volum::value_type& pair, free_v) {
            const std::string source_path = pair.first;
            const proto::VolumRequired& free_volum = pair.second;
            if (free_volum.size() > volum.size()) {
                found_device = true;
                break;
            }
        }
        if (!found_device) {
            err = kNoDevice;
            return false;
        }
    }

    if (container->require.res.memory.size()
        + sum_tmpfs_size + res_assigned_.memory.size() 
        > res_total_.memory.size()) {
        err = kNoMemoryForTmpfs;
        return false;
    }
    //ports check
    typedef std::map<std::string, proto::PortRequired> PortMap;
    const PortMap& ports = container->require.res.ports;
    BOOST_FOREACH(const PortMap::value_type& pair, ports) {
        const std::string port_key = pair.first;
        const proto::PortRequired& port = pair.second;
        if (port.port() == "dynamic") {
            if (res_assigned_.ports.size() < res_total_.ports.size() ) {
                continue; // can random choose one
            } else {
                err = kNoPort;// no free ports
                return false; 
            }
        }
        if (res_total_.ports.find(port.port()) == res_total_.ports.end()) {
            err = kNoPort;
            return false;
        }
        if (res_assigned_.ports.find(port.port()) != res_assigned_.ports.end()) {
            err = kPortConflict;
            return false;
        }
    }
    return true;
}

void Agent::Put(Container::Ptr container) {

}

void Agent::Evict(Container::Ptr container) {

}

Scheduler::Scheduler() {

}

void Scheduler::AddAgent(Agent::Ptr agent) {

}

void Scheduler::RemoveAgent(const AgentEndpoint& endpoint) {

}

ContainerGroupId Scheduler::Submit(const Requirement& require, int replica) {
    return "";
}

void Scheduler::Kill(const ContainerGroupId& group_id) {

}

void Scheduler::ScaleUp(const ContainerGroupId& group_id, int replica) {

}

void Scheduler::ScaleDown(const ContainerGroupId& group_id, int replica) {

}

void Scheduler::ShowAssignment(const AgentEndpoint& endpoint,
                               std::vector<Container::Ptr>& containers) {

}

void Scheduler::ShowContainerGroup(const ContainerGroupId group_id,
                                   std::vector<Container::Ptr>& containers) {

}

void Scheduler::ChangeStatus(const ContainerId& container_id, ContainerStatus status) {

}

void Scheduler::Start() {

}

} //namespace sched
} //namespace galaxy
} //namespace baidu
