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
    bool NewJob(const JobDescription& job);
    bool UpdateJob(const JobDescription& job);
    bool ListJob(std::vector<JobInstanceDescription>* jobs);
    bool TerminateJob(int64_t job_id);
    bool ListTask(int64_t job_id,std::vector<TaskDescription>* tasks);
private:
    RpcClient* rpc_client_;
    Master_Stub* master_;
};

bool GalaxyImpl::TerminateJob(int64_t /*job_id*/) {

    return true;
}

bool GalaxyImpl::NewJob(const JobDescription& job) {
    NewJobRequest request;
    NewJobResponse response;
    request.set_job_name(job.job_name);
    request.set_job_raw(job.pkg.source);
    request.set_cmd_line(job.cmd_line);
    request.set_replica_num(job.replicate_count);
    rpc_client_->SendRequest(master_, &Master_Stub::NewJob,
                             &request,&response,5,1);
    return true;
}

bool GalaxyImpl::UpdateJob(const JobDescription& /*job*/) {
   return true;
}

bool GalaxyImpl::ListJob(std::vector<JobInstanceDescription>* jobs) {
    ListJobRequest request;
    ListJobResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::ListJob,
                             &request,&response,5,1);
    int job_num = response.jobs_size();
    for(int i = 0; i< job_num;i++){
        const JobInstance& job = response.jobs(i);
        JobInstanceDescription job_instance ;
        job_instance.job_id = job.job_id();
        job_instance.job_name = job.job_name();
        job_instance.running_task_num = job.running_task_num();
        job_instance.replicate_count = job.replica_num();
        jobs->push_back(job_instance);
    }
    return true;
}

bool GalaxyImpl::ListTask(int64_t /*job_id*/, std::vector<TaskDescription>* /*job*/) {
      return true;
}



Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
