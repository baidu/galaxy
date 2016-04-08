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

void AssignTask(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::AssignTaskRequest* request,
               ::baidu::galaxy::AssignTaskResponse* response,
               ::google::protobuf::Closure* done);

void ListTasks(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ListTasksRequest* request,
               ::baidu::galaxy::ListTasksResponse* response,
               ::google::protobuf::Closure* done);

void StartTask(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::StartTaskRequest* request,
               ::baidu::galaxy::StartTaskResponse* response,
               ::google::protobuf::Closure* done);

void StopTask(::google::protobuf::RpcController* controller,
              const ::baidu::galaxy::StopTaskRequest* request,
              ::baidu::galaxy::StopTaskResponse* response,
              ::google::protobuf::Closure* done);

void UpdateTask(::google::protobuf::RpcController* controller,
                const ::baidu::galaxy::UpdateTaskRequest* request,
                ::baidu::galaxy::UpdateTaskResponse* response,
                ::google::protobuf::Closure* done);

void ExecuteCmd(::google::protobuf::RpcController* controller,
                const ::baidu::galaxy::ExecuteCmdRequest* request,
                ::baidu::galaxy::ExecuteCmdResponse* response,
                ::google::protobuf::Closure* done);

};

} //namespace galaxy
} //namespace baidu

