// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scheduler/scheduler_io.h"

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <gflags/gflags.h>
#include "logging.h"

DECLARE_int32(max_scale_down_size);
DECLARE_int32(max_scale_up_size);
DECLARE_int32(scheduler_get_pending_job_timeout);
DECLARE_int32(scheduler_get_pending_job_period);
DECLARE_int32(scheduler_sync_resource_timeout);
DECLARE_int32(scheduler_sync_resource_period);

namespace baidu {
namespace galaxy {

void SchedulerIO::HandleMasterChange(const std::string& new_master_endpoint) {
    if (new_master_endpoint.empty()) {
        LOG(WARNING, "the master endpoint is deleted from nexus");
    }
    if (new_master_endpoint != master_addr_) {
        MutexLock lock(&master_mutex_);
        LOG(INFO, "master change to %s", new_master_endpoint.c_str());
        master_addr_ = new_master_endpoint; 
        if (master_stub_) {
            delete master_stub_;
        }
        if (!rpc_client_.GetStub(master_addr_, &master_stub_)) {
            LOG(WARNING, "connect master %s failed", master_addr_.c_str()); 
            return;
        }  
    }
}

bool SchedulerIO::Init() {
    bool ok = master_watcher_.Init(
                    boost::bind(&SchedulerIO::HandleMasterChange, this, _1) );
    assert(ok);
    master_addr_ = master_watcher_.GetMasterEndpoint();
    LOG(INFO, "create scheduler io from master %s", master_addr_.c_str());
    bool ret = rpc_client_.GetStub(master_addr_, &master_stub_);
    assert(ret == true);
    return ok;
}

void SchedulerIO::Sync() {
    SyncResources();
    SyncPendingJob();
}

void SchedulerIO::SyncPendingJob() {
    MutexLock lock(&master_mutex_);
    GetPendingJobsRequest* req = new GetPendingJobsRequest();
    req->set_max_scale_up_size(FLAGS_max_scale_up_size);
    req->set_max_scale_down_size(FLAGS_max_scale_down_size);
    GetPendingJobsResponse* response = new GetPendingJobsResponse();
    boost::function<void (const GetPendingJobsRequest*, GetPendingJobsResponse*, bool, int)> call_back;
    call_back = boost::bind(&SchedulerIO::SyncPendingJobCallBack, this, _1, _2, _3, _4);
    rpc_client_.AsyncRequest(master_stub_,
                             &Master_Stub::GetPendingJobs,
                             req, response,
                             call_back,
                             FLAGS_scheduler_get_pending_job_timeout, 0);
}

void SchedulerIO::SyncPendingJobCallBack(const GetPendingJobsRequest* request,
                                   GetPendingJobsResponse* response,
                                   bool failed, int) {
    MutexLock lock(&master_mutex_);
    boost::scoped_ptr<const GetPendingJobsRequest> request_ptr(request);
    boost::scoped_ptr<GetPendingJobsResponse> response_ptr(response);
    if (failed) {
        LOG(WARNING, "query master failed ");
        thread_pool_.DelayTask(FLAGS_scheduler_get_pending_job_period, boost::bind(&SchedulerIO::SyncPendingJob, this));
        return;
    }
    LOG(INFO, "get pending jobs from master , pending_size %d, scale_down_size %d", response_ptr->scale_up_jobs_size(),
         response_ptr->scale_down_jobs_size());
    std::vector<JobInfo*> scale_up_jobs;
    for (int i = 0; i < response_ptr->scale_up_jobs_size(); i++) {
        scale_up_jobs.push_back(response_ptr->mutable_scale_up_jobs(i));
    }
    std::vector<JobInfo*> scale_down_jobs;
    for (int i = 0; i < response_ptr->scale_down_jobs_size(); i++) {
        scale_down_jobs.push_back(response_ptr->mutable_scale_down_jobs(i));
    }
    std::vector<ScheduleInfo*> propose;
    do{
        int32_t status = scheduler_.ScheduleScaleUp(scale_up_jobs, &propose);
        if (status < 0) {
            LOG(WARNING, "fail to schedule scale up ");
            break;
        } 
        status = scheduler_.ScheduleScaleDown(scale_down_jobs, &propose);
        if (status < 0) {
            LOG(WARNING, "fail to schedule scale down ");
            break;
        } 
        if (propose.size() > 0) {
            ProposeRequest pro_request;
            ProposeResponse pro_response;
            for (std::vector<ScheduleInfo*>::iterator it = propose.begin();
                  it != propose.end(); ++it) {
                ScheduleInfo* sched = pro_request.add_schedule();
                sched->CopyFrom(*(*it));
            }
            bool ret = rpc_client_.SendRequest(master_stub_,
                                  &Master_Stub::Propose,
                                  &pro_request,
                                  &pro_response,
                                  5, 1);
            if (!ret) {
                LOG(WARNING,"fail to propse");
            }
        } 
    }while(false);
    CleanPropse(propose);
    thread_pool_.DelayTask(FLAGS_scheduler_get_pending_job_period, boost::bind(&SchedulerIO::SyncPendingJob, this));
}

void SchedulerIO::SyncResources() {
    MutexLock lock(&master_mutex_);
    LOG(INFO, "sync resource from master %s", master_addr_.c_str());
    GetResourceSnapshotRequest* request = new GetResourceSnapshotRequest();
    scheduler_.BuildSyncRequest(request);
    GetResourceSnapshotResponse* response = new GetResourceSnapshotResponse();
    boost::function<void (const GetResourceSnapshotRequest*, GetResourceSnapshotResponse*, bool, int)> call_back;
    call_back = boost::bind(&SchedulerIO::SyncResourcesCallBack, this, _1, _2, _3, _4);
    rpc_client_.AsyncRequest(master_stub_,
                             &Master_Stub::GetResourceSnapshot,
                             request, response,
                             call_back,
                             FLAGS_scheduler_sync_resource_timeout, 0);

}

void SchedulerIO::SyncResourcesCallBack(const GetResourceSnapshotRequest* request,
                                        GetResourceSnapshotResponse* response,
                                        bool failed, int) {
    MutexLock lock(&master_mutex_);
    if (!failed) {
        int32_t agent_count = scheduler_.SyncResources(response);
        LOG(INFO, "sync resource from master successfully, agent count %d",
             agent_count);

    }
    delete request;
    delete response;
    thread_pool_.DelayTask(FLAGS_scheduler_sync_resource_period, boost::bind(&SchedulerIO::SyncResources, this));
}

void SchedulerIO::CleanPropse(std::vector<ScheduleInfo*>& propose) {
    for (std::vector<ScheduleInfo*>::iterator it = propose.begin();
         it != propose.end(); ++it) {
        delete *it;
    }
    propose.clear();
}

} // galaxy
} // baidu
