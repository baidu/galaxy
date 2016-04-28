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

typedef std::string AgentEndpoint;
typedef std::string JobId;
typedef std::string ContainerId;
typedef std::string DevicePath;

enum ContainerStatus {
    kNotInit = 0,
    kPending = 1,
    kAllocating = 2,
    kRunning = 3,
    kDestroying = 4,
    kTerminated = 5
};

enum ResourceError {
    kOk = 0,
    kNoCpu = 1,
    kNoMemory = 2,
    kNoMedium = 3,
    kNoDevice = 4,
    kNoPort = 5,
    kPortConflict = 6,
    kLabelMismatch = 7,
    kNoMemoryForTmpfs = 8,
    kPoolMismatch = 9
};

struct Requirement {
    const std::string label;
    const std::string pool;
    proto::CpuRequired cpu;
    proto::MemoryRequired memory;
    std::vector<proto::VolumRequired> volums;
    std::vector<proto::PortRequired> ports;
    typedef boost::shared_ptr<Requirement> Ptr;
};

struct Container {
    ContainerId id;
    JobId job_id;
    ContainerStatus status;
    Requirement::Ptr require;
    std::vector<DevicePath> allocated_volums;
    std::set<std::string> allocated_port;
    AgentEndpoint allocated_agent;
    ResourceError last_res_err;
    Container() : status(kNotInit) {};
    typedef boost::shared_ptr<Container> Ptr;
};

typedef std::map<ContainerId, Container::Ptr> ContainerMap;

struct Job {
    JobId id;
    Requirement::Ptr require;
    int replica;
    std::map<ContainerId, Container::Ptr> containers;
    std::map<ContainerId, Container::Ptr> states[6];
    typedef boost::shared_ptr<Job> Ptr;
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
    void SetAssignment(int64_t cpu_assigned,
                       int64_t memory_assigned,
                       const std::map<DevicePath, VolumInfo>& volum_assigned,
                       const std::set<std::string> port_assigned);
    bool TryPut(const Container* container, ResourceError& err);
    void Put(Container::Ptr container);
    void Evict(Container::Ptr container);
    void AddLabel(const std::string& label);
    void RemoveLabel(const std::string& label);
    void SetPool(const std::string& pool);
    typedef boost::shared_ptr<Agent> Ptr;
private:
    bool SelectDevices(const std::vector<proto::VolumRequired>& volums,
                       std::vector<DevicePath>& devices);
    bool RecurSelectDevices(size_t i, const std::vector<proto::VolumRequired>& volums,
                            std::map<DevicePath, VolumInfo>& volum_free,
                            std::vector<DevicePath>& devices);
    AgentEndpoint endpoint_;
    std::set<std::string> labels_;
    std::string pool_;
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
    JobId Submit(const std::string& job_name,
                 const Requirement& require, 
                 int replica);
    void Kill(const JobId& job_id);
    void ScaleUpDown(const JobId& job_id, int replica);
    //start the main schueduling loop
    void Start();
    //
    void ShowAssignment(const AgentEndpoint& endpoint,
                        std::vector<Container>& containers);
    void ShowJob(const JobId job_id,
                 std::vector<Container>& containers);
    void ChangeStatus(const JobId& job_id,
                      const ContainerId& container_id, 
                      ContainerStatus new_status);

private:
    void ChangeStatus(Container::Ptr container,
                      ContainerStatus new_status);
    void ChangeStatus(Job::Ptr job,
                      Container::Ptr container,
                      ContainerStatus new_status);

    JobId GenerateJobId(const std::string& job_name);
    ContainerId GenerateContainerId(const JobId& job_id, int offset);
    void TryOneAgent(AgentEndpoint pre_endpoint);

    std::map<AgentEndpoint, Agent::Ptr> agents_;
    std::map<JobId, Job::Ptr> jobs_;
    Mutex mu_;
    ThreadPool pool_;
};

} //namespace sched
} //namespace galaxy
} //namespace baidu
