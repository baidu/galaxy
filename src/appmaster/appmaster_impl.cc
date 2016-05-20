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

#include <glog/logging.h>
DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_string(appmaster_nexus_path);
DECLARE_string(appmaster_endpoint);

namespace baidu {
namespace galaxy {

AppMasterImpl::AppMasterImpl() :
        endpoint_(FLAGS_appmaster_endpoint),
        nexus_(NULL) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_servers);
    SDKError err;
    bool ok = false;
    std::string key = FLAGS_nexus_root_path + "/" + FLAGS_appmaster_nexus_path;
    ok = nexus_->Put(key, endpoint_, &err);
    if (!ok) {
        LOG(INFO) << "appmaster write endpoint failed";
        abort();
    }
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

void AppMasterImpl::FetchTask(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::FetchTaskRequest* request,
                               ::baidu::galaxy::proto::FetchTaskResponse* response,
                               ::google::protobuf::Closure* done) {
    if (!request->has_podid()) {
        PodDescription* pod = response->mutable_pod();
        TaskDescription* task_desc = pod->add_tasks();
        task_desc->set_id("task-1");
        ImagePackage* image_package = task_desc->mutable_exe_package();
        image_package->set_start_cmd("echo $JOB_ID\"-\"$POD_ID\"-\"$TASK_ID && echo sleep 10 && exit 1");
        image_package->set_stop_cmd("");
        Package* e_package = image_package->mutable_package();
        e_package->set_src_path("ftp://yq01-ps-rtg0000.yq01:/home/galaxy/sandbox/galaxy/galaxy.tgz");
        e_package->set_dst_path("galaxy.tar.gz");
        e_package->set_version("123457");

        DataPackage* data_package = task_desc->mutable_data_package();
        Package* d_package = data_package->add_packages();
        d_package->set_src_path("ftp://yq01-ps-rtg0000.yq01:/home/galaxy/rtg_galaxy_test/galaxy");
        d_package->set_dst_path("galaxy");
        d_package->set_version("123456");
    }
    done->Run();
    return;
}

}
}
