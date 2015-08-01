// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "master_impl.h"

namespace baidu {
namespace galaxy {

void MasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::SubmitJobRequest* request,
                           ::baidu::galaxy::SubmitJobResponse* response,
                           ::google::protobuf::Closure* done) {
    job_manager_.Add(request->job());
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
                         const ::baidu::galaxy::GetResourceSnapshotRequest*,
                         ::baidu::galaxy::GetResourceSnapshotResponse*,
                         ::google::protobuf::Closure* done) {
    controller->SetFailed("Method GetResourceSnapshot() not implemented.");
    done->Run();
}

void MasterImpl::Propose(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::ProposeRequest* request,
                         ::baidu::galaxy::ProposeResponse* response,
                         ::google::protobuf::Closure* done) {
    for (int i = 0; i < request->schedule_size(); i++) {
        const ScheduleInfo& sche_info = request->schedule(i);
        response->add_status(job_manager_.Propose(sche_info));
    }
    done->Run();
}

void MasterImpl::ListAgents(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::ListAgentsRequest*,
                            ::baidu::galaxy::ListAgentsResponse*,
                            ::google::protobuf::Closure* done) {
    controller->SetFailed("Method ListAgents() not implemented.");
    done->Run();
}

}
}
