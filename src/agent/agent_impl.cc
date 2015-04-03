// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "agent_impl.h"

#include <boost/bind.hpp>
#include <errno.h>
#include <string.h>
#include "common/util.h"
#include "rpc/rpc_client.h"

extern std::string FLAGS_master_addr;
extern std::string FLAGS_agent_port;
extern std::string FLAGS_agent_work_dir;
extern double FLAGS_cpu_num;
extern int FLAGS_mem_gbytes;

namespace galaxy {

AgentImpl::AgentImpl() {
    rpc_client_ = new RpcClient();
    ws_mgr_ = new WorkspaceManager(FLAGS_agent_work_dir);
    task_mgr_ = new TaskManager();
    ws_mgr_->Init();
    if (!rpc_client_->GetStub(FLAGS_master_addr, &master_)) {
        assert(0);
    }
    thread_pool_.Start();
    thread_pool_.AddTask(boost::bind(&AgentImpl::Report, this));
}

AgentImpl::~AgentImpl() {
    delete ws_mgr_;
    delete task_mgr_;

}

void AgentImpl::Report() {
    HeartBeatRequest request;
    HeartBeatResponse response;
    std::string addr = common::util::GetLocalHostName() + ":" + FLAGS_agent_port;

    std::vector<TaskStatus > status_vector;
    task_mgr_->Status(status_vector);
    std::vector<TaskStatus>::iterator it = status_vector.begin();
    for(; it != status_vector.end(); ++it){
        TaskStatus* req_status = request.add_task_status();
        req_status->set_task_id(it->task_id());
        req_status->set_status(it->status());
    }
    request.set_agent_addr(addr);
    request.set_cpu_share(FLAGS_cpu_num);
    request.set_mem_share(FLAGS_mem_gbytes);
    request.set_used_cpu_share(ws_mgr_->GetUsedCpuShare());
    request.set_used_mem_share(ws_mgr_->GetUsedMemShare());

    LOG(INFO, "Reprot to master %s,task count %d,"
        "cpu_share %f, cpu_used %f, mem_share %d, mem_used %d",
        addr.c_str(),request.task_status_size(), FLAGS_cpu_num,
        ws_mgr_->GetUsedCpuShare(), FLAGS_mem_gbytes, ws_mgr_->GetUsedMemShare());
    rpc_client_->SendRequest(master_, &Master_Stub::HeartBeat,
                                &request, &response, 5, 1);
    thread_pool_.DelayTask(5000, boost::bind(&AgentImpl::Report, this));
}

void AgentImpl::RunTask(::google::protobuf::RpcController* /*controller*/,
                        const ::galaxy::RunTaskRequest* request,
                        ::galaxy::RunTaskResponse* response,
                        ::google::protobuf::Closure* done) {
    LOG(INFO, "Run Task %s %s", request->task_name().c_str(), request->cmd_line().c_str());
    TaskInfo task_info;
    task_info.set_task_id(request->task_id());
    task_info.set_task_name(request->task_name());
    task_info.set_cmd_line(request->cmd_line());
    task_info.set_task_raw(request->task_raw());
    task_info.set_required_cpu(request->cpu_share());
    task_info.set_required_mem(request->mem_share());
    task_info.set_task_offset(request->task_offset());
    task_info.set_job_replicate_num(request->job_replicate_num());
    LOG(INFO,"start to prepare workspace for %s",request->task_name().c_str());
    LOG(INFO,"cpu_share:%lf\tmem_share:%d",
        task_info.required_cpu(),
        task_info.required_mem());
    int ret = ws_mgr_->Add(task_info);
    if (ret != 0 ){
        LOG(FATAL,"fail to prepare workspace ");
        response->set_status(-2);
        done->Run();
    } else {
        LOG(INFO,"start  task for %s",request->task_name().c_str());
        DefaultWorkspace * workspace ;
        workspace = ws_mgr_->GetWorkspace(task_info);
        ret = task_mgr_->Add(task_info,workspace);
        if (ret != 0){
           LOG(FATAL,"fail to start task");
           response->set_status(-1);
        }
        response->set_status(0);
        done->Run();
    }
    //OpenProcess(request->task_name(), request->task_raw(), request->cmd_line(),"/tmp");
    //done->Run();
}
void AgentImpl::KillTask(::google::protobuf::RpcController* /*controller*/,
                         const ::galaxy::KillTaskRequest* request,
                         ::galaxy::KillTaskResponse* response,
                         ::google::protobuf::Closure* done){
    LOG(INFO,"kill task %d",request->task_id());
    int status = task_mgr_->Remove(request->task_id());
    LOG(INFO,"kill task %d status %d",request->task_id(),status);
    status = ws_mgr_->Remove(request->task_id());
    LOG(INFO,"clean workspace task  %d status %d",request->task_id(),status);
    response->set_status(status);
    done->Run();
}
} // namespace galxay

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
