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
               const ::baidu::galaxy::SubmitJobRequest* request,
               ::baidu::galaxy::SubmitJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::UpdateJobRequest* request,
               ::baidu::galaxy::UpdateJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::RemoveJobRequest* request,
               ::baidu::galaxy::RemoveJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ListJobs(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ListJobsRequest* request,
               ::baidu::galaxy::ListJobsResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ShowJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::ShowJobRequest* request,
               ::baidu::galaxy::ShowJobResponse* response,
               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::ExecuteCmd(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::ExecuteCmdRequest* request,
                               ::baidu::galaxy::ExecuteCmdResponse* response,
                               ::google::protobuf::Closure* done) {

}


}
}


