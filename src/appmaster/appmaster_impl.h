// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <string>
#include <map>
#include <set>

#include "src/protocol/appmaster.pb.h"

namespace baidu {
namespace galaxy {

class AppMasterImpl : public baidu::galaxy::proto::AppMaster {
public:

    AppMasterImpl();
    virtual ~AppMasterImpl();
    void Init();
    void Start();
    void SubmitJob(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::SubmitJobRequest* request,
                  ::baidu::galaxy::proto::SubmitJobResponse* response,
                  ::google::protobuf::Closure* done);

    void UpdateJob(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::UpdateJobRequest* request,
                  ::baidu::galaxy::proto::UpdateJobResponse* response,
                  ::google::protobuf::Closure* done);

    void RemoveJob(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::RemoveJobRequest* request,
                  ::baidu::galaxy::proto::RemoveJobResponse* response,
                  ::google::protobuf::Closure* done);

    void ListJobs(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::ListJobsRequest* request,
                  ::baidu::galaxy::proto::ListJobsResponse* response,
                  ::google::protobuf::Closure* done);

    void ShowJob(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::ShowJobRequest* request,
                  ::baidu::galaxy::proto::ShowJobResponse* response,
                  ::google::protobuf::Closure* done);

    void ExecuteCmd(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::ExecuteCmdRequest* request,
                   ::baidu::galaxy::proto::ExecuteCmdResponse* response,
                   ::google::protobuf::Closure* done);

    void FetchTask(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::FecthTaskRequest* request,
                  ::baidu::galaxy::proto::FetchTaskResponse* response,
                  ::google::protobuf::Closure* done);
private:
    JobManager job_manager_;
    RpcClient* rpc_client;
    InsSDK *nexus_;
    std::string resman__endpoint_;
    RMWatcher* resman_watcher_;
    Mutex resman_mutex_;
    ResourceManager_Stub* resman_;
};

} //namespace galaxy
} //namespace baidu

