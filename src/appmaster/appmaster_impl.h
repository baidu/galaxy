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

class AppMasterImpl : public AppMaster {
public:

AppMasterImpl();
virtual ~AppMasterImpl();

void SubmitJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::SubmitJobRequest* request,
               ::baidu::galaxy::SubmitJobResponse* response,
               ::google::protobuf::Closure* done);

void UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::UpdateJobRequest* request,
               ::baidu::galaxy::UpdateJobResponse* response,
               ::google::protobuf::Closure* done);

void RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::RemoveJobRequest* request,
               ::baidu::galaxy::RemoveJobResponse* response,
               ::google::protobuf::Closure* done);

void ListJobs(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ListJobsRequest* request,
               ::baidu::galaxy::ListJobsResponse* response,
               ::google::protobuf::Closure* done);

void ShowJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ShowJobRequest* request,
               ::baidu::galaxy::ShowJobResponse* response,
               ::google::protobuf::Closure* done);

void ExecuteCmd(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::ExecuteCmdRequest* request,
                               ::baidu::galaxy::ExecuteCmdResponse* response,
                               ::google::protobuf::Closure* done);

};

} //namespace galaxy
} //namespace baidu

