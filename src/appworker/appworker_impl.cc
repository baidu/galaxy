// Copyright (c) 2019, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "appworker_impl.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <gflags/gflags.h>
#include <timer.h>

#include "protocol/appmaster.pb.h"

DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_string(appmaster_nexus_path);
DECLARE_int32(appworker_fetch_task_timeout);
DECLARE_int32(appworker_fetch_task_interval);
DECLARE_int32(appworker_update_appmaster_stub_interval);
DECLARE_int32(appworker_background_thread_pool_size);
DECLARE_string(appworker_job_id);
DECLARE_string(appworker_endpoint);
DECLARE_string(appworker_container_id);

namespace baidu {
namespace galaxy {

AppWorkerImpl::AppWorkerImpl() :
        mutex_appworker_(),
        job_id_(FLAGS_appworker_job_id),
        container_id_(FLAGS_appworker_container_id),
        endpoint_(FLAGS_appworker_endpoint),
        start_time_(),
        nexus_(NULL),
        appmaster_stub_(NULL),
        backgroud_pool_(FLAGS_appworker_background_thread_pool_size) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_servers);
    backgroud_pool_.DelayTask(
        FLAGS_appworker_fetch_task_interval,
        boost::bind(&AppWorkerImpl::FetchTask, this)
    );
    backgroud_pool_.AddTask(boost::bind(&AppWorkerImpl::UpdateAppMasterStub, this));
}

AppWorkerImpl::~AppWorkerImpl () {
    if (NULL != nexus_) {
        delete nexus_;
    }
    if (NULL != appmaster_stub_) {
        delete appmaster_stub_;
    }
    backgroud_pool_.Stop(false);
}

void AppWorkerImpl::Start() {
    start_time_ = baidu::common::timer::get_micros();
    LOG(INFO) << "appworker start, job_id: " << job_id_.c_str()\
        << ", container_id: " << container_id_.c_str()\
        << ", endpoint: " << endpoint_.c_str();
}

void AppWorkerImpl::UpdateAppMasterStub() {
    MutexLock lock(&mutex_appworker_);
    SDKError err;
    std::string new_endpoint = "";
    std::string key = FLAGS_nexus_root_path + "/" + FLAGS_appmaster_nexus_path;
    bool ok = nexus_->Get(key, &new_endpoint, &err);
    do {
        if (!ok) {
           LOG(INFO) << "get appmaster endpoint from nexus failed: "\
               << ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str();
           break;
        }
        if (appmaster_endpoint_ == new_endpoint) {
           break;
        }
        LOG(INFO) << "appmaster endpoint updated, from: " << appmaster_endpoint_.c_str()\
           << ", to: " << new_endpoint.c_str();
        appmaster_endpoint_ = new_endpoint;
        if(rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_)) {
            LOG(INFO) << "appmaster stub updated, endpoint: " << appmaster_endpoint_.c_str();
        }
    } while (0);

    backgroud_pool_.DelayTask(
        FLAGS_appworker_update_appmaster_stub_interval,
        boost::bind(&AppWorkerImpl::UpdateAppMasterStub, this)
    );
}

void AppWorkerImpl::FetchTask () {
    MutexLock lock(&mutex_appworker_);
    if (NULL == appmaster_stub_) {
        LOG(INFO) << "appmaster stub is NULL, can not fetch task";
        backgroud_pool_.DelayTask(
            FLAGS_appworker_fetch_task_interval,
            boost::bind(&AppWorkerImpl::FetchTask, this)
        );
        return;
    }

    FetchTaskRequest* request = new FetchTaskRequest;
    FetchTaskResponse* response = new FetchTaskResponse;
    request->set_jobid(job_id_);
    request->set_endpoint(endpoint_);
    boost::function<void (const FetchTaskRequest*, FetchTaskResponse*, bool, int)> fetch_task_callback;
    fetch_task_callback = boost::bind(&AppWorkerImpl::FetchTaskCallback, this,
                                      _1, _2, _3, _4);
    rpc_client_.AsyncRequest(appmaster_stub_, &AppMaster_Stub::FetchTask,
                             request, response, fetch_task_callback,
                             FLAGS_appworker_fetch_task_timeout, 0);
}

void AppWorkerImpl::FetchTaskCallback(const FetchTaskRequest* request,
                                      FetchTaskResponse* response,
                                      bool failed, int /*error*/) {
    MutexLock lock(&mutex_appworker_);
    ErrorCode* error_code = response->mutable_error_code();
    LOG(WARNING) << "fetch task get status: " << proto::Status_Name(error_code->status()).c_str()\
        << " from appmaster";
    switch (error_code->status()) {
        case proto::kJobNotFound:
            exit(-1);
        case proto::kTerminate:
            pod_manager_.DeletePod();
            break;
        case proto::kOk:
            pod_manager_.CreatePod(response->mutable_pod());
            break;
        default:
            LOG(INFO) << "appworker fetch task ok, nope";
    }

    backgroud_pool_.DelayTask(
        FLAGS_appworker_fetch_task_interval,
        boost::bind(&AppWorkerImpl::FetchTask, this)
    );
}

}   // ending namespace galaxy
}   // ending namespace baidu
