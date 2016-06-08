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
DECLARE_int32(master_fail_last_threshold);
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
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kUpdate),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kUpdate),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kRemove),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kUpdateFinish),
        BuildFsmValue(kJobRunning, boost::bind(&JobManager::RecoverJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kRemove),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobDestroying, kRemoveFinish),
        BuildFsmValue(kJobFinished, boost::bind(&JobManager::ClearJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kPauseUpdate),
        BuildFsmValue(kJobUpdatePause, boost::bind(&JobManager::PauseUpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdatePause, kUpdateContinue),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::ContinueUpdateJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdatePause, kUpdateRollback),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::RollbackJob, this, _1, _2))));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdatePause, kRemove),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2))));
    
    for (FSM::iterator it = fsm_.begin(); it != fsm_.end(); it++) {
        LOG(INFO) << "key:" << it->first << " value: " << JobStatus_Name(it->second->next_status_);
    }
    return;
}

void JobManager::BuildDispatch() {
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobPending), boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobRunning), boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobUpdating), boost::bind(&JobManager::UpdatePod, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobDestroying), boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobFinished), boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    dispatch_.insert(std::make_pair(JobStatus_Name(kJobUpdatePause), boost::bind(&JobManager::PauseUpdatePod, this, _1, _2)));
    return;
}

void JobManager::BuildAging() {
    aging_.insert(std::make_pair(JobStatus_Name(kJobPending), boost::bind(&JobManager::CheckPending, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobRunning), boost::bind(&JobManager::CheckRunning, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobUpdating), boost::bind(&JobManager::CheckUpdating, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobDestroying), boost::bind(&JobManager::CheckDestroying, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobFinished), boost::bind(&JobManager::CheckClear, this, _1)));
    aging_.insert(std::make_pair(JobStatus_Name(kJobUpdatePause), boost::bind(&JobManager::CheckPauseUpdate, this, _1)));
    return;
}

void JobManager::CheckPending(Job* job) {
    mutex_.AssertHeld();
    return;
}

void JobManager::CheckRunning(Job* job) {
    mutex_.AssertHeld();
    return;
}

void JobManager::CheckPauseUpdate(Job* job) {
    mutex_.AssertHeld();
    return;
}

void JobManager::CheckUpdating(Job* job) {
    mutex_.AssertHeld();
    for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
        it != job->pods_.end(); ++it) {
        PodInfo* pod = it->second;
        if (pod->update_time() != job->update_time_) {
            LOG(INFO) << "pod : " << pod->podid() << " updating " 
            << __FUNCTION__;  
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
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
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
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        return;
    }
    return;
}

void JobManager::CheckClear(Job* job) {
    mutex_.AssertHeld();
    VLOG(10) << "DEBUG: CheckClear ";
    for (std::map<PodId, PodInfo*>::iterator it = job->history_pods_.begin();
        it != job->history_pods_.end(); it++) {
        PodInfo* podinfo = it->second;
        job->history_pods_.erase(it->first);
        delete podinfo;
    }

    JobId id = job->id_;
    std::map<JobId, Job*>::iterator job_it = jobs_.find(id);
    if (job_it != jobs_.end()) {
        Job* job = job_it->second;
        jobs_.erase(id);
        VLOG(10) << "erase job :" << id << "DEBUG END";
        delete job;
    }
    DeleteFromNexus(id);
    return;
}

void JobManager::CheckJobStatus(Job* job) {
    MutexLock lock(&mutex_);
    if (job == NULL) {
        return;
    }
    VLOG(10) << "DEBUG: CheckJobStatus "
    << "jobid[" << job->id_ << "] status[" << JobStatus_Name(job->status_) << "]";
    std::map<std::string, AgingFunc>::iterator it = aging_.find(JobStatus_Name(job->status_));
    if (job->status_ != kJobFinished) {
        job_checker_.DelayTask(FLAGS_master_job_check_interval * 1000, boost::bind(&JobManager::CheckJobStatus, this, job));
    }
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
    if ((::baidu::common::timer::get_micros() - pod->heartbeat_time())/1000000 >
        FLAGS_master_pod_dead_time) {
        std::map<std::string, PodInfo*>::iterator it = job->pods_.find(pod->podid());
        if (it != job->pods_.end()) {
            job->pods_.erase(pod->podid());
            LOG(INFO) << "pod[" << pod->podid() << " heartbeat[" << 
                pod->heartbeat_time() << "] now[" <<  ::baidu::common::timer::get_micros()
                <<"] dead & remove. " << __FUNCTION__;
            DestroyService(pod->mutable_services());
            delete pod;
        }
        return;
    }
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval * 1000,
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
    job->last_desc_.CopyFrom(job_desc);
    job->action_type_ = kActionNull;

    job->create_time_ = ::baidu::common::timer::get_micros();
    job->update_time_ = ::baidu::common::timer::get_micros();
    job->rollback_time_ = ::baidu::common::timer::get_micros();
    SaveToNexus(job);   
    MutexLock lock(&mutex_); 
    jobs_[job_id] = job;
    job_checker_.DelayTask(FLAGS_master_job_check_interval * 1000, boost::bind(&JobManager::CheckJobStatus, this, job));
    LOG(INFO) << "job[" << job_id << "] jobname[" << job_desc.name() 
        <<"] step[" << job_desc.deploy().step() << "] replica[" 
        << job_desc.deploy().replica() << "]"
        << "submitted" << __FUNCTION__;
    return kOk;
}

Status JobManager::Update(const JobId& job_id, const JobDescription& job_desc) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "update job " << job_id << "failed."
        << "job not found" << __FUNCTION__;
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
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
            << "] reject event [" << JobEvent_Name(kUpdate) << "]" << __FUNCTION__;
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::PauseUpdate(const JobId& job_id) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "pause update job " << job_id << "failed."
        << "job not found" << __FUNCTION__;
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kPauseUpdate);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
            << "] reject event [" << JobEvent_Name(kPauseUpdate) << "]" << __FUNCTION__;
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::ContinueUpdate(const JobId& job_id, const int32_t break_point) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "continue update job " << job_id << "failed."
        << "job not found" << __FUNCTION__;
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kUpdateContinue);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, &break_point);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
            << "] reject event [" << JobEvent_Name(kUpdateContinue) << "]" << __FUNCTION__;
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::Rollback(const JobId& job_id) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING) << "rollback job " << job_id << "failed."
        << "job not found" << __FUNCTION__;
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kUpdateRollback);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
            << "] reject event [" << JobEvent_Name(kUpdateRollback) << "]" << __FUNCTION__;
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::Terminate(const JobId& jobid,
                            const User& user,
                            const std::string hostname) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return kJobNotFound;
    }
    Job* job = it->second;
    job->user_.CopyFrom(user);
    std::string fsm_key = BuildFsmKey(job->status_, kRemove);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    } else {
        LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
            << "] reject event [" << JobEvent_Name(kRemove) << "]" << __FUNCTION__;
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
    job->rollback_time_ = job->update_time_;
    job->update_time_ = ::baidu::common::timer::get_micros();
    job->last_desc_.CopyFrom(job->desc_);
    if (desc->pod().tasks_size() != job->desc_.pod().tasks_size()) {
        job->action_type_ = kActionRebuild;
        LOG(INFO) << "job : " << job->id_ << "set act_type : rebuild" << __FUNCTION__;     
    }

    for (int i = 0; i < desc->pod().tasks_size(); i++) {
        for (int j = 0; j < job->desc_.pod().tasks_size(); j++) {
            if (desc->pod().tasks(i).id() !=
                job->desc_.pod().tasks(j).id()) {
                continue;
            }
            if (desc->pod().tasks(i).exe_package().package().version() !=
                job->desc_.pod().tasks(j).exe_package().package().version()) {
                job->action_type_ = kActionRebuild;
                LOG(INFO) << "job : " << job->id_ << "set act_type : rebuild" << __FUNCTION__; 
            } else if (desc->pod().tasks(i).data_package().packages_size() !=
                job->desc_.pod().tasks(j).data_package().packages_size()) {
                job->action_type_ = kActionRebuild;
                LOG(INFO) << "job : " << job->id_ << "set act_type : rebuild" << __FUNCTION__; 
            } else {
                for (int k = 0; k < job->desc_.pod().tasks(j).data_package().packages_size();
                    k++) {
                    if (desc->pod().tasks(i).data_package().packages(k).version() !=
                        job->desc_.pod().tasks(j).data_package().packages(k).version()) {
                        job->action_type_ = kActionReload;
                        LOG(INFO) << "job : " << job->id_ << "set act_type : reload" << __FUNCTION__; 
                    }
                }

            }
        }
    }
    job->desc_.CopyFrom(*desc);
    LOG(INFO) << "job desc update success: %s" << desc->name();
    return kOk;
}

Status JobManager::ContinueUpdateJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    int32_t break_point = *(int32_t*)arg; 
    if (break_point != 0) {
        job->desc_.deploy().set_update_break_count(break_point);
    } else {
        job->desc_.deploy().set_update_break_count(job->desc_.deploy().replica());
    }
    return kOk;
}

Status JobManager::RollbackJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    job->update_time_ = job->rollback_time_;
    job->desc_.CopyFrom(job->last_desc_);
    return kOk;
}

Status JobManager::RemoveJob(Job* job, void* arg) {
    return kOk;
}

Status JobManager::ClearJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    MutexLock lock(&resman_mutex_);
    proto::RemoveContainerGroupRequest* container_request = new proto::RemoveContainerGroupRequest();
    proto::RemoveContainerGroupResponse* container_response = new proto::RemoveContainerGroupResponse();
    container_request->mutable_user()->CopyFrom(*(User*)arg);
    container_request->set_id(job->id_);
    boost::function<void (const proto::RemoveContainerGroupRequest*, proto::RemoveContainerGroupResponse*, 
            bool, int)> call_back;
    call_back = boost::bind(&JobManager::RemoveContainerGroupCallBack, this, _1, _2, _3, _4);
    ResMan_Stub* resman;
    rpc_client_.GetStub(resman_endpoint_, &resman);
    rpc_client_.AsyncRequest(resman,
                            &ResMan_Stub::RemoveContainerGroup,
                            container_request,
                            container_response,
                            call_back,
                            5, 0);
    delete resman;
    return kOk;
}

void JobManager::RemoveContainerGroupCallBack(const proto::RemoveContainerGroupRequest* request,
                                          proto::RemoveContainerGroupResponse* response,
                                          bool failed, int) {
    boost::scoped_ptr<const proto::RemoveContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<proto::RemoveContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != kOk) {
        LOG(WARNING) << "fail to remove container group";
        return;
    }
    return;
}

PodInfo* JobManager::CreatePod(Job* job,
                            std::string podid,
                            std::string endpoint) {
    PodInfo* podinfo = new PodInfo();
    podinfo->set_podid(podid);
    podinfo->set_jobid(job->id_);
    podinfo->set_endpoint(endpoint);
    podinfo->set_status(kPodDeploying);
    podinfo->set_reload_status(kPodFinished);
    podinfo->set_start_time(::baidu::common::timer::get_micros());
    podinfo->set_update_time(job->update_time_);
    podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
    podinfo->set_fail_count(0);
    podinfo->set_last_normal_time(::baidu::common::timer::get_micros());
    podinfo->set_send_rebuild_time(::baidu::common::timer::get_micros());
    job->pods_[podid] = podinfo;
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval * 1000,
        boost::bind(&JobManager::CheckPodAlive, this, podinfo, job));
    LOG(INFO) << "DEBUG: CreatePod " << podinfo->DebugString()
    << "END DEBUG";
    return podinfo;
}

Status JobManager::PodHeartBeat(Job* job, void* arg) {
    mutex_.AssertHeld();
    proto::FetchTaskRequest* request = (proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    Status rlt_code = kOk;
    PodInfo* podinfo = NULL;
    if (pod_it != job->pods_.end()) {
        podinfo = pod_it->second;
        //repeated fetch from different worker
        if (podinfo->endpoint() != request->endpoint()) {  
            //abandon worker request
            if ((request->start_time()) < podinfo->start_time()) {
                LOG(WARNING) << "DEBUG: PodHeartBeat "
                << "abandon worker : " << request->endpoint()
                << "END DEBUG";
                rlt_code = kTerminate;
            } else {
                //worker replace
                podinfo->set_endpoint(request->endpoint());
                podinfo->set_status(kPodDeploying);
                podinfo->set_start_time(request->start_time());
                podinfo->set_update_time(job->update_time_);
                podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
                podinfo->set_last_normal_time(::baidu::common::timer::get_micros());
                podinfo->set_fail_count(request->fail_count());
                LOG(INFO) << "DEBUG: PodHeartBeat "
                    << "new worker : " << request->endpoint()
                    << "END DEBUG";
                rlt_code = kOk;
            }
        } else {
        //refresh
            RefreshPod(request, podinfo, job);
        }
    } else {
        //interval control
        if (request->status() == kPodStopping) {
            LOG(INFO) << "DEBUG: ignore finished pod ";
        } else if (request->status() == kPodStopping) {
            LOG(INFO << LOG(INFO) << "DEBUG: kill finished pod ";)
            rlt_code = kTerminate;
        } else if (job->deploying_pods_.size() >= job->desc_.deploy().step()) {
            LOG(WARNING) << "DEBUG: fetch task deny "
            << " deploying: " << job->deploying_pods_.size()
            << " step: " << job->desc_.deploy().step();
            rlt_code = kSuspend;
        } else if (job->pods_.size() >= job->desc_.deploy().replica()) {
            LOG(WARNING) << "DEBUG: fetch task deny "
            << " pod cnt: " << job->pods_.size() 
            << " replica: " << job->desc_.deploy().replica();
            rlt_code = kTerminate;
        } else if (request->status() != kPodPending) {
            LOG(INFO) << "DEBUG: rebuild previous pod "
            << " jobid: " << request->jobid()
            << " podid: " << request->podid()
            << " endpoint: " << request->endpoint()
            << " END DEBUG";
            podinfo = CreatePod(job, request->podid(), request->endpoint());
            RefreshPod(request, podinfo, job);   
            rlt_code = kOk;
        } else { 
            podinfo = CreatePod(job, request->podid(), request->endpoint());
            job->deploying_pods_.insert(podinfo->podid());
            rlt_code = kOk;
        }
    }
    if (podinfo != NULL && podinfo->status() == kPodFinished) {
        job->pods_.erase(podinfo->podid());
        DestroyService(podinfo->mutable_services());
        job->history_pods_[podinfo->podid()] = podinfo;
        rlt_code = kTerminate;
    } else if (podinfo != NULL && podinfo->status() == kPodFailed) {
        if ((::baidu::common::timer::get_micros() - podinfo->last_normal_time()) / 1000000 > 
            FLAGS_master_fail_last_threshold) {
            rlt_code = kRebuild;
        } else {
            rlt_code = kSuspend;
        }
    }
    LOG(INFO) << __FUNCTION__ << " code : " << rlt_code;
    return rlt_code;
}

Status JobManager::TryRebuild(Job* job, PodInfo* podinfo) {
    if (job->deploying_pods_.size() >= job->desc_.deploy().step()) {
        LOG(WARNING) << "DEBUG: TryRebuild suspend "
        << " deploying: " << job->deploying_pods_.size()
        << " step: " << job->desc_.deploy().step();
        return kSuspend;
    } else {        
        job->deploying_pods_.insert(podinfo->podid());
        return kRebuild;
    }
}

Status JobManager::TryReload(Job* job, PodInfo* pod) {
    if (job->reloading_pods_.size() > job->desc_.deploy().step()) {
        LOG(WARNING) << "DEBUG: TryReload suspend "
        << " deploying: " << job->reloading_pods_.size()
        << " step: " << job->desc_.deploy().step();
        return kSuspend;
    } else {
        job->reloading_pods_.insert(pod->podid());
        return kReload;
    }
}

void JobManager::ReduceUpdateList(Job* job, 
                                 std::string podid,
                                 PodStatus pod_status,
                                 PodStatus reload_status) {
    if (pod_status > kPodServing 
        && job->deploying_pods_.find(podid) != job->deploying_pods_.end()) {
        job->deploying_pods_.erase(podid);
    } else if((reload_status == kPodFinished || reload_status == kPodFailed) &&
        job->reloading_pods_.find(podid) != job->reloading_pods_.end()) {
        job->reloading_pods_.erase(podid);
    }
    return;
}

bool JobManager::ReachBreakpoint(Job* job) {
    std::map<PodId, PodInfo*>::iterator it = job->pods_.begin();
    uint32_t updated_cnt = 0;
    for (; it != job->pods_.end(); it++) {
        PodInfo* pod_ptr = it->second;
        if (pod_ptr->send_rebuild_time() == job->update_time_) {
            updated_cnt++;
        }
    }
    if (updated_cnt >= job->desc_.deploy().update_break_count()) {
        return true;
    } else {
        return false;
    }
}

bool JobManager::IsSerivceSame(const ServiceInfo& src, const ServiceInfo& dest) {
    if (src.name() != dest.name()) {
        return false;
    }
    if (src.port() != dest.port()) {
        return false;
    }
    if (src.status() != dest.status()) {
        return false;
    }
    if (src.ip() != dest.ip()) {
        return false;
    }
    return true;
}

void JobManager::RefreshService(ServiceList* src, PodInfo* pod) {
    for (int i = 0; i < pod->services().size(); i++) {
        bool found = false;
        for (int j = 0; j < src->size(); j++) {
            if (pod->services(i).name() == src->Get(j).name()) {
                found = true;
                break;
            }
        }
        if (!found) {
            pod->mutable_services()->DeleteSubrange(i, 1);
            //todo
        }
    }
    for (int i = 0; i < src->size(); i++) {
        ServiceInfo src_serv = src->Get(i);
        bool need_refresh_naming = false;
        bool found = false;
        for (int j = 0; j < pod->services().size(); j++) {
            if (src_serv.name() != pod->services(j).name()) {
                continue;
            } else {
                found = true;
                if (!IsSerivceSame(src_serv, pod->services(j))) {
                    pod->mutable_services(j)->CopyFrom(src_serv);
                    need_refresh_naming = true;
                    LOG(INFO) << "refresh service : "
                    << " name : " << pod->services(j).name()
                    << " ip : " << pod->services(j).ip()
                    << " port : " << pod->services(j).port()
                    << " status : " << pod->services(j).status();
                }
                break;
            }
        }
        if (!found) {
            pod->add_services()->CopyFrom(src_serv);
            LOG(INFO) << " add service : " << src_serv.name();
            need_refresh_naming = true;
        }
    }
    return;
}

void JobManager::DestroyService(ServiceList* services) {
    for (int i = 0; i < services->size(); i++) {
        ServiceInfo serv = services->Get(i);
        //todo
    }
    services->Clear();
    return;
}

void JobManager::RefreshPod(::baidu::galaxy::proto::FetchTaskRequest* request,
                            PodInfo* podinfo,
                            Job* job) {
    podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
    podinfo->set_fail_count(request->fail_count());
    if (request->fail_count() == 0) {
        podinfo->set_last_normal_time(::baidu::common::timer::get_micros());
    }
    podinfo->set_status(request->status());
    podinfo->set_reload_status(request->reload_status());
    ReduceUpdateList(job, podinfo->podid(), request->status(), request->reload_status());
    if (request->().size() != 0) {
        LOG(INFO) << "DEBUG servie msg : "
        << request->DebugString();
        RefreshService(request->mutable_services(), podinfo);
    } else {
        DestroyService(podinfo->mutable_services());
    }
    LOG(INFO) << "DEBUG: PodHeartBeat "
            << "refresh pod id : " << request->podid() << " status :"
            << podinfo->status() << " heartbeat time : " << podinfo->heartbeat_time()
            << " last_normal_time : " << podinfo->last_normal_time()
            << " fail count : " << podinfo->fail_count()
            << " services : " << podinfo->services().size()
            << "END DEBUG";
    return;
}

Status JobManager::PauseUpdatePod(Job* job, void* arg) {
    mutex_.AssertHeld();
    proto::FetchTaskRequest* request = (proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    Status rlt_code = kSuspend;
    PodInfo* podinfo = NULL;
    if (pod_it != job->pods_.end()) {
        podinfo = pod_it->second;
        //repeated fetch from different worker
        if (podinfo->endpoint() != request->endpoint()) {  
            //abandon worker request
            if ((request->start_time()) < podinfo->start_time()) {
                LOG(WARNING) << "DEBUG: PodHeartBeat "
                << "abandon worker : " << request->endpoint()
                << "END DEBUG";
                rlt_code = kTerminate;
            } else {
                //worker replace
                podinfo->set_endpoint(request->endpoint());
                podinfo->set_status(kPodDeploying);
                podinfo->set_start_time(request->start_time());
                podinfo->set_update_time(job->update_time_);
                podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
                podinfo->set_last_normal_time(::baidu::common::timer::get_micros());
                podinfo->set_fail_count(request->fail_count());
                LOG(INFO) << "DEBUG: PodHeartBeat "
                    << "new worker : " << request->endpoint()
                    << "END DEBUG";
                rlt_code = kSuspend;
            }
        } else {
        //refresh
            RefreshPod(request, podinfo, job);
        }
    } else {
        //interval control
        if (request->status() == kPodStopping) {
            LOG(INFO) << "DEBUG: ignore finished pod ";
        } else if (request->status() == kPodStopping) {
            LOG(INFO << LOG(INFO) << "DEBUG: kill finished pod ";)
            rlt_code = kTerminate;
        } else if (job->pods_.size() >= job->desc_.deploy().replica()) {
            LOG(WARNING) << "DEBUG: fetch task deny "
            << " pod cnt: " << job->pods_.size() 
            << " replica: " << job->desc_.deploy().replica();
            rlt_code = kTerminate;
        } else if (request->status() != kPodPending) {
            LOG(INFO) << "DEBUG: rebuild previous pod "
            << " jobid: " << request->jobid()
            << " podid: " << request->podid()
            << " endpoint: " << request->endpoint()
            << " END DEBUG";
            podinfo = CreatePod(job, request->podid(), request->endpoint());
            RefreshPod(request, podinfo, job);   
            rlt_code = kSuspend;
        } else { 
            podinfo = CreatePod(job, request->podid(), request->endpoint());
            rlt_code = kSuspend;
        }
    }
    if (podinfo != NULL && podinfo->status() == kPodFinished) {
        job->pods_.erase(podinfo->podid());
        DestroyService(podinfo->mutable_services());
        job->history_pods_[podinfo->podid()] = podinfo;
        rlt_code = kTerminate;
    } 
    LOG(INFO) << __FUNCTION__ << " code : " << rlt_code;
    return rlt_code;
}

Status JobManager::UpdatePod(Job* job, void* arg) {
    mutex_.AssertHeld();
    ::baidu::galaxy::proto::FetchTaskRequest* request =
    (::baidu::galaxy::proto::FetchTaskRequest*)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    Status rlt_code = kOk;
    PodInfo* podinfo = NULL;

    if(pod_it == job->pods_.end()) {
        if (job->pods_.size() >= job->desc_.deploy().replica()) {
            //replica control
            LOG(WARNING) << "DEBUG: fetch task deny "
            << " pod cnt: " << job->pods_.size() 
            << " replica: " << job->desc_.deploy().replica();
            return kTerminate;
        }
        podinfo = CreatePod(job, request->podid(), request->endpoint());
        if (request->status() != kPodPending) {
            //previous pod       
            LOG(INFO) << "DEBUG: rebuild previous pod "
            << " jobid: " << request->jobid()
            << " podid: " << request->podid()
            << " endpoint: " << request->endpoint()
            << " END DEBUG";
            podinfo->set_status(request->status());
            podinfo->set_start_time(request->start_time());
        }
    } else {
        podinfo = pod_it->second;
        RefreshPod(request, podinfo, job);
    }
    //update process
    if (job->update_time_ != request->update_time()) {
        if (ReachBreakpoint(job)) {
            std::string fsm_key = BuildFsmKey(job->status_, kPauseUpdate);
            std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
            if (fsm_it != fsm_.end()) {
                Status rlt = fsm_it->second->trans_func_(job);
                if (kOk != rlt) {
                    LOG(WARNING) << "trans func exec fail . Status : " << rlt;
                }
                job->status_ = fsm_it->second->next_status_;
                LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
                JobStatus_Name(job->status_);
                SaveToNexus(job);
            } else {
                LOG(INFO) << "job[" << job->id_ << "][" << JobStatus_Name(job->status_) 
                    << "] reject event [" << JobEvent_Name(kPauseUpdate) << "]" << __FUNCTION__;
                return kStatusConflict;
            }
            rlt_code = kSuspend;
        } else if (job->action_type_ == kActionNull) {
            rlt_code = kOk;
        } else if (job->action_type_ == kActionRebuild) {
            rlt_code = TryRebuild(job, podinfo);
        } else if (job->action_type_ == kActionReload) {
            rlt_code = TryReload(job, podinfo);
        } 
    } else {
        podinfo->set_update_time(job->update_time_);
    }
    LOG(INFO) << "pod : " << request->podid() << "update status :" 
        << Status_Name(rlt_code) << " " << __FUNCTION__;
    return rlt_code;
}

Status JobManager::DistroyPod(Job* job, void* arg) {
    mutex_.AssertHeld();
    return kTerminate;
}

Status JobManager::HandleFetch(const ::baidu::galaxy::proto::FetchTaskRequest* request,
                             ::baidu::galaxy::proto::FetchTaskResponse* response) {
    MutexLock lock(&mutex_);
    std::map<std::string, Job*>::iterator job_it = jobs_.find(request->jobid());
    if (job_it == jobs_.end()) {
        response->mutable_error_code()->set_status(kJobNotFound);
        response->mutable_error_code()->set_reason("Jobid not found");
        LOG(WARNING) << "Fetch job[" << request->jobid() << "]" 
        << "from worker[" << request->endpoint() << "][" 
        << request->podid() << "]" << "failed." << __FUNCTION__;
        return kJobNotFound;
    }
    Job* job = job_it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kFetch);
    std::map<std::string, FsmTrans*>::iterator fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it->second->trans_func_(job, NULL);
        if (kOk != rlt) {
            LOG(WARNING) << "FSM trans exec failed" << __FUNCTION__;
            return rlt;
        }
        job->status_ = fsm_it->second->next_status_;
        LOG(INFO) << "job[" << job->id_ << "] status trans to : " << 
        JobStatus_Name(job->status_);
        SaveToNexus(job);
    }
    std::map<std::string, DispatchFunc>::iterator dispatch_it = dispatch_.find(JobStatus_Name(job->status_));
    if (dispatch_it == dispatch_.end()) {
        LOG(WARNING) << "dispatch_func null." << __FUNCTION__;
        return kError;
    }
    Status rlt = dispatch_it->second(job, (void*)request);
    response->mutable_error_code()->set_status(rlt);
    if (kError == rlt) {
        LOG(WARNING) << "dispatch_func exec failed." << __FUNCTION__;
        return rlt;
    }
    response->set_update_time(job->update_time_);
    response->mutable_pod()->CopyFrom(job->desc_.pod());
    return kOk;
}

void JobManager::ReloadJobInfo(const JobInfo& job_info) {
    Job* job = new Job();
    job->status_ = job_info.status();
    job->user_.CopyFrom(job_info.user());
    job->desc_.CopyFrom(job_info.desc());
    job->id_ = job_info.jobid();
    job->last_desc_.CopyFrom(job_info.last_desc());
    job->create_time_ = job_info.create_time();
    job->update_time_ = job_info.update_time();
    job->action_type_ = job_info.action();   
    MutexLock lock(&mutex_);
    jobs_[job->id_] = job;
    job_checker_.DelayTask(FLAGS_master_job_check_interval * 1000, boost::bind(&JobManager::CheckJobStatus, this, job));
    return;
}

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
        uint32_t state_stat[kPodTerminated + 1] = {0};
        for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
            it != job->pods_.end(); it++) {
            const PodInfo* pod = it->second;
            state_stat[pod->status()]++;
        }

        overview->set_running_num(state_stat[kPodRunning]);
        overview->set_deploying_num(state_stat[kPodDeploying] + state_stat[kPodStarting] + state_stat[kPodReady]);
        overview->set_death_num(state_stat[kPodFinished] + state_stat[kPodFailed] + state_stat[kPodStopping] +
            state_stat[kPodTerminated] + job->history_pods_.size());
        overview->set_pending_num(job->desc_.deploy().replica() - 
            overview->deploying_num() - overview->death_num() - overview->running_num());
        overview->set_create_time(job->create_time_);
        overview->set_update_time(job->update_time_);
        VLOG(10) << "DEBUG GetJobsOverview: " << overview->DebugString()
        << "DEBUG END";
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
    job_info->mutable_desc()->CopyFrom(job->desc_);
    std::map<PodId, PodInfo*>::iterator pod_it = job->pods_.begin();
    for (; pod_it != job->pods_.end(); ++pod_it) {
        PodInfo* pod = pod_it->second;
        job_info->add_pods()->CopyFrom(*pod);
    }
    for (pod_it = job->history_pods_.begin(); pod_it != job->history_pods_.end(); ++pod_it) {
        PodInfo* pod = pod_it->second;
        job_info->add_pods()->CopyFrom(*pod);
    }
    VLOG(10) << "DEBUG GetJobInfo: " << job_info->DebugString()
        << "DEBUG END";
    return kOk;
}

bool JobManager::SaveToNexus(const Job* job) {
    if (job == NULL) {
        return false;
    }
    JobInfo job_info;
    job_info.set_jobid(job->id_);
    job_info.set_status(job->status_);
    job_info.mutable_last_desc()->CopyFrom(job->last_desc_);
    job_info.mutable_desc()->CopyFrom(job->desc_);
    job_info.mutable_user()->CopyFrom(job->user_);
    job_info.set_action(job->action_type_);
    job_info.set_create_time(job->create_time_);
    job_info.set_update_time(job->update_time_);
    std::string job_raw_data;
    std::string job_key = FLAGS_nexus_root + FLAGS_jobs_store_path 
                          + "/" + job->id_;
    job_info.SerializeToString(&job_raw_data);
    ::galaxy::ins::sdk::SDKError err;
    bool put_ok = nexus_->Put(job_key, job_raw_data, &err);
    if (!put_ok) {
        LOG(WARNING) << "fail to put job " <<job_info.desc().name()
        << " id " << job_info.jobid() << " to nexus err msg"
        << ::galaxy::ins::sdk::InsSDK::StatusToString(err);
    }
    return true; 
}

bool JobManager::DeleteFromNexus(const JobId& job_id) {
    std::string job_key = FLAGS_nexus_root + FLAGS_jobs_store_path 
                          + "/" + job_id;
    ::galaxy::ins::sdk::SDKError err;
    bool delete_ok = nexus_->Delete(job_key, &err);
    if (!delete_ok) {
        LOG(WARNING) << "fail to delete job :" << job_key 
        << "from nexus err msg " 
        << ::galaxy::ins::sdk::InsSDK::StatusToString(err);
    }
    return true;
}

void JobManager::SetResmanEndpoint(std::string new_endpoint) {
    MutexLock lock(&resman_mutex_);
    resman_endpoint_ = new_endpoint;
    return;
}

}
}
