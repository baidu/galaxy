// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "scheduler.h"

namespace baidu {
namespace galaxy {
namespace sched {

Agent::Agent(const AgentEndpoint endpoint,
             const Resource& res_total) {

}

bool Agent::CanPut(const Container* container) {
    return false;
}

void Agent::Put(Container::Ptr container) {

}

void Agent::Evict(Container::Ptr container) {

}

Scheduler::Scheduler() {

}

void Scheduler::AddAgent(Agent::Ptr agent) {

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
