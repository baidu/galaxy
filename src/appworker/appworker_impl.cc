// Copyright (c) 2019, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "appworker_impl.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <algorithm>
#include <cctype>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
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
DECLARE_string(appworker_endpoint_env);
DECLARE_string(appworker_job_id_env);
DECLARE_string(appworker_pod_id_env);
DECLARE_string(appworker_task_ids_env);
DECLARE_string(appworker_cgroup_subsystems_env);
// DECLARE_string(appworker_endpoint);
// DECLARE_string(appworker_job_id);
// DECLARE_string(appworker_pod_id);
// DECLARE_string(appworker_task_ids);

namespace baidu {
namespace galaxy {

AppWorkerImpl::AppWorkerImpl() :
    mutex_appworker_(),
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

void AppWorkerImpl::PrepareEnvs() {
    // 1.job_id
    char* c_job_id = getenv(FLAGS_appworker_job_id_env.c_str());
    if (NULL == c_job_id) {
        LOG(WARNING) << FLAGS_appworker_job_id_env << " is  not set";
        exit(-1);
    }
    job_id_ = std::string(c_job_id);
    // 2.pod_id
    char* c_pod_id = getenv(FLAGS_appworker_pod_id_env.c_str());
    if (NULL == c_pod_id) {
        LOG(WARNING) << FLAGS_appworker_pod_id_env << " is  not set";
        exit(-1);
    }
    pod_id_ = std::string(c_pod_id);
    // 3.endpoint
    char* c_endpoint = getenv(FLAGS_appworker_endpoint_env.c_str());
    if (NULL == c_endpoint) {
        LOG(WARNING) << FLAGS_appworker_endpoint_env << " is  not set";
        exit(-1);
    }
    endpoint_ = std::string(c_endpoint);
    // 4.task_ids
    char* c_task_ids = getenv(FLAGS_appworker_task_ids_env.c_str());
    if (NULL == c_task_ids) {
        LOG(WARNING) << FLAGS_appworker_task_ids_env << " is  not set";
        exit(-1);
    }
    std::string s_task_ids = std::string(c_task_ids);
    boost::split(task_ids_,
                 s_task_ids,
                 boost::is_any_of(","),
                 boost::token_compress_on);

    // 5.cgroup subsystems
    char* c_cgroup_subsystems = getenv(FLAGS_appworker_cgroup_subsystems_env.c_str());
    if (NULL == c_cgroup_subsystems) {
        LOG(WARNING) << FLAGS_appworker_cgroup_subsystems_env << "is not set";
        exit(-1);
    }
    std::string s_cgroup_subsystems = std::string(c_cgroup_subsystems);
    boost::split(cgroup_subsystems_,
                 s_cgroup_subsystems,
                 boost::is_any_of(","),
                 boost::token_compress_on);
    // 6.task cgroup paths
    std::vector<std::string>::iterator t_it = task_ids_.begin();
    for (; t_it != task_ids_.end(); t_it++) {
        std::map<std::string, std::string> cgroup_paths;
        std::vector<std::string>::iterator c_it = cgroup_subsystems_.begin();
        for (; c_it != cgroup_subsystems_.end(); ++c_it) {
            std::string key = "BAIDU_GALAXY_CONTAINER_" + *t_it + "_" + *c_it + "_PATH";
            transform(key.begin(), key.end(), key.begin(), toupper);
            char* c_value = getenv(key.c_str());
            if (NULL == c_value) {
                LOG(WARNING) << key << " is  not set";
                exit(-1);
            }
            std::string value = std::string(c_value);
            cgroup_paths.insert(std::make_pair(*c_it, value));
        }
        task_cgroup_paths_.push_back(cgroup_paths);
    }
}

void AppWorkerImpl::Start() {
    start_time_ = baidu::common::timer::get_micros();
    LOG(INFO) << "appworker start, endpoint: " << endpoint_\
        << ", job_id: " <<job_id_\
        << ", pod_id: " << pod_id_;
    PrepareEnvs();
}

void AppWorkerImpl::UpdateAppMasterStub() {
    MutexLock lock(&mutex_appworker_);
    SDKError err;
    std::string new_endpoint = "";
    std::string key = FLAGS_nexus_root_path + "/" + FLAGS_appmaster_nexus_path;
    bool ok = nexus_->Get(key, &new_endpoint, &err);
    do {
        if (!ok) {
           LOG(WARNING) << "get appmaster endpoint from nexus failed: "\
               << ::galaxy::ins::sdk::InsSDK::StatusToString(err);
           break;
        }
        if (appmaster_endpoint_ == new_endpoint) {
           break;
        }
        appmaster_endpoint_ = new_endpoint;
        if(rpc_client_.GetStub(appmaster_endpoint_, &appmaster_stub_)) {
            LOG(INFO) << "appmaster stub updated, endpoint: " << appmaster_endpoint_;
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
    switch (error_code->status()) {
        case proto::kJobNotFound:
            exit(-1);
        case proto::kTerminate:
            pod_manager_.DeletePod();
            break;
        case proto::kOk:
        {
            PodEnv env;
            env.job_id = job_id_;
            env.pod_id = pod_id_;
            env.task_ids = task_ids_;
            env.cgroup_subsystems = cgroup_subsystems_;
            env.task_cgroup_paths = task_cgroup_paths_;
            pod_manager_.CreatePod(env, response->pod());
            break;
        }
        default:
            break;
    }

    backgroud_pool_.DelayTask(
        FLAGS_appworker_fetch_task_interval,
        boost::bind(&AppWorkerImpl::FetchTask, this)
    );
}

}   // ending namespace galaxy
}   // ending namespace baidu
