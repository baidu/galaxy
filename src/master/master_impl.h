// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  GALAXY_MASTER_IMPL_H_
#define  GALAXY_MASTER_IMPL_H_

#include "proto/master.pb.h"

#include <map>
#include <queue>
#include <set>

#include "common/mutex.h"
#include "common/thread_pool.h"

namespace galaxy {

class Agent_Stub;
struct AgentInfo {
    int64_t id;
    std::string addr;
    int32_t task_num;
    Agent_Stub* stub;
    int32_t alive_timestamp;
    std::set<int64_t> running_tasks;
};

struct JobInfo {
    int64_t id;
    int32_t replica_num;
    std::string job_name;
    std::string job_raw;
    std::string cmd_line;
    int32_t running_num;
    std::map<std::string,int> running_agents;
};

class RpcClient;

class MasterImpl : public Master {
public:
    MasterImpl();
    ~MasterImpl() {}
public:
    void HeartBeat(::google::protobuf::RpcController* controller,
                   const ::galaxy::HeartBeatRequest* request,
                   ::galaxy::HeartBeatResponse* response,
                   ::google::protobuf::Closure* done);
    void NewTask(::google::protobuf::RpcController* controller,
                 const ::galaxy::NewTaskRequest* request,
                 ::galaxy::NewTaskResponse* response,
                 ::google::protobuf::Closure* done);

    void TerminateTask(::google::protobuf::RpcController* controller,
                       const ::galaxy::TerminateTaskRequest* request,
                       ::galaxy::TerminateTaskResponse* response,
                       ::google::protobuf::Closure* done);
    void ListJob(::google::protobuf::RpcController* controller,
                 const ::galaxy::ListJobRequest* request,
                 ::galaxy::ListJobResponse* response,
                 ::google::protobuf::Closure* done);

    void ListTask(::google::protobuf::RpcController* controller,
                 const ::galaxy::ListTaskRequest* request,
                 ::galaxy::ListTaskResponse* response,
                 ::google::protobuf::Closure* done);
private:
    void DeadCheck();
    void Schedule();
    std::string AllocResource();
    bool ScheduleTask(JobInfo* job, const std::string& agent_addr);
    void UpdateJobsOnAgent(AgentInfo* agent,
                           const std::set<int64_t>& running_tasks,
                           bool clear_all = false);
private:
    common::ThreadPool thread_pool_;
    std::map<std::string, AgentInfo> agents_;
    std::map<int64_t, TaskInstance> tasks_;
    std::map<int64_t, JobInfo> jobs_;
    std::map<int32_t, std::set<std::string> > alives_;
    int64_t next_agent_id_;
    int64_t next_task_id_;
    int64_t next_job_id_;
    Mutex agent_lock_;

    RpcClient* rpc_client_;
};

} // namespace galaxy

#endif  // GALAXY_MASTER_IMPL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
