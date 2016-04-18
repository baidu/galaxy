// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "appmaster_impl.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/utsname.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <snappy.h>

namespace baidu {
namespace galaxy {

AppMasterImpl::AppMasterImpl() {

}

AppMasterImpl::~AppMasterImpl() {

}

void AppMasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::SubmitJobRequest* request,
               ::baidu::galaxy::proto::SubmitJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::UpdateJobRequest* request,
               ::baidu::galaxy::proto::UpdateJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::RemoveJobRequest* request,
               ::baidu::galaxy::proto::RemoveJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ListJobs(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::ListJobsRequest* request,
               ::baidu::galaxy::proto::ListJobsResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ShowJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::ShowJobRequest* request,
               ::baidu::galaxy::proto::ShowJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ExecuteCmd(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::ExecuteCmdRequest* request,
                               ::baidu::galaxy::proto::ExecuteCmdResponse* response,
                               ::google::protobuf::Closure* done) {
}

}
}


