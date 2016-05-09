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
#include "mutex.h"
#include "thread_pool.h"

namespace baidu {
namespace galaxy {
namespace sched {

using proto::ContainerStatus;
using proto::kContainerPending;
using proto::kContainerAllocating;
using proto::kContainerReady;
using proto::kContainerError;
using proto::kContainerDestroying;
using proto::kContainerTerminated;

typedef std::string AgentEndpoint;
typedef std::string GroupId;
typedef std::string ContainerId;
typedef std::string DevicePath;

enum ResourceError {
    kOk = 0,
    kNoCpu = 1,
    kNoMemory = 2,
    kNoMedium = 3,
    kNoDevice = 4,
    kNoPort = 5,
    kPortConflict = 6,
    kTagMismatch = 7,
    kNoMemoryForTmpfs = 8,
    kPoolMismatch = 9,
    kTooManyPods = 10
};

struct Requirement {
    const std::string tag;
    const std::set<std::string> pool_names;
    int max_per_host;
    proto::CpuRequired cpu;
    proto::MemoryRequired memory;
    std::vector<proto::VolumRequired> volums;
    std::vector<proto::PortRequired> ports;
    Requirement() : max_per_host(0) {};
    typedef boost::shared_ptr<Requirement> Ptr;
};

struct Container {
    ContainerId id;
    GroupId group_id;
    int priority;
    proto::ContainerStatus status;
    Requirement::Ptr require;
    std::vector<DevicePath> allocated_volums;
    std::set<std::string> allocated_port;
    AgentEndpoint allocated_agent;
    ResourceError last_res_err;
    typedef boost::shared_ptr<Container> Ptr;
};

struct ContainerPriorityLess{
    bool operator() (const Container::Ptr& a, const Container::Ptr& b) {
        return a->priority < b->priority;
    }
};

typedef std::map<ContainerId, Container::Ptr> ContainerMap;

struct Group {
    GroupId id;
    Requirement::Ptr require;
    int priority; //lower one is important
    bool terminated;
    std::map<ContainerId, Container::Ptr> containers;
    std::map<ContainerId, Container::Ptr> states[7];
    int update_interval;
    int last_update_time;
    Group() : terminated(false) {};
    int Replica() const {
        return states[kContainerPending].size() 
               + states[kContainerAllocating].size() 
               + states[kContainerReady].size();
    }
    typedef boost::shared_ptr<Group> Ptr;
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
                   const std::set<std::string>& tags,
                   const std::string& pool_name);
    void SetAssignment(int64_t cpu_assigned,
                       int64_t memory_assigned,
                       const std::map<DevicePath, VolumInfo>& volum_assigned,
                       const std::set<std::string> port_assigned,
                       const std::map<ContainerId, Container::Ptr>& containers);
    void SetAssignment(const proto::AgentInfo& agent_info);
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
    std::set<std::string> tags_;
    std::string pool_name_;
    int64_t cpu_total_;
    int64_t cpu_assigned_;
    int64_t memory_total_;
    int64_t memory_assigned_;
    std::map<DevicePath, VolumInfo> volum_total_;
    std::map<DevicePath, VolumInfo> volum_assigned_;
    std::set<std::string> port_assigned_;
    size_t port_total_;
    std::map<ContainerId, Container::Ptr> containers_;
    std::map<GroupId, int> container_counts_;
};

struct GroupQueueLess {
    bool operator () (const Group::Ptr& a, const Group::Ptr& b) {
        if (a->priority < b->priority) {
            return true;
        } else if (a->priority == b->priority) {
            return a->id < b->id;
        } else {
            return false;
        }
    }
};

class Scheduler {
public:
    explicit Scheduler();
    //start the main schueduling loop
    void Start();

    void AddAgent(Agent::Ptr agent);
    void RemoveAgent(const AgentEndpoint& endpoint);
    GroupId Submit(const std::string& group_name,
                 const Requirement& require, 
                 int replica, int priority);
    bool Kill(const GroupId& group_id);
    bool ManualSchedule(const AgentEndpoint& endpoint,
                        const GroupId& group_id);

    bool ChangeReplica(const GroupId& group_id, int replica);
    
    // @update_interval : 
    //      --- intervals between updateing two containers, in seconds
    bool Update(const GroupId& group_id,
                const Requirement& require,
                int update_interval);

    void ShowAssignment(const AgentEndpoint& endpoint,
                        std::vector<Container>& containers);
    void ShowGroup(const GroupId group_id,
                   std::vector<Container>& containers);
    void ChangeStatus(const GroupId& group_id,
                      const ContainerId& container_id, 
                      proto::ContainerStatus new_status);
    void AddTag(const AgentEndpoint& endpoint, const std::string& tag);
    void RemoveTag(const AgentEndpoint& endpoint, const std::string& tag);
    void SetPool(const AgentEndpoint& endpoint, const std::string& pool_name);

private:
    void ChangeStatus(Container::Ptr container,
                      proto::ContainerStatus new_status);
    void ChangeStatus(Group::Ptr group,
                      Container::Ptr container,
                      proto::ContainerStatus new_status);
    void ScaleDown(Group::Ptr group, int replica);
    void ScaleUp(Group::Ptr group, int replica);

    GroupId GenerateGroupId(const std::string& group_name);
    ContainerId GenerateContainerId(const GroupId& group_id, int offset);
    void ScheduleNextAgent(AgentEndpoint pre_endpoint);
    void CheckTagAndPool(Agent::Ptr agent);
    void CheckVersion(Agent::Ptr agent);
    bool CheckTagAndPoolOnce(Agent::Ptr agent, Container::Ptr container);
    void CheckGroupGC(Group::Ptr group);
    bool RequireHasDiff(const Requirement* v1, const Requirement* v2);

    std::map<AgentEndpoint, Agent::Ptr> agents_;
    std::map<GroupId, Group::Ptr> groups_;
    std::set<Group::Ptr, GroupQueueLess> group_queue_;
    Mutex mu_;
    ThreadPool sched_pool_;
    ThreadPool gc_pool_;
};

} //namespace sched
} //namespace galaxy
} //namespace baidu
