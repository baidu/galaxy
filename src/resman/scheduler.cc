// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"
#include <boost/foreach.hpp>

namespace baidu {
namespace galaxy {
namespace sched {

Agent::Agent(const AgentEndpoint& endpoint,
            int64_t cpu,
            int64_t memory,
            const std::map<DevicePath, VolumInfo>& volums) {

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
