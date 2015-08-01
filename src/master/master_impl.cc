// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "master_impl.h"
#include <gflags/gflags.h>
#include "master_util.h"
#include <logging.h>

DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_string(master_lock_path);
DECLARE_string(master_path);

namespace baidu {
namespace galaxy {

MasterImpl::MasterImpl() : nexus_(NULL) {
    nexus_ = new InsSDK(FLAGS_nexus_servers);
}

MasterImpl::~MasterImpl() {
    delete nexus_;
}

static void OnMasterLockChange(const ::galaxy::ins::sdk::WatchParam& param, 
                               ::galaxy::ins::sdk::SDKError error) {
    MasterImpl* master = static_cast<MasterImpl*>(param.context);
    master->OnLockChange(param.value);
}

static void OnMasterSessionTimeout(void* ctx) {
    MasterImpl* master = static_cast<MasterImpl*>(ctx);
    master->OnSessionTimeout();
}

void MasterImpl::OnLockChange(std::string lock_session_id) {
    std::string self_session_id = nexus_->GetSessionID();
    if (self_session_id != lock_session_id) {
        LOG(FATAL, "master lost lock , die.");
        abort();
    }
}

void MasterImpl::OnSessionTimeout() {
    LOG(FATAL, "master lost session with nexus, die.");
    abort();
}

void MasterImpl::AcquireMasterLock() {
    std::string master_lock = FLAGS_nexus_root_path + FLAGS_master_lock_path;
    ::galaxy::ins::sdk::SDKError err;
    nexus_->RegisterSessionTimeout(&OnMasterSessionTimeout, this);
    bool ret = nexus_->Lock(master_lock, &err); //whould block until accquired
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    std::string master_endpoint = MasterUtil::SelfEndpoint();
    std::string master_path_key = FLAGS_nexus_root_path + FLAGS_master_path;
    ret = nexus_->Put(master_path_key, master_endpoint, &err);
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    nexus_->Watch(master_lock, &OnMasterLockChange, this, &err);
    assert(ret && err == ::galaxy::ins::sdk::kOK);
    LOG(INFO, "master lock [ok].  %s -> %s", 
        master_path_key.c_str(), master_endpoint.c_str());
}

void MasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::SubmitJobRequest* request,
                           ::baidu::galaxy::SubmitJobResponse* response,
                           ::google::protobuf::Closure* done) {
    JobId job_id = job_manager_.Add(request->job());
    response->set_jobid(job_id);
    done->Run();
}

void MasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::UpdateJobRequest*,
                           ::baidu::galaxy::UpdateJobResponse*,
                           ::google::protobuf::Closure* done) {
    controller->SetFailed("Method UpdateJob() not implemented.");
    done->Run();
}

void MasterImpl::SuspendJob(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::SuspendJobRequest* request,
                            ::baidu::galaxy::SuspendJobResponse* response,
                            ::google::protobuf::Closure* done) {
    response->set_status(job_manager_.Suspend(request->jobid()));
    done->Run();
}

void MasterImpl::ResumeJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::ResumeJobRequest* request,
                           ::baidu::galaxy::ResumeJobResponse* response,
                           ::google::protobuf::Closure* done) {
    response->set_status(job_manager_.Resume(request->jobid()));
    done->Run();
}

void MasterImpl::TerminateJob(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::TerminateJobRequest*,
                              ::baidu::galaxy::TerminateJobResponse*,
                              ::google::protobuf::Closure* done) {
    controller->SetFailed("Method TerminateJob() not implemented.");
    done->Run();
}

void MasterImpl::ShowJob(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::ShowJobRequest*,
                         ::baidu::galaxy::ShowJobResponse*,
                         ::google::protobuf::Closure* done) {
    controller->SetFailed("Method ShowJob() not implemented.");
    done->Run();
}

void MasterImpl::ListJobs(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::ListJobsRequest*,
                          ::baidu::galaxy::ListJobsResponse*,
                          ::google::protobuf::Closure* done) {
    controller->SetFailed("Method ListJobs() not implemented.");
    done->Run();
}

void MasterImpl::HeartBeat(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::HeartBeatRequest* request,
                          ::baidu::galaxy::HeartBeatResponse*,
                          ::google::protobuf::Closure* done) {
    std::string agent_addr = request->endpoint();
    job_manager_.KeepAlive(agent_addr);
    done->Run();
}

void MasterImpl::GetPendingJobs(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::GetPendingJobsRequest*,
                                ::baidu::galaxy::GetPendingJobsResponse* response,
                                ::google::protobuf::Closure* done) {
    job_manager_.GetPendingPods(response->mutable_jobs());
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::GetResourceSnapshot(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::GetResourceSnapshotRequest* request,
                         ::baidu::galaxy::GetResourceSnapshotResponse* response,
                         ::google::protobuf::Closure* done) {
    job_manager_.GetAliveAgentsInfo(response->mutable_agents());
    response->set_status(kOk);
    done->Run();
}

void MasterImpl::Propose(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::ProposeRequest* request,
                         ::baidu::galaxy::ProposeResponse* response,
                         ::google::protobuf::Closure* done) {
    for (int i = 0; i < request->schedule_size(); i++) {
        const ScheduleInfo& sche_info = request->schedule(i);
        response->set_status(job_manager_.Propose(sche_info));
    }
    done->Run();
    job_manager_.DeployPod();
}

void MasterImpl::ListAgents(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::ListAgentsRequest* request,
                            ::baidu::galaxy::ListAgentsResponse* response,
                            ::google::protobuf::Closure* done) {
    job_manager_.GetAgentsInfo(response->mutable_agents());
    response->set_status(kOk);
    done->Run();
}

}
}
