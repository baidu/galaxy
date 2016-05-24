// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "job_manager.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <gflags/gflags.h>
#include "timer.h"

DECLARE_string(nexus_root);
DECLARE_string(nexus_addr);
DECLARE_int32(master_job_check_interval);
DECLARE_int32(master_pod_check_interval);
DECLARE_int32(master_pod_dead_time);
DECLARE_string(jobs_store_path);

namespace baidu {
namespace galaxy {
JobManager::JobManager() : nexus_(NULL) {
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr);
}

JobManager::~JobManager() {
    delete nexus_;
}

void JobManager::Start() {
    BuildFsm();
    BuildDispatch();
    BuildAging();
    return;
}

std::string JobManager::BuildFsmKey(const JobStatus& status,
                                        const JobEvent& event) {
    return JobStatus_Name(status) + ":" + JobEvent_Name(event);
}

FsmTrans* JobManager::BuildFsmValue(const JobStatus& status,
                                    const TransFunc& func) {
    FsmTrans* trans = new FsmTrans();
    trans->next_status_ = status;
    trans->trans_func_ = func;
    return trans;
}

void JobManager::BuildFsm() {
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kFetch),
        BuildFsmValue(kJobRunning, boost::bind(&JobManager::StartJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kRemove),
        BuildFsmValue(kJobFinished, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kUpdate),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kUpdate),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kRemove),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kUpdateFinish),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RecoverJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kRemove),
        BuildFsmValue(kJobRunning, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobDestroying, kRemoveFinish),
        BuildFsmValue(kJobFinished, boost::bind(&JobManager::ClearJob, this, _1, _2))));
    return;
}

void JobManager::BuildDispatch() {
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobPending), boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobRunning), boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobUpdating), boost::bind(&JobManager::UpdatePod, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobDestroying), boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobFinished), boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    return;
}

void JobManager::BuildAging() {
    aging_.insert(std::make_pair(JobStatus_Name(kJobPending), boost::bind(&JobManager::CheckPending, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobRunning), boost::bind(&JobManager::CheckRunning, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobUpdating), boost::bind(&JobManager::CheckUpdating, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobDestroying), boost::bind(&JobManager::CheckDestroying, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobFinished), boost::bind(&JobManager::CheckClear, this, _1)));
    return;
}

void JobManager::CheckPending(Job* job) {
    return;
}

void JobManager::CheckRunning(Job* job) {
    return;
}

void JobManager::CheckUpdating(Job* job) {
    mutex_.AssertHeld();
    for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
        it != job->pods_.end(); ++it) {
        PodInfo* pod = it->second;
        if (pod->update_time() != job->update_time_) {
            //log updating
            return;
        }
    }
    std::string fsm_key = BuildFsmKey(job->status_, kUpdateFinish);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            return;
        }
        job->status_ = fsm_it->second->next_status_;
        //SaveToNexus(job);
    } else {
        return;
    }
    return;
}

void JobManager::CheckDestroying(Job* job) {
    mutex_.AssertHeld();
    if (job->pods_.size() != 0) {
        return;
    }
    std::string fsm_key = BuildFsmKey(job->status_, kRemoveFinish);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, (void*)&job->user_);
        if (kOk != rlt) {
            return;
        }
        job->status_ = fsm_it->second->next_status_;
        //SaveToNexus(job);
    } else {
        return;
    }
    return;
}

void JobManager::CheckClear(Job* job) {
    mutex_.AssertHeld();
    ClearJob(job, (void*)&job->user_);
    return;
}

void JobManager::CheckJobStatus(Job* job) {
    MutexLock lock(&mutex_);
    if (job == NULL) {
        return;
    }
    job_checker_.DelayTask(FLAGS_master_job_check_interval * 1000, boost::bind(&JobManager::CheckJobStatus, this, job));
    std::map<std::string, AgingFunc>::iterator it = aging_.find(JobStatus_Name(job->status_));
    if (it != aging_.end()) {
        it->second(job);
    }
    return;
}

void JobManager::CheckPodAlive(PodInfo* pod, Job* job) {
    MutexLock lock(&mutex_);
    if (job == NULL || pod == NULL) {
        return;
    }
    if ((::baidu::common::timer::get_micros() - pod->heartbeat_time())/1000 >
        FLAGS_master_pod_dead_time) {
        std::map<std::string, PodInfo*>::iterator it = job->pods_.find(pod->podid());
        if (it != job->pods_.end()) {
            job->pods_.erase(pod->podid());
            delete pod;
        }
        return;
    }
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval,
        boost::bind(&JobManager::CheckPodAlive, this, pod, job));
    return;
}

Status JobManager::Add(const JobId& job_id, const JobDescription& job_desc) { 
    Job* job = new Job();
    job->status_ = kJobPending;
    job->desc_.CopyFrom(job_desc);
    job->id_ = job_id;
    // add default version
    if (!job->desc_.has_version()) {
        job->desc_.set_version("1.0.0");
    }
    job->pod_desc_[job_desc.version()].CopyFrom(job_desc.pod());
    job->curent_version_ = job_desc.version();
    job->action_type_ = kActionNull;

    job->create_time_ = ::baidu::common::timer::get_micros();
    job->update_time_ = ::baidu::common::timer::get_micros();
    // TODO add nexus lock
    //SaveToNexus(job);
    
    MutexLock lock(&mutex_); 
    jobs_[job_id] = job;
    /*
    LOG(INFO, "job %s[%s] submitted with deploy_step %d, replica %d , pod version %s",
      job_id.c_str(), 
      job_desc.name().c_str(),
      job_desc.deploy().step(),
      job_desc.deploy()replica(),
      job->latest_version.c_str());
      */
    return kOk;
}

Status JobManager::Update(const JobId& job_id, const JobDescription& job_desc) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        //LOG(WARNING, "update job failed, job not found: %s", job_id.c_str());
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kUpdate);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, (void*)&job_desc);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        /*
        bool save_ok = SaveToNexus(job);
        if (!save_ok) {
            return kError;
        }
        */
    } else {
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::Terminte(const JobId& jobid) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kRemove);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
    } else {
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::StartJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    return kOk;
}

Status JobManager::RecoverJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    return kOk;
}

Status JobManager::UpdateJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    JobDescription* desc = (JobDescription*)arg;
    job->update_time_ = ::baidu::common::timer::get_micros();
    job->curent_version_ = desc->version();
    job->pod_desc_[desc->version()] = desc->pod();
    for (int i = 0; i < desc->pod().tasks_size(); i++) {
        for (int j = 0; j < job->desc_.pod().tasks_size(); j++) {
            if (desc->pod().tasks(i).id() !=
                job->desc_.pod().tasks(j).id()) {
                continue;
            }
            if (desc->pod().tasks(i).exe_package().package().version() !=
                job->desc_.pod().tasks(j).exe_package().package().version()) {
                job->action_type_ = kActionRebuild;
            } else if (desc->pod().tasks(i).data_package().packages_size() !=
                job->desc_.pod().tasks(j).data_package().packages_size()) {
                job->action_type_ = kActionRebuild;
            } else {
                for (int k = 0; k < job->desc_.pod().tasks(j).data_package().packages_size();
                    k++) {
                    if (desc->pod().tasks(i).data_package().packages(k).version() !=
                        job->desc_.pod().tasks(j).data_package().packages(k).version()) {
                        job->action_type_ = kActionReload;
                    }
                }

            }
        }
    }
    for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
        it != job->pods_.end(); it++) {
        it->second->set_update_time(job->update_time_);
    }
    //LOG(INFO, "job desc updated succes: %s", job_desc.name().c_str());
    return kOk;
}

Status JobManager::RemoveJob(Job* job, void* arg) {
    return kOk;
}

Status JobManager::ClearJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    proto::RemoveContainerGroupRequest* container_request = new proto::RemoveContainerGroupRequest();
    proto::RemoveContainerGroupResponse* container_response = new proto::RemoveContainerGroupResponse();
    container_request->mutable_user()->CopyFrom(*(User*)arg);
    container_request->set_id(job->id_);
    boost::function<void (const proto::RemoveContainerGroupRequest*, proto::RemoveContainerGroupResponse*, 
            bool, int)> call_back;
    call_back = boost::bind(&JobManager::RemoveContainerGroupCallBack, this, _1, _2, _3, _4);
    ResMan_Stub* resman_;
    rpc_client_.GetStub(resman_endpoint_, &resman_);
    rpc_client_.AsyncRequest(resman_,
                            &ResMan_Stub::RemoveContainerGroup,
                            container_request,
                            container_response,
                            call_back,
                            5, 0);
    delete resman_;
    return kOk;
}

void JobManager::RemoveContainerGroupCallBack(const proto::RemoveContainerGroupRequest* request,
                                          proto::RemoveContainerGroupResponse* response,
                                          bool failed, int) {
    MutexLock lock(&resman_mutex_);
    boost::scoped_ptr<const proto::RemoveContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<proto::RemoveContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != kOk) {
        //LOG(WARNING, "fail to remove container group");
        return;
    }
    JobId id = request->id();
    std::map<JobId, Job*>::iterator job_it = jobs_.find(id);
    if (job_it != jobs_.end()) {
        Job* job = job_it->second;
        jobs_.erase(id);
        delete job;
    }
    if (DeleteFromNexus(id)) {
        //LOG
    }
    return;
}

void JobManager::CreatePod(Job* job,
                            std::string podid,
                            std::string endpoint) {
    PodInfo* podinfo = new PodInfo();
    podinfo->set_podid(podid);
    podinfo->set_jobid(job->id_);
    podinfo->set_endpoint(endpoint);
    podinfo->set_status(kPodDeploying);
    podinfo->set_version(job->curent_version_);
    podinfo->set_start_time(::baidu::common::timer::get_micros());
    podinfo->set_update_time(::baidu::common::timer::get_micros());
    job->pods_[podid] = podinfo;
    job->deploying_pods_.insert(podid);
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval,
        boost::bind(&JobManager::CheckPodAlive, this, podinfo, job));
    return;
}

Status JobManager::PodHeartBeat(Job* job, void* arg) {
    mutex_.AssertHeld();
    proto::FetchTaskRequest* request = (proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    //heartbeat
    if (pod_it != job->pods_.end()) {
        PodInfo* podinfo = pod_it->second;
        //repeated fetch from different worker
        if (podinfo->endpoint() != request->endpoint()) {  
            //abandon worker request
            if ((request->update_time()) < podinfo->update_time()) {
                return kTerminate;
            }
            //new worker request
            podinfo->set_endpoint(request->endpoint());
            podinfo->set_status(kPodDeploying);
            podinfo->set_version(job->curent_version_);
            podinfo->set_start_time(job->create_time_);
            podinfo->set_update_time(::baidu::common::timer::get_micros());
            return kOk;
        }
        //refresh
        podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
        if (request->status() != kPodDeploying &&
            job->deploying_pods_.find(request->podid()) != 
            job->deploying_pods_.end()) {
            job->deploying_pods_.erase(request->podid());
        }
        return kOk;
    }
    //new pod
    if (job->deploying_pods_.size() >= job->desc_.deploy().step()) {
        return kDeny;
    }
    if (job->pods_.size() >= job->desc_.deploy().replica()) {
        return kDeny;
    }
    CreatePod(job, request->podid(), request->endpoint());
    return kOk;
}

Status JobManager::UpdatePod(Job* job, void* arg) {
    mutex_.AssertHeld();
    ::baidu::galaxy::proto::FetchTaskRequest* request =
    (::baidu::galaxy::proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    if (pod_it == job->pods_.end()) {
        //new pod
        if (job->deploying_pods_.size() >= job->desc_.deploy().step()) {
            return kDeny;
        }
        if (job->pods_.size() >= job->desc_.deploy().replica()) {
            return kDeny;
        }
        CreatePod(job, request->podid(), request->endpoint());
        return kOk;
    }
    PodInfo* podinfo = pod_it->second;
    //fresh
    podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
    if (request->status() != kPodDeploying &&
        job->deploying_pods_.find(request->podid()) != 
        job->deploying_pods_.end()) {
        job->deploying_pods_.erase(request->podid());
    }
    //
    Status rlt_code;
    if (podinfo->update_time() <= request->update_time()) {
        if (job->action_type_ == kActionNull) {
            rlt_code = kOk;
        } else if (job->action_type_ == kActionRebuild) {
            rlt_code = kRebuild;
        } else if (job->action_type_ == kActionReload) {
            rlt_code = kReload;
        } else {
            rlt_code = kError;
        }
    }
    return rlt_code;
}

Status JobManager::DistroyPod(Job* job, void* arg) {
    mutex_.AssertHeld();
    ::baidu::galaxy::proto::FetchTaskRequest* request =
    (::baidu::galaxy::proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    if (pod_it == job->pods_.end()) {
        return kTerminate;
    }
    PodInfo* podinfo = pod_it->second;
    job->pods_.erase(podinfo->podid());
    delete podinfo;
    return kTerminate;
}

Status JobManager::HandleFetch(const ::baidu::galaxy::proto::FetchTaskRequest* request,
                             ::baidu::galaxy::proto::FetchTaskResponse* response) {
    MutexLock lock(&mutex_);
    std::map<std::string, Job*>::iterator job_it = jobs_.find(request->jobid());
    if (job_it == jobs_.end()) {
        response->mutable_error_code()->set_status(kJobNotFound);
        response->mutable_error_code()->set_reason("Jobid not found");
        return kJobNotFound;
    }
    Job* job = job_it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kFetch);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
    } else {
        return kStatusConflict;
    }
    std::map<std::string, DispatchFunc>::iterator dispatch_it = dispatch_.find(JobStatus_Name(job->status_));
    if (dispatch_it == dispatch_.end()) {
        return kError;
    }
    Status rlt = dispatch_it->second(job, (void*)request);
    response->mutable_error_code()->set_status(rlt);
    if (kError == rlt) {
        return rlt;
    }
    response->set_update_time(job->update_time_);
    response->mutable_pod()->CopyFrom(job->desc_.pod());
    return kOk;
}
/*
void JobManager::ReloadJobInfo(const JobInfo& job_info) {
    Job* job = new Job();
    job->status_ = job_info.status();
    job->desc_.CopyFrom(job_info.desc());
    job->id_ = job_info.jobid();
    job->curent_version_ = job_info.version();
    for (int32_t i = 0; i < job_info.pods_size(); i++) {
        const PodDescription& desc = job_info.pods(i);
        job->pod_desc_[desc.version()] = desc;
    }
    if (job_info.has_start_time()) {
        job->create_time_ = job_info.start_time();
    } 

    if (job_info.has_update_time()) {
        job->update_time_ = job_info.update_time();
    }
    MutexLock lock(&mutex_);
    jobs_[job_id] = job;
}
*/
void JobManager::GetJobsOverview(JobOverviewList* jobs_overview) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.begin();
    for (; job_it != jobs_.end(); ++job_it) {
        JobId jobid = job_it->first;
        Job* job = job_it->second;
        JobOverview* overview = jobs_overview->Add();
        overview->mutable_desc()->CopyFrom(job->desc_);
        overview->set_jobid(jobid);
        overview->set_status(job->status_);
        uint32_t state_stat[kPodFinished + 1] = {0};
        for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
            it != job->pods_.end(); it++) {
            const PodInfo* pod = it->second;
            state_stat[pod->status()]++;
        }
        overview->set_running_num(state_stat[kPodServing]);
        overview->set_pending_num(state_stat[kPodPending]);
        overview->set_deploying_num(state_stat[kPodPending] + state_stat[kPodStarting]);
        overview->set_death_num(state_stat[kPodFinished] + state_stat[kPodFailed]);
        overview->set_create_time(job->create_time_);
        overview->set_update_time(job->update_time_);
    }
    return;
}

Status JobManager::GetJobInfo(const JobId& jobid, JobInfo* job_info) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        //LOG(WARNING, "get job info failed, no such job: %s", jobid.c_str());
        return kJobNotFound;
    }
    Job* job = job_it->second;
    job_info->set_jobid(jobid);
    job_info->set_status(job->status_);
    job_info->set_create_time(job->create_time_);
    job_info->set_update_time(job->update_time_);
    job_info->set_version(job->curent_version_);
    job_info->mutable_desc()->CopyFrom(job->desc_);
    std::map<PodId, PodInfo*>::iterator pod_it = job->pods_.begin();
    for (; pod_it != job->pods_.end(); ++pod_it) {
        PodInfo* pod = pod_it->second;
        job_info->add_pods()->CopyFrom(*pod);
    }
    return kOk;
}
/*
bool JobManager::SaveToNexus(const Job* job) {
    if (job == NULL) {
        return false;
    }
    JobInfo job_info;
    job_info.set_jobid(job->id_);
    job_info.set_state(job->state_);
    job_info.set_update_state(job->update_state_);
    job_info.mutable_desc()->CopyFrom(job->desc_);
    // delete pod 
    if (job_info.desc().has_pod()) { 
        job_info.mutable_desc()->release_pod();
    }
    job_info.set_latest_version(job->latest_version);
    std::map<Version, PodDescription>::const_iterator it = job->pod_desc_.begin();
    for(; it != job->pod_desc_.end(); ++it) {
        PodDescription* pod_desc = job_info.add_pod_descs();
        pod_desc->CopyFrom(it->second);
    }
    job_info.set_create_time(job->create_time);
    job_info.set_update_time(job->update_time);
    std::string job_raw_data;
    std::string job_key = FLAGS_nexus_root_path + FLAGS_jobs_store_path 
                          + "/" + job->id_;
    job_info.SerializeToString(&job_raw_data);
    ::galaxy::ins::sdk::SDKError err;
    bool put_ok = nexus_->Put(job_key, job_raw_data, &err);
    if (!put_ok) {
        LOG(WARNING, "fail to put job[%s] %s to nexus err msg %s", 
          job_info.desc().name().c_str(),
          job_info.jobid().c_str(),
          ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
    }
    return put_ok; 
}
*/
bool JobManager::DeleteFromNexus(const JobId& job_id) {
    std::string job_key = FLAGS_nexus_addr + FLAGS_jobs_store_path 
                          + "/" + job_id;
    ::galaxy::ins::sdk::SDKError err;
    bool delete_ok = nexus_->Delete(job_key, &err);
    if (!delete_ok) {
        /*
        LOG(WARNING, "fail to delete job %s fxwrom nexus err msg %s", 
          job_id.c_str(),
          ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
          */
    }
    return delete_ok;
}

}
}
