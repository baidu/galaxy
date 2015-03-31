// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "galaxy.h"

#include "proto/master.pb.h"
#include "rpc/rpc_client.h"

namespace galaxy {

class GalaxyImpl : public Galaxy {
public:
    GalaxyImpl(const std::string& master_addr)
      : master_(NULL) {
        rpc_client_ = new RpcClient();
        rpc_client_->GetStub(master_addr, &master_);
    }
    virtual ~GalaxyImpl() {}
    bool NewTask(const std::string& task_name,
                 const std::string& task_raw,
                 const std::string& cmd_line,
                 int32_t count);
    bool ListTask(int64_t taks_id = -1);
    bool ListJob();

    bool TerminateTask(int64_t task_id);
private:
    RpcClient* rpc_client_;
    Master_Stub* master_;
};

bool GalaxyImpl::TerminateTask(int64_t task_id) {
    TerminateTaskRequest request;        
    request.set_task_id(task_id);
    TerminateTaskResponse response;
    rpc_client_->SendRequest(master_, 
            &Master_Stub::TerminateTask, 
            &request, &response, 5, 1);
    if (response.has_status() 
            && response.status() == 0) {
        fprintf(stdout, "SUCCESS\n"); 
    }  
    else {
        if (response.has_status()) {
            fprintf(stdout, "FAIL %d\n", response.status()); 
        }
        else {
            fprintf(stdout, "FAIL unkown\n");
        }
    }
    return true;
}

bool GalaxyImpl::NewTask(const std::string& task_name,
                         const std::string& task_raw,
                         const std::string& cmd_line,
                         int32_t count) {
    NewTaskRequest request;
    request.set_task_name(task_name);
    request.set_task_raw(task_raw);
    request.set_cmd_line(cmd_line);
    request.set_replica_num(count);
    NewTaskResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::NewTask,
                             &request, & response, 5, 1);
    return true;
}

bool GalaxyImpl::ListJob() {
    ListJobRequest request;
    ListJobResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::ListJob,
                             &request, &response, 5, 1);
    fprintf(stdout, "================================\n");
    int job_num = response.jobs_size();
    for (int i = 0; i < job_num; i++) {
        const JobInstance& job = response.jobs(i);
        fprintf(stdout, "%ld\t%s\t%d\t%d\n",
                job.job_id(), job.job_name().c_str(),
                job.running_task_num(), job.replica_num());
    }
    fprintf(stdout, "================================\n");
    return true;
}

bool GalaxyImpl::ListTask(int64_t task_id) {
    ListTaskRequest request;
    if (task_id != -1) {
        request.set_task_id(task_id); 
    }
    ListTaskResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::ListTask,
                             &request, &response, 5, 1);
    fprintf(stdout, "================================\n");
    int task_size = response.tasks_size();
    for (int i = 0; i < task_size; i++) {
        if (!response.tasks(i).has_info() ||
                !response.tasks(i).info().has_task_id()) {
            continue; 
        }
        int64_t task_id = response.tasks(i).info().task_id();
        std::string task_name;
        std::string agent_addr;
        std::string state;
        if (response.tasks(i).has_info()){
            if (response.tasks(i).info().has_task_name()) {
                task_name = response.tasks(i).info().task_name(); 
            }
        }
        if (response.tasks(i).has_status()) {
            int task_state = response.tasks(i).status();
            if (TaskState_IsValid(task_state)) {
                state = TaskState_Name((TaskState)task_state); 
            } 
        }
        if (response.tasks(i).has_agent_addr()) {
            agent_addr = response.tasks(i).agent_addr(); 
        }
        fprintf(stdout, "%ld\t%s\t%s\t%s\n", task_id, task_name.c_str(), state.c_str(), agent_addr.c_str());
    }
    fprintf(stdout, "================================\n");
    return true;
}


Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
