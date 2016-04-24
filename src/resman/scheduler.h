// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "src/protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {
namespace sched {

typedef std::string AgentEndpoint;
typedef std::string ContainerGroupId;
typedef std::string ContainerId;
typedef std::string DevicePath;

enum ContainerStatus {
    kPending = 1,
    kAllocating = 2,
    kRunning = 3,
    kError = 4,
    kDestroying = 5,
    kTerminated = 6
};

enum ResourceError {
    kNoCpu = 1,
    kNoMemory = 2,
    kNoMedium = 3,
    kNoDevice = 4,
    kNoPort = 5,
    kPortConflict = 6,
    kLabelMismatch = 7,
    kNoMemoryForTmpfs = 8
};

struct Requirement {
    const std::string label;
    proto::CpuRequired cpu;
    proto::MemoryRequired memory;
    std::vector<proto::VolumRequired> volums;
    std::vector<proto::PortRequired> ports;
    typedef boost::shared_ptr<Requirement> Ptr;
};

struct Container {
    ContainerId id;
    ContainerGroupId group_id;
    ContainerStatus status;
    Requirement::Ptr require;
    std::vector<DevicePath> allocated_volums;
    std::set<std::string> allocated_port;
    AgentEndpoint allocated_agent;
    typedef boost::shared_ptr<Container> Ptr;
};

struct ContainerGroup {
    ContainerGroupId id;
    Requirement::Ptr require;
    std::vector<Container::Ptr> containers;
    std::list<Container::Ptr> pending_queue;
    typedef boost::shared_ptr<ContainerGroup> Ptr;
};

struct VolumInfo {
    proto::VolumMedium medium;
    bool exclusive;
    int64_t size;
    VolumInfo() : exclusive(false), size(0) {}
};

class Agent {
public:
    friend class Scheduler;
    explicit Agent(const AgentEndpoint& endpoint,
                   int64_t cpu,
                   int64_t memory,
                   const std::map<DevicePath, VolumInfo>& volums,
                   const std::set<std::string>& labels);
    bool TryPut(const Container* container, ResourceError& err);
    void Put(Container::Ptr container);
    void Evict(Container::Ptr container);
    typedef boost::shared_ptr<Agent> Ptr;
private:
    bool SelectDevices(const std::vector<proto::VolumRequired>& volums,
                       std::vector<DevicePath>& devices);
    bool RecurSelectDevices(size_t i, const std::vector<proto::VolumRequired>& volums,
                            std::map<DevicePath, VolumInfo>& volum_free,
                            std::vector<DevicePath>& devices);
    AgentEndpoint endpoint_;
    std::set<std::string> labels_;
    int64_t cpu_total_;
    int64_t cpu_assigned_;
    int64_t memory_total_;
    int64_t memory_assigned_;
    std::map<DevicePath, VolumInfo> volum_total_;
    std::map<DevicePath, VolumInfo> volum_assigned_;
    std::set<std::string> port_assigned_;
    size_t port_total_;
    std::map<ContainerId, Container::Ptr> containers_;
};

class Scheduler {
public:
    explicit Scheduler();
    void AddAgent(Agent::Ptr agent);
    void RemoveAgent(const AgentEndpoint& endpoint);
    ContainerGroupId Submit(const Requirement& require, int replica);
    void Kill(const ContainerGroupId& group_id);
    void ScaleUp(const ContainerGroupId& group_id, int replica);
    void ScaleDown(const ContainerGroupId& group_id, int replica);
    //start the main schueduling loop
    void Start();
    //
    void ShowAssignment(const AgentEndpoint& endpoint,
                        std::vector<Container>& containers);
    void ShowContainerGroup(const ContainerGroupId group_id,
                            std::vector<Container>& containers);
    void ChangeStatus(const ContainerId& container_id, ContainerStatus status);

private:
    std::map<ContainerId, Container::Ptr> containers_;
    std::map<AgentEndpoint, Agent::Ptr> agent_assign_;
    std::map<ContainerGroupId, ContainerGroup::Ptr> container_groups_;
};

} //namespace sched
} //namespace galaxy
} //namespace baidu
