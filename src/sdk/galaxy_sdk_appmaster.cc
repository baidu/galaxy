// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_sdk_util.h"
#include "rpc/rpc_client.h"
#include "ins_sdk.h"
#include <gflags/gflags.h>
#include "galaxy_sdk_appmaster.h"

//nexus
DECLARE_string(nexus_addr);
DECLARE_string(nexus_root);
DECLARE_string(appmaster_path);

namespace baidu {
namespace galaxy {

class RpcClient;

namespace sdk {

AppMaster::AppMaster() : rpc_client_(NULL),
                       appmaster_stub_(NULL) {
    full_key_ = FLAGS_nexus_root + FLAGS_appmaster_path; 
	rpc_client_ = new RpcClient();
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr); 
}

AppMaster::~AppMaster() {
	delete rpc_client_;
    if (NULL != appmaster_stub_) {
        delete appmaster_stub_;
    }
    delete nexus_;
}

bool AppMaster::GetStub() {
    std::string endpoint;
    ::galaxy::ins::sdk::SDKError err;
    bool ok = nexus_->Get(full_key_, &endpoint, &err);
    if (!ok || err != ::galaxy::ins::sdk::kOK) {
        fprintf(stderr, "get appmaster endpoint from nexus failed: %s\n", 
                ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
        return false;
    }
    if(!rpc_client_->GetStub(endpoint, &appmaster_stub_)) {
        fprintf(stderr, "connect appmaster fail, appmaster: %s\n", endpoint.c_str());
        return false;
    }
    return true;
}

bool AppMaster::SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response) {
    ::baidu::galaxy::proto::SubmitJobRequest pb_request;
    ::baidu::galaxy::proto::SubmitJobResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    pb_request.set_hostname(request.hostname);
    FillJobDescription(request.job, pb_request.mutable_job());
    bool ok = rpc_client_->SendRequest(appmaster_stub_, 
                                        &::baidu::galaxy::proto::AppMaster_Stub::SubmitJob,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }

    response->jobid = pb_response.jobid();
    return true;
}

bool AppMaster::UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response) {
    ::baidu::galaxy::proto::UpdateJobRequest pb_request;
    ::baidu::galaxy::proto::UpdateJobResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    FillJobDescription(request.job, pb_request.mutable_job());
    pb_request.set_hostname(request.hostname);
    pb_request.set_jobid(request.jobid);
    bool ok = rpc_client_->SendRequest(appmaster_stub_,
                                        &::baidu::galaxy::proto::AppMaster_Stub::UpdateJob,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }

    return true;
}

bool AppMaster::StopJob(const StopJobRequest& request, StopJobResponse* response) {
    ::baidu::galaxy::proto::StopJobRequest pb_request;
    ::baidu::galaxy::proto::StopJobResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    pb_request.set_jobid(request.jobid);
    pb_request.set_hostname(request.hostname);

    bool ok = rpc_client_->SendRequest(appmaster_stub_,
                                        &::baidu::galaxy::proto::AppMaster_Stub::StopJob,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }
    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }
    return true;
}

bool AppMaster::RemoveJob(const RemoveJobRequest& request, RemoveJobResponse* response) {
    ::baidu::galaxy::proto::RemoveJobRequest pb_request;
    ::baidu::galaxy::proto::RemoveJobResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    pb_request.set_jobid(request.jobid);
    pb_request.set_hostname(request.hostname);
    bool ok = rpc_client_->SendRequest(appmaster_stub_, 
                                        &::baidu::galaxy::proto::AppMaster_Stub::RemoveJob,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }
    return true;
}

bool AppMaster::ListJobs(const ListJobsRequest& request, ListJobsResponse* response) {
    ::baidu::galaxy::proto::ListJobsRequest pb_request;
    ::baidu::galaxy::proto::ListJobsResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    bool ok = rpc_client_->SendRequest(appmaster_stub_,
                                        &::baidu::galaxy::proto::AppMaster_Stub::ListJobs,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "Appmaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }

    for (int i = 0; i < pb_response.jobs().size(); ++i) {
        const ::baidu::galaxy::proto::JobOverview& pb_job = pb_response.jobs(i); 
        JobOverview job;
        job.jobid = pb_job.jobid();
        job.running_num = pb_job.running_num();
        job.pending_num = pb_job.pending_num();
        job.deploying_num = pb_job.deploying_num();
        job.death_num = pb_job.death_num();
        job.fail_count = pb_job.fail_count();
        job.create_time = pb_job.create_time();
        job.update_time = pb_job.update_time();
        job.status = (JobStatus)pb_job.status();
        PbJobDescription2SdkJobDescription(pb_job.desc(), &job.desc);
        response->jobs.push_back(job);
    }
    return true;
}

bool AppMaster::ShowJob(const ShowJobRequest& request, ShowJobResponse* response) {
    ::baidu::galaxy::proto::ShowJobRequest pb_request;
    ::baidu::galaxy::proto::ShowJobResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    pb_request.set_jobid(request.jobid);
    bool ok = rpc_client_->SendRequest(appmaster_stub_,
                                        &::baidu::galaxy::proto::AppMaster_Stub::ShowJob,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }

    response->job.jobid = pb_response.job().jobid();
    response->job.version = pb_response.job().version();
    response->job.create_time = pb_response.job().create_time();
    response->job.update_time = pb_response.job().update_time();
    response->job.status = (JobStatus)pb_response.job().status();
    PbJobDescription2SdkJobDescription(pb_response.job().desc(), &response->job.desc);
    for (int32_t i = 0; i < pb_response.job().pods().size(); ++i) {
        PodInfo pod;
        pod.podid = pb_response.job().pods(i).podid();
        pod.jobid = pb_response.job().pods(i).jobid();
        pod.endpoint = pb_response.job().pods(i).endpoint();
        pod.status = (PodStatus)pb_response.job().pods(i).status();
        pod.version = pb_response.job().pods(i).version();
        pod.start_time = pb_response.job().pods(i).start_time();
        pod.update_time = pb_response.job().pods(i).update_time();
        pod.fail_count = pb_response.job().pods(i).fail_count();
        response->job.pods.push_back(pod);
    }

    return true;
}

bool AppMaster::ExecuteCmd(const ExecuteCmdRequest& request, ExecuteCmdResponse* response) {
    ::baidu::galaxy::proto::ExecuteCmdRequest pb_request;
    ::baidu::galaxy::proto::ExecuteCmdResponse pb_response;
    FillUser(request.user, pb_request.mutable_user());
    pb_request.set_jobid(request.jobid);
    pb_request.set_cmd(request.cmd);
    bool ok = rpc_client_->SendRequest(appmaster_stub_,
                                        &::baidu::galaxy::proto::AppMaster_Stub::ExecuteCmd,
                                        &pb_request, &pb_response, 5, 1);
    if (!ok) {
        response->error_code.reason = "AppMaster Rpc SendRequest failed";
        return false;
    }

    response->error_code.status = (Status)pb_response.error_code().status();
    response->error_code.reason = pb_response.error_code().reason();
    if (response->error_code.status != kOk) {
        return false;
    }

    return true;
}


} //end namespace sdk
} //end namespace galaxy
} // end namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
