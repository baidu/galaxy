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

#include <timer.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <snappy.h>

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

    // write endpoint to nexus
    SDKError err;
    bool ok = false;
    std::string key = FLAGS_nexus_root_path + "/" + FLAGS_appmaster_nexus_path;
    ok = nexus_->Put(key, endpoint_, &err);
    if (ok) {
        LOG(INFO) << "appmaster write endpoint to nexus ok: " << key.c_str();
    } else {
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
        LOG(INFO) << "worker first fetch";

        response->set_action(proto::kRunPod);
        PodInfo* pod_info = response->mutable_info();
        pod_info->set_jobid("test_job");
        pod_info->set_podid("test_job-1");
        pod_info->set_version("1");
        pod_info->set_start_time(baidu::common::timer::get_micros());
        PodDescription* pod_desc = pod_info->mutable_desc();
        TaskDescription* task_desc = pod_desc->add_tasks();
        task_desc->set_id("task-1");
        ImagePackage* image_package = task_desc->mutable_exe_package();
        Package* package = image_package->mutable_package();
        package->set_source_path("ftp://yq01-ps-rtg0000.yq01:/home/galaxy/rtg_galaxy_test/galaxy.tar.gz");
        package->set_dest_path("./");
        package->set_version("1");
        image_package->set_start_cmd("sleep 3000");
        image_package->set_stop_cmd("");
    }

    done->Run();
    return;
}


}
}


