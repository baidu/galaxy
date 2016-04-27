// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "appworker_impl.h"

#include <boost/bind.hpp>
#include <gflags/gflags.h>

#include "protocol/galaxy.pb.h"
#include "protocol/appmaster.pb.h"

DECLARE_int32(appworker_fetch_task_rpc_timeout);
DECLARE_int32(appworker_background_thread_pool_size);
DECLARE_string(appworker_container_id);
DECLARE_string(appmaster_nexus_path);
DECLARE_string(appmaster_endpoint);

namespace baidu {
namespace galaxy {

AppWorkerImpl::AppWorkerImpl() :
    mutex_appworker_(),
    container_id_(FLAGS_appworker_container_id),
    backgroud_thread_pool_(FLAGS_appworker_background_thread_pool_size),
    appmaster_endpoint_(FLAGS_appmaster_endpoint),
    appmaster_stub_(NULL) {

    if(!rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_) {
        LOG(ERROR) << "connect appmaster %s failed" << appmaster_endpoint_.c_str();
    }
}

AppWorkerImpl::~AppWorkerImpl () {
    backgroud_thread_pool_.Stop(false);
    LOG(INFO) << "appworker destroyed";
}

int AppWorkerImpl::Init() {
    LOG(INFO) << "AppWorkerImpl init.";
    backgroud_thread_pool_.AddTask(boost::bind(&AppWorkerImpl::FetchTask, this));

    return 0;
}

int AppWorkerImpl::RefreshAppMasterStub() {
    int ret = nexus_->Get(FLAGS_appmaster_nexus_path, &appmaster_endpoint_);
    if (ret != 0 ) {
        LOG(ERROR) << "get appmaster endpoint from nexus failed.";
    } else {
        LOG(INFO) << "get appmaster endpoint: " << appmaster_endpoint_;
        if(!rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_) {
            rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_);
            LOG(ERROR) << "connect appmaster %s failed" << appmaster_endpoint_.c_str();
        }
    }

    return ret;
}

void AppWorkerImpl::FetchTask () {
    MutexLock lock(&mutex_appworker_);
    LOG(INFO) << "fetch task called";

    proto::FetchTaskRequest* request = new proto::FetchTaskRequest;
    proto::FetchTaskResponse* response = new proto::FetchTaskResponse;
    request->set_containerid(container_id_);
    TaskInfoList* task_infos = new TaskInfoList;
    proto::TaskInfo* task_info = new proto::TaskInfo;
    task_infos->Add()->CopyFrom(*task_info);
    request->mutable_task_infos()->CopyFrom(*task_infos);

    boost::function<void (const proto::FetchTaskRequest*, proto::FetchTaskResponse*,
                          bool, int)> fetch_task_callback;
    fetch_task_callback = boost::bind(&AppWorkerImpl::FetchTaskCallback, this,
                                      _1, _2, _3, _4);

    rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_);
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
