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

DECLARE_string(nexus_servers);

namespace baidu {
namespace galaxy {

AppMasterImpl::AppMasterImpl() : nexus_(NULL) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_servers);
    resman_(NULL);
    resman_watcher_ = new ResourceManagerWatcher();
    mutex_resman_endpoint_();
}

AppMasterImpl::~AppMasterImpl() {
    if (rpc_client != NULL) {
        delete rpc_client;
        rpc_client = NULL;
    }
    delete resman_watcher_;
    delete nexus_;
}

void AppMasterImpl::CreateContainerGroupCallBack(const CreateContainerGroupRequest* request,
                                         CreateContainerGroupResponse* response,
                                         bool failed, int,
                                         JobDescription job_desc,
                                         SubmitJobResponse* submit_response,
                                         ::google::protobuf::Closure* done) {
    MutexLock lock(&resman_mutex_);
    boost::scoped_ptr<const CreateContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<CreateContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != kOk) {
        LOG(WARNING, "fail to create container group");
        submitresponse->set_error_code(response_ptr->error_code());
        done->Run();
        return;
    }
    Status status = job_manager_.Add(request->id(), job_desc);
    ErrorCode error_code;
    if (status != kOk) {
        error_code.set_status(status);
        error_code.set_reason("appmaster add job error");
        submit_response->set_error_code(error_code);
        done->Run();
        return;
    }
    error_code.set_status(status);
    error_code.set_reason("submit job ok");
    submit_response->set_jobid(response->id());
    done->Run();
    return;
}

void AppMasterImpl::BuildContainerDescription(const ::baidu::galaxy::proto::JobDescription& job_desc,
                                              ::baidu::galaxy::proto::ContainerDescription* container_desc) {

    optional VolumRequired workspace_volum = 4;
    repeated VolumRequired data_volums = 5;   
    repeated Cgroup cgroups = 7;

    container_desc->set_priority(job_desc.priority());
    container_desc->set_run_user(job_desc.run_user());
    container_desc->set_version(job_desc.version());
    container_desc->set_max_per_host(job_desc.deploy().max_per_host());
    container_desc->set_tag(job_desc.tag());
    container_desc->set_cmd_line(FLAGS_appworker_cmdline);
    for (int i = 0; i < request->pool_names_size(); i++) {
        container_desc->add_pool_names(request->pool_names(i));
    }
    container_request->mutable_desc()->mutable_workspace_volum()->CopyFrom(submit_request->job().pod_desc().workspace_volum());
    container_request->mutable_desc()->mutable_data_volums()->CopyFrom(submit_request->job().pod_desc().data_volums());
    for (int i = 0; i < submit_request->job().pod_desc().tasks_size(); i++) {
        Cgroup* cgroup = container_request->add_cgroups();
        cgroup->mutable_cpu()->CopyFrom(submit_request->job().pod_desc().tasks(i).cpu());
        cgroup->mutable_memory()->CopyFrom(submit_request->job().pod_desc().tasks(i).memory());
        cgroup->mutable_tcp_throt()->CopyFrom(submit_request->job().pod_desc().tasks(i).tcp_throt();
        cgroup->mutable_blkio()->CopyFrom(submit_request->job().pod_desc().tasks(i).blkio());
    }
    return;
}

void AppMasterImpl::SubmitJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::SubmitJobRequest* request,
               ::baidu::galaxy::proto::SubmitJobResponse* response,
               ::google::protobuf::Closure* done) {
    const JobDescription& job_desc = request->job();
    MutexLock lock(&resman_mutex_);
    CreateContainerGroupRequest* container_request = new CreateContainerGroupRequest();
    container_request->set_user(request->user());
    container_request->set_name(request->name());
    container_request->set_desc(request->desc());
    BuildContainerDescription(request->desc, container_request->mutable_desc());
    container_request->set_replica(request->desc().deploy().replica());
    CreateContainerGroupResponse* container_response = new CreateContainerGroupResponse();
    boost::function<void (const CreateContainerGroupRequest*, CreateContainerGroupResponse*, 
            bool, int, SubmitJobResponse*, ::google::protobuf::Closure*)> call_back;
    callback = boost::bind(&AppMasterImpl::CreateContainerGroupCallBack, this, _1, _2, _3, _4, _5, _6, _7);
    rpc_client.AsyncRequest(resman_,
                            &ResMan_Stub::CreateContainerGroup,
                            container_request,
                            container_response,
                            callback,
                            5, 0, request->desc(), response, done);
    return;
}

void AppMasterImpl::UpdateContainerGroupCallBack(const UpdateContainerGroupRequest* request,
                                         UpdateContainerGroupResponse* response,
                                         bool failed, int,
                                         JobDescription job_desc, 
                                         UpdateJobResponse* update_response,
                                         ::google::protobuf::Closure* done) {
    MutexLock lock(&resman_mutex_);
    boost::scoped_ptr<const UpdateContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<UpdateContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != kOk) {
        LOG(WARNING, "fail to update container group");
        update_response->set_error_code(response_ptr->error_code());
        done->Run();
        return;
    }
    Status status = job_manager_.Update(response->id(), job_desc);
    ErrorCode error_code;
    if (status != kOk) {
        error_code.set_status(status);
        error_code.set_reason("appmaster update job error");
        update_response->set_error_code(error_code);
        done->Run();
        return;
    }
    error_code.set_status(status);
    error_code.set_reason("updte job ok");
    done->Run();
    return;
}

void AppMasterImpl::UpdateJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::UpdateJobRequest* request,
               ::baidu::galaxy::proto::UpdateJobResponse* response,
               ::google::protobuf::Closure* done) {
    const JobDescription& job_desc = request->job();
    MutexLock lock(&resman_mutex_);
    UpdateContainerGroupRequest* container_request = new UpdateContainerGroupRequest();
    container_request->set_user(request->user());
    container_request->set_name(request->name());
    container_request->set_desc(request->desc());
    container_request->set_interal(request->desc().deploy().interval());
    BuildContainerDescription(request->desc, container_request->mutable_desc());
    container_request->set_replica(request->desc().deploy().replica());
    UpdateContainerGroupResponse* container_response = new UpdateContainerGroupResponse();
    boost::function<void (const UpdateContainerGroupRequest*, UpdateContainerGroupResponse*, 
            bool, int, JobDescription， UpdateJobResponse*, ::google::protobuf::Closure*)> call_back;
    callback = boost::bind(&AppMasterImpl::UpdateContainerGroupCallBack, this, _1, _2, _3, _4, _5 _6);
    rpc_client.AsyncRequest(resman_,
                            &ResMan_Stub::UpdateContainerGroup,
                            container_request,
                            container_response,
                            callback,
                            5, 0, request->desc(), response, done);
    return;
}

void AppMasterImpl::RemoveJob(::google::protobuf::RpcController* controller,
               const ::baidu::galaxy::proto::RemoveJobRequest* request,
               ::baidu::galaxy::proto::RemoveJobResponse* response,
               ::google::protobuf::Closure* done) {
    
    return;
}

void AppMasterImpl::ListJobs(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::ListJobsRequest* request,
                            ::baidu::galaxy::proto::ListJobsResponse* response,
                            ::google::protobuf::Closure* done) {
    Status rlt = job_manager_.GetJobsOverView(response->montable_jobs());
    ErrorCode error_code;
    error_code.set_status(rlt)；
    response->set_error_code(error_code);
    done->Run();
}

void AppMasterImpl::ShowJob(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::ShowJobRequest* request,
                            ::baidu::galaxy::proto::ShowJobResponse* response,
                            ::google::protobuf::Closure* done) {

    Status rlt = job_manager_.GetJobInfo(request->jobid(), response->JobInfo()));
    ErrorCode error_code;
    error_code.set_status(rlt)；
    response->set_error_code(error_code);
    done->Run();
}

void AppMasterImpl::ExecuteCmd(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::ExecuteCmdRequest* request,
                               ::baidu::galaxy::proto::ExecuteCmdResponse* response,
                               ::google::protobuf::Closure* done) {
}

void AppMasterImpl::FetchTask(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::proto::FecthTaskRequest* request,
                              ::baidu::galaxy::proto::FetchTaskResponse* response,
                              ::google::protobuf::Closure* done) {
    Status status = job_manager_.AssignTask(request, response);
    done->Run();
    return;
}

}
}


