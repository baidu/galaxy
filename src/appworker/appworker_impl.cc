// Copyright (c) 2019, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "appworker_impl.h"

#include <boost/bind.hpp>
#include <gflags/gflags.h>

#include "protocol/galaxy.pb.h"
#include "protocol/appmaster.pb.h"

DECLARE_string(nexus_servers);
DECLARE_int32(appworker_fetch_task_rpc_timeout);
DECLARE_int32(appworker_background_thread_pool_size);
DECLARE_string(appworker_job_id);
DECLARE_string(appworker_pod_id);
DECLARE_string(appmaster_nexus_path);

namespace baidu {
namespace galaxy {

AppWorkerImpl::AppWorkerImpl() :
    mutex_appworker_(),
    job_id_(FLAGS_appworker_job_id),
    pod_id_(FLAGS_appworker_pod_id),
    nexus_(NULL),
    appmaster_stub_(NULL),
    backgroud_thread_pool_(FLAGS_appworker_background_thread_pool_size) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_servers);
}

AppWorkerImpl::~AppWorkerImpl () {
    backgroud_thread_pool_.Stop(false);
    LOG(INFO) << "appworker destroyed";
}

int AppWorkerImpl::Init() {
    if (RefreshAppMasterStub() != 0) {
        return -1;
    }

    backgroud_thread_pool_.AddTask(boost::bind(&AppWorkerImpl::FetchTask, this));
    return 0;
}

int AppWorkerImpl::RefreshAppMasterStub() {
    MutexLock lock(&mutex_appworker_);
    ::galaxy::ins::sdk::SDKError err;
    std::string appmaster_endpoint = "";
    LOG(INFO) << "appmaster_nexus_path: " << FLAGS_appmaster_nexus_path;
    bool ok = nexus_->Get(FLAGS_appmaster_nexus_path, &appmaster_endpoint, &err);
    if (!ok) {
        LOG(ERROR) << "get appmaster endpoint from nexus failed: " \
            << ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str();
        return -1;
    } else {
        LOG(INFO) << "get appmaster endpoint: " << appmaster_endpoint;
        if(!rpc_client_.GetStub(appmaster_endpoint, &appmaster_stub_)) {
            LOG(ERROR) << "connect appmaster %s failed" << appmaster_endpoint.c_str();
            return -1;
        }
    }

    return 0;
}

void AppWorkerImpl::FetchTask () {
    MutexLock lock(&mutex_appworker_);
    proto::FetchTaskRequest* request = new proto::FetchTaskRequest;
    proto::FetchTaskResponse* response = new proto::FetchTaskResponse;
    request->set_jobid(job_id_);
    request->set_podid(pod_id_);
    TaskInfoList* task_infos = new TaskInfoList;
    proto::TaskInfo* task_info = new proto::TaskInfo;
    task_infos->Add()->CopyFrom(*task_info);
    request->mutable_task_infos()->CopyFrom(*task_infos);

    boost::function<void (const proto::FetchTaskRequest*, proto::FetchTaskResponse*, bool, int)> fetch_task_callback;
    fetch_task_callback = boost::bind(&AppWorkerImpl::FetchTaskCallback, this,
                                      _1, _2, _3, _4);

    rpc_client_.AsyncRequest(appmaster_stub_, &proto::AppMaster_Stub::FetchTask,
                             request, response, fetch_task_callback,
                             FLAGS_appworker_fetch_task_rpc_timeout, 0);
}

void AppWorkerImpl::FetchTaskCallback(const proto::FetchTaskRequest* request,
                                      proto::FetchTaskResponse* response,
                                      bool failed, int /*error*/) {
    MutexLock lock(&mutex_appworker_);
    proto::Status status = response->status();
    if (failed || status != proto::kOk) {
        LOG(INFO) << "fetch task fail: " << status;
        backgroud_thread_pool_.DelayTask(2000,
           boost::bind(&AppWorkerImpl::FetchTask, this));
    }

}


}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
