// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  GALAXY_MASTER_IMPL_H_
#define  GALAXY_MASTER_IMPL_H_

#include "proto/master.pb.h"

#include <map>

#include "common/mutex.h"

namespace galaxy {

class Agent_Stub;
struct AgentInfo {
    int64_t id;
    std::string addr;
    int32_t task_num;
    Agent_Stub* stub;
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
private:
    std::map<std::string, AgentInfo> agents_;
    int64_t next_agent_id_;
    int64_t next_task_id_;
    Mutex agent_lock_;

    RpcClient* rpc_client_;
};

} // namespace galaxy

#endif  // GALAXY_MASTER_IMPL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
