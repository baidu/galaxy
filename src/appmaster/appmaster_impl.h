// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <string>
#include <map>
#include <set>

#include "rpc/rpc_client.h"
#include "ins_sdk.h"
#include "src/protocol/appmaster.pb.h"


namespace baidu {
namespace galaxy {

typedef ::galaxy::ins::sdk::InsSDK InsSDK;
typedef::galaxy::ins::sdk::SDKError  SDKError;
typedef proto::PodDescription PodDescription;
typedef proto::TaskDescription TaskDescription;
typedef proto::ImagePackage ImagePackage;
typedef proto::DataPackage DataPackage;
typedef proto::Package Package;
typedef proto::PodInfo PodInfo;


class AppMasterImpl : public baidu::galaxy::proto::AppMaster {
public:

AppMasterImpl();
virtual ~AppMasterImpl();

void FetchTask(::google::protobuf::RpcController* controller,
               const::baidu::galaxy::proto::FetchTaskRequest* request,
               ::baidu::galaxy::proto::FetchTaskResponse* response,
               ::google::protobuf::Closure* done);

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

private:
    std::string endpoint_;
    InsSDK* nexus_;

};

} //namespace galaxy
} //namespace baidu

