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
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <snappy.h>

DECLARE_string(nexus_root);
DECLARE_string(nexus_addr);
DECLARE_string(appworker_cmdline);

const std::string sMASTERLock = "/appmaster_lock";
const std::string sMASTERAddr = "/appmaster";
const std::string sRESMANPath = "/resman";

namespace baidu {
namespace galaxy {

AppMasterImpl::AppMasterImpl() : running_(false) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr);
    resman_watcher_ = new baidu::galaxy::Watcher();
}

AppMasterImpl::~AppMasterImpl() {
    //delete resman_watcher_;
    delete nexus_;
}

void AppMasterImpl::Init() {
    if (!resman_watcher_->Init(boost::bind(&AppMasterImpl::HandleResmanChange, this, _1),
                                        FLAGS_nexus_addr, FLAGS_nexus_root, sRESMANPath)) {
        LOG(FATAL) << "init res manager watch failed, agent will exit ...";
        exit(1);
    }
    LOG(INFO) << "init resource manager watcher successfully";
    running_ = true;
    return;
}

void AppMasterImpl::HandleResmanChange(const std::string& new_endpoint) {
    if (new_endpoint.empty()) {
        LOG(WARNING) << "endpoint of RM is deleted from nexus";
    }
    if (new_endpoint != resman_endpoint_) {
        MutexLock lock(&resman_mutex_);
        LOG(INFO) << "RM changes to " << new_endpoint.c_str();
        resman_endpoint_ = new_endpoint;
        job_manager_.SetResmanEndpoint(resman_endpoint_);
    }
    return;
}

void AppMasterImpl::OnMasterLockChange(const ::galaxy::ins::sdk::WatchParam& param,
                                ::galaxy::ins::sdk::SDKError err) {
    AppMasterImpl* impl = static_cast<AppMasterImpl*>(param.context);
    impl->OnLockChange(param.value);
}

void AppMasterImpl::OnLockChange(std::string lock_session_id) {
    std::string self_session_id = nexus_->GetSessionID();
    if (self_session_id != lock_session_id) {
        LOG(FATAL) << "master lost lock , die.";
        abort();
    }
}

bool AppMasterImpl::RegisterOnNexus(const std::string endpoint) {
    ::galaxy::ins::sdk::SDKError err;
    bool ret = nexus_->Lock(FLAGS_nexus_root + sMASTERLock, &err);
    if (!ret) {
        LOG(WARNING) << "failed to acquire appmaster lock, " << err;
        return false;
    }
    ret = nexus_->Put(FLAGS_nexus_root + sMASTERAddr, endpoint, &err);
    if (!ret) {
        LOG(WARNING) << "failed to write appmaster endpoint to nexus, " << err;
        return false;
    }
    ret = nexus_->Watch(FLAGS_nexus_root + sMASTERLock, &OnMasterLockChange, this, &err);
    if (!ret) {
        LOG(WARNING) << "failed to watch appmaster lock, " << err;
        return false;
    }
    return true;
}

void AppMasterImpl::CreateContainerGroupCallBack(JobDescription job_desc,
                                         proto::SubmitJobResponse* submit_response,
                                         ::google::protobuf::Closure* done,
                                         const proto::CreateContainerGroupRequest* request,
                                         proto::CreateContainerGroupResponse* response,
                                         bool failed, int err) {
    boost::scoped_ptr<const baidu::galaxy::proto::CreateContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<baidu::galaxy::proto::CreateContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != proto::kOk) {
        //LOG(WARNING, "fail to create container group");
        submit_response->mutable_error_code()->CopyFrom(response_ptr->error_code());
        done->Run();
        return;
    }
    Status status = job_manager_.Add(response->id(), job_desc);
    if (status != proto::kOk) {
        submit_response->mutable_error_code()->set_status(status);
        submit_response->mutable_error_code()->set_reason("appmaster add job error");
        done->Run();
        return;
    }
    submit_response->mutable_error_code()->set_status(status);
    submit_response->mutable_error_code()->set_reason("submit job ok");
    submit_response->set_jobid(response->id());
    done->Run();
    return;
}

void AppMasterImpl::BuildContainerDescription(const ::baidu::galaxy::proto::JobDescription& job_desc,
                                              ::baidu::galaxy::proto::ContainerDescription* container_desc) {
    container_desc->set_priority(job_desc.priority());
    container_desc->set_run_user(job_desc.run_user());
    container_desc->set_version(job_desc.version());
    container_desc->set_max_per_host(job_desc.deploy().max_per_host());
    container_desc->set_tag(job_desc.tag());
    container_desc->set_cmd_line(FLAGS_appworker_cmdline);
    for (int i = 0; i < job_desc.pool_names_size(); i++) {
        container_desc->add_pool_names(job_desc.pool_names(i));
    }
    container_desc->mutable_workspace_volum()->CopyFrom(job_desc.pod().workspace_volum());
    container_desc->mutable_data_volums()->CopyFrom(job_desc.pod().data_volums());
    for (int i = 0; i < job_desc.pod().tasks_size(); i++) {
        Cgroup* cgroup = container_desc->add_cgroups();
        cgroup->mutable_cpu()->CopyFrom(job_desc.pod().tasks(i).cpu());
        cgroup->mutable_memory()->CopyFrom(job_desc.pod().tasks(i).memory());
        cgroup->mutable_tcp_throt()->CopyFrom(job_desc.pod().tasks(i).tcp_throt());
        cgroup->mutable_blkio()->CopyFrom(job_desc.pod().tasks(i).blkio());
    }
    return;
}

void AppMasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::SubmitJobRequest* request,
               ::baidu::galaxy::proto::SubmitJobResponse* response,
               ::google::protobuf::Closure* done) {
    const JobDescription& job_desc = request->job();
    MutexLock lock(&resman_mutex_);
    proto::CreateContainerGroupRequest* container_request = new proto::CreateContainerGroupRequest();
    container_request->mutable_user()->CopyFrom(request->user());
    container_request->set_name(job_desc.name());
    BuildContainerDescription(job_desc, container_request->mutable_desc());
    container_request->set_replica(job_desc.deploy().replica());
    proto::CreateContainerGroupResponse* container_response = new proto::CreateContainerGroupResponse();
    
    boost::function<void (const proto::CreateContainerGroupRequest*,
                          proto::CreateContainerGroupResponse*, 
                          bool, int)> call_back;
    call_back = boost::bind(&AppMasterImpl::CreateContainerGroupCallBack, this,
                            job_desc, response, done,
                            _1, _2, _3, _4);
    ResMan_Stub* resman_;
    rpc_client_.GetStub(resman_endpoint_, &resman_);
    rpc_client_.AsyncRequest(resman_,
                            &ResMan_Stub::CreateContainerGroup,
                            container_request,
                            container_response,
                            call_back,
                            5, 0);
    delete resman_;
    return;
}

void AppMasterImpl::UpdateContainerGroupCallBack(JobDescription job_desc, 
                                         proto::UpdateJobResponse* update_response,
                                         ::google::protobuf::Closure* done,
                                         const proto::UpdateContainerGroupRequest* request,
                                         proto::UpdateContainerGroupResponse* response,
                                         bool failed, int err) {
    boost::scoped_ptr<const proto::UpdateContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<proto::UpdateContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != proto::kOk) {
        //LOG(WARNING, "fail to update container group");
        update_response->mutable_error_code()->CopyFrom(response_ptr->error_code());
        done->Run();
        return;
    }
    Status status = job_manager_.Update(request->id(), job_desc);
    if (status != proto::kOk) {
        update_response->mutable_error_code()->set_status(status);
        update_response->mutable_error_code()->set_reason("appmaster update job error");
        done->Run();
        return;
    }
    update_response->mutable_error_code()->set_status(status);
    update_response->mutable_error_code()->set_reason("updte job ok");
    done->Run();
    return;
}

void AppMasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::UpdateJobRequest* request,
               ::baidu::galaxy::proto::UpdateJobResponse* response,
               ::google::protobuf::Closure* done) {
    const JobDescription& job_desc = request->job();
    MutexLock lock(&resman_mutex_);
    proto::UpdateContainerGroupRequest* container_request = new proto::UpdateContainerGroupRequest();
    container_request->mutable_user()->CopyFrom(request->user());
    container_request->set_id(request->jobid());
    container_request->set_interval(job_desc.deploy().interval());
    BuildContainerDescription(job_desc, container_request->mutable_desc());
    container_request->set_replica(job_desc.deploy().replica());
    proto::UpdateContainerGroupResponse* container_response = new proto::UpdateContainerGroupResponse();
    boost::function<void (const proto::UpdateContainerGroupRequest*,
                          proto::UpdateContainerGroupResponse*, 
                          bool, int)> call_back;
    call_back = boost::bind(&AppMasterImpl::UpdateContainerGroupCallBack, this,
                            job_desc, response, done,
                            _1, _2, _3, _4);
    ResMan_Stub* resman_;
    rpc_client_.GetStub(resman_endpoint_, &resman_);
    rpc_client_.AsyncRequest(resman_,
                            &ResMan_Stub::UpdateContainerGroup,
                            container_request,
                            container_response,
                            call_back,
                            5, 0);
    delete resman_;
    return;
}

void AppMasterImpl::RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::RemoveJobRequest* request,
               ::baidu::galaxy::proto::RemoveJobResponse* response,
               ::google::protobuf::Closure* done) {
    Status status = job_manager_.Terminate(request->jobid(), 
                                            request->user(), request->hostname());
    response->mutable_error_code()->set_status(status);
    response->mutable_error_code()->set_reason("remove job ok");
    done->Run();
    return;
}

void AppMasterImpl::ListJobs(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::ListJobsRequest* request,
                            ::baidu::galaxy::proto::ListJobsResponse* response,
                            ::google::protobuf::Closure* done) {
    job_manager_.GetJobsOverview(response->mutable_jobs());
    response->mutable_error_code()->set_status(kOk);
    done->Run();
}

void AppMasterImpl::ShowJob(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::ShowJobRequest* request,
                            ::baidu::galaxy::proto::ShowJobResponse* response,
                            ::google::protobuf::Closure* done) {

    Status rlt = job_manager_.GetJobInfo(request->jobid(), response->mutable_job());
    response->mutable_error_code()->set_status(rlt);
    done->Run();
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
    Status status = job_manager_.HandleFetch(request, response);
    if (status != kOk) {
        LOG(WARNING) << "FetchTask failed, code:" << status << ", method:" << __FUNCTION__;
    }
    done->Run();
    return;
}

}
}


