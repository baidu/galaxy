// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "job_manager.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <gflags/gflags.h>
#include "proto/agent.pb.h"
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "master_util.h"
#include "utils/resource_utils.h"
#include "timer.h"
#include <logging.h>
#include "utils/trace.h"

DECLARE_int32(master_agent_timeout);
DECLARE_int32(master_agent_rpc_timeout);
DECLARE_int32(master_query_period);
DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_int32(master_pending_job_wait_timeout);
DECLARE_int32(master_job_trace_interval);
DECLARE_int32(master_cluster_trace_interval);
DECLARE_int32(master_preempt_interval);
namespace baidu {
namespace galaxy {
JobManager::JobManager()
    : on_query_num_(0), 
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_servers);
}

JobManager::~JobManager() {
    delete nexus_;
    delete job_index_;
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
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kFetch)),
        BuildFsmValue(kJobRunning, boost::bind(&JobManager::StartJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kRemove)),
        BuildFsmValue(kJobFinish, boost::bind(&JobManager::FinishJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobPending, kUpdate)),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kUpdate),
        BuildFsmValue(kJobUpdating, boost::bind(&JobManager::UpdateJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobRunning, kRemove),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RemoveJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kUpdateFinish),
        BuildFsmValue(kJobDestroying, boost::bind(&JobManager::RecoverJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobUpdating, kRemove),
        BuildFsmValue(kJobRunning, boost::bind(&JobManager::RemoveJob, this, _1, _2)));
    fsm_.insert(std::make_pair(BuildFsmKey(kJobDestroying, kRemoveFinish),
        BuildFsmValue(kJobFinish, boost::bind(&JobManager::ClearJob, this, _1, _2)));
    return;
}

void JobManager::BuildDispatch() {
    dispatch_.insert(std::make_pair(kJobPending, boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(kJobRunning, boost::bind(&JobManager::PodHeartBeat, this, _1, _2)));
    dispatch_.insert(std::make_pair(kJobUpdating, boost::bind(&JobManager::UpdatePod, this, _1, _2)));
    dispatch_.insert(std::make_pair(kJobDestroying, boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    dispatch_.insert(std::make_pair(kJobFinish, boost::bind(&JobManager::DistroyPod, this, _1, _2)));
    return;
}

void JobManager::BuildAging() {
    aging_.insert(std::make_pair(kJobPending, boost::bind(&JobManager::CheckPending, this, _1, _2)));
    aging_.insert(std::make_pair(kJobRunning, boost::bind(&JobManager::CheckRunning, this, _1, _2)));
    aging_.insert(std::make_pair(kJobUpdating, boost::bind(&JobManager::CheckUpdating, this, _1, _2)));
    aging_.insert(std::make_pair(kJobDestroying, boost::bind(&JobManager::CheckDestroying, this, _1, _2)));
    aging_.insert(std::make_pair(kJobFinish, boost::bind(&JobManager::CheckClear, this, _1, _2)));
    return;
}

Status JobManager::CheckPending(Job* job) {
    return kOk;
}

Status JobManager::CheckRunning(Job* job) {
    return kOk;
}

Status JobManager::CheckUpdating(Job* job) {
    mutex_.AssertHeld();
    for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
        it != job->pods_.end(); ++it) {
        PodInfo* pod = it->second;
        if (pod->update_time() != job->update_time_) {
            //log updating
            return kOk;
        }
    }
    std::string fsm_key = BuildFsmKey(job->status_, kUpdateFinish);
    std::map<std::string, FsmTrans*> fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it.second.trans_func_(job, NULL);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it.second.next_status_;
        SaveToNexus(job);
    } else {
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::CheckDestroying(Job* job) {
    mutex_.AssertHeld();
    if (job->pods_.size() != 0) {
        return kOk;
    }
    std::string fsm_key = BuildFsmKey(job->status_, kRemoveFinish);
    std::map<std::string, FsmTrans*> fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it.second.trans_func_(job, job->user_.c_str());
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it.second.next_status_;
        SaveToNexus(job);
    } else {
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::CheckClear(Job* job) {
    return ClearJob(job, job->user_.c_str());
}

Status JobManager::CheckJobStatus(Job* job) {
    MutexLock lock(&mutex_);
    if (job == NULL) {
        return kError;
    }
    job_checker_.DelayTask(FLAGS_master_job_check_interval, boost::bind(&JobManager::CheckJobStatus, this, job));
    std::map<JobStatus, DispatchFunc>::iterator it = aging_.find(job->status_);
    if (it == aging_.end()) {
        return kError;
    }
    Status rlt = it->second(Job);
    return rlt;
}

Status JobManager::CheckPodAlive(PodInfo* pod, Job* job) {
    MutexLock lock(&mutex_);
    if (job == NULL || pod == NULL) {
        return kError;
    }
    if ((::baidu::common::timer::get_micros() - pod->heartbeat_time())/1000 >
        FLAGS_master_pod_dead_time) {
        std::map<std::string, PodInfo*>::iterator it = job->pods_.find(pod->podid());
        if (it != ob->pods_.end()) {
            job->pods_.erase(pod->podid());
            delete pod;
        }
        return kOk;
    }
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval,
        boost::bind(&JobManager::CheckPodAlive, this, pod, job));
    return kOk;
}

Status JobManager::Add(const std::string job_id, const JobDescription job_desc) { 
    Job* job = new Job();
    job->state_ = kJobPending;
    job->desc_.CopyFrom(job_desc);
    job->id_ = job_id;
    // add default version
    if (!job->desc_.has_version()) {
        job->desc_.set_version("1.0.0");
    }
    job->pod_desc_[job->desc_.version()] = job->desc_;
    job->curent_version_ = job->desc_.pod().version();
    job->opt_type_ = kActionNull;

    job->create_time = ::baidu::common::timer::get_micros();
    job->update_time = ::baidu::common::timer::get_micros();
    job->pod_desc_[JobDescriptor->version()] = job_desc->pod_decs();
    // TODO add nexus lock
    bool save_ok = SaveToNexus(job);
    if (!save_ok) {
        return kJobSubmitFail;
    }
    
    MutexLock lock(&mutex_); 
    jobs_[job_id] = job;
    LOG(INFO, "job %s[%s] submitted with deploy_step %d, replica %d , pod version %s",
      job_id.c_str(), 
      job_desc.name().c_str(),
      job_desc.deploy().step(),
      job_desc.deploy()replica(),
      job->latest_version.c_str());
    trace_pool_.DelayTask(FLAGS_master_job_trace_interval, boost::bind(&JobManager::TraceJobStat, this, job_id));
    return kOk;
}

Status JobManager::Update(const JobId& job_id, const JobDescriptor& job_desc) {
    Job job;
    // TODO add nexus lock
    bool save_ok = SaveToNexus(&job);
    if (!save_ok) {
        return kJobUpdateFail;
    }
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator it;
    it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        LOG(WARNING, "update job failed, job not found: %s", job_id.c_str());
        return kJobNotFound;
    }
    Job* job = it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kUpdate);
    std::map<std::string, FsmTrans*> fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it.second.trans_func_(job, &job_desc);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it.second.next_status_;
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
    std::string fsm_key = BuildFsmKey(job->status_, kTerminate);
    std::map<std::string, FsmTrans*> fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it.second.trans_func_(job, &job_desc);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it.second.next_status_;
    } else {
        return kStatusConflict;
    }
    return kOk;
}

Status JobManager::StartJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    return kOk;
}

Status JobManager::UpdateJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    JobDescription* desc = (JobDescription*)arg;
    job->update_time_ = ::baidu::common::timer::get_micros();
    job->curent_version_ = desc->version();
    job->pod_desc_[desc->version()] = desc->pod_decs();
    job->replica_ = desc->depoly().replica();
    for (int i = 0; i < desc.pod_decs().tasks_size(); i++) {
        for (int j = 0; j < job->desc_.tasks_size(); j++) {
            if (desc->pod_decs().tasks[i].id() !=
                job->desc_.tasks[j].id()) {
                continue;
            }
            if (desc->pod_decs().tasks[i].exe_package().version() !=
                job->desc_tasks[j].exe_package().version()) {
                job->action_type_ = kActionRebuild;
            } else if (desc->pod_decs().tasks[i].data_package().version() !=
                job->desc_tasks[j].data_package().version()) {
                job->action_type_ = kActionReload;
            }
        }
    }
    for (std::map<std::string, PodInfo*>::iterator it = pods_.begin();
        it != pods_.end(); it++) {
        it->second->set_act(UpdateAction);
        it->second->set_update_time(job->update_time);
    }

    LOG(INFO, "job desc updated succes: %s", job_desc.name().c_str());
    return kOk;
}

Status JobManager::RemoveJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    job.desc_.set_replica(0);
    return kOk;
}

Status JobManager::ClearJob(Job* job, void* arg) {
    mutex_.AssertHeld();
    StopContainerGroupRequest* container_request = new StopContainerGroupRequest();
    StopContainerGroupResponse* container_response = new StopContainerGroupResponse();
    container_request->set_user((char*)arg);
    container_request->set_id(job->id_)；
    boost::function<void (const StopContainerGroupRequest*, StopContainerGroupResponse*, 
            bool, int)> call_back;
    callback = boost::bind(&JobManager::StopContainerGroupCallBack, this, _1, _2, _3, _4);
    rpc_client.AsyncRequest(resman_,
                            &ResMan_Stub::StopContainerGroup,
                            container_request,
                            container_response,
                            callback,
                            5, 0);
    return kOk;
}

void JobManager::StopContainerGroupCallBack(const StopContainerGroupRequest* request,
                                          StopContainerGroupResponse* response,
                                          bool failed, int) {
    MutexLock lock(&resman_mutex_);
    boost::scoped_ptr<const StopContainerGroupRequest> request_ptr(request);
    boost::scoped_ptr<StopContainerGroupResponse> response_ptr(response);
    if (failed || response_ptr->error_code().status() != kOk) {
        LOG(WARNING, "fail to remove container group");
        return;
    }
    std::map<std::string, Job*>::iterator job_it = jobs_.find(request->id());
    if (job_it != jobs_.end()) {
        Job* job = job_it->second;
        jobs_.erase(request->id());
        delete job;
    }
    if (DeleteFromNexus(request->id()) != true) {
        //LOG
    }
    return;
}

void JobManager::CreatePod(Job* job,
                            std::string podid,
                            std::string endpoint,
                            std::string containerid) {
    PodInfo* podinfo = new PodInfo();
    podinfo->set_podid(podid);
    pobinfo_->set_jobid(job->jobid);
    pobinfo_->set_endpoint(endpoint);
    podinfo_->set_containerid(containerid);
    pobinfo_->set_status(kPodDeploying);
    pobinfo_->set_version(job->curent_version_);
    podinfo_->set_create_time(::baidu::common::timer::get_micros());
    pobinfo_->set_update_time(::baidu::common::timer::get_micros());
    job.pods_[request->podid()] = podinfo;
    job->deploying_pods_.insert(request->containerid());
    pod_checker_.DelayTask(FLAGS_master_pod_check_interval,
        boost::bind(&JobManager::CheckPodAlive, this, pod, job));
    return;
}

Status JobManager::PodHeartBeat(Job* job, void* arg) {
    mutex_.AssertHeld();
    ::baidu::galaxy::proto::FetchTaskRequest* request = 
    (::baidu::galaxy::proto::FetchTaskRequest)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    //heartbeat
    if (pod_it != job->pods_.end()) {
        //repeated fetch from different worker
        if (pod_it->second->containerid() != request->containerid()) {  
            //abandon worker request
            if ((request->update_time()) < pod_it->second->update_time()) {
                return kTerminate;
            }
            //new worker request
            pod_it->second->set_endpoint(request->endpoint());
            pod_it->second->set_containerid(request->containerid());
            pod_it->second->set_status(kPodDeploying);
            pod_it->set_version(job_it->curent_version_);
            pod_it->set_create_time(job_it->create_time_);
            pod_it->set_update_time(::baidu::common::timer::get_micros());
            return kOk;
        }
        PodInfo* podinfo = pod_it.second;
        //refresh
        podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
        if (request->status() != kPodDeploying &&
            job->depolying_containers_.find(request->containerid() != 
            job->depolying_containers_.end())) {
            job->deploying_pods_.erase(request->containerid());
        }
        return kOk;
    }
    //new pod
    if (job->depolying_containers_.size() >= job->desc_.deploy().step()) {
        return kDeny;
    }
    if (job->replica_cnt >= job->desc_.depoly().replica()) {
        return kDeny;
    }
    CreatePod(job, request->podid(), request->endpoint(), request->containerid());
    return kOk;
}

Status JobManager::UpdatePod(Job* job, void* arg) {
    mutex_.AssertHeld();
    ::baidu::galaxy::proto::FetchTaskRequest* request =
    (::baidu::galaxy::proto::FetchTaskRequest)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    if (pod_it == job->pods_.end()) {
        //new pod
        if (job->depolying_containers_.size() >= job->desc_.deploy().step()) {
            return kDeny;
        }
        if (job->replica_cnt >= job->desc_.depoly().replica()) {
            return kDeny;
        }
        CreatePod(job, request->podid(), request->endpoint(), request->containerid());
        return kOk;
    }
    PodInfo* podinfo = pod_it.second;
    //fresh
    podinfo->set_heartbeat_time(::baidu::common::timer::get_micros());
    if (request->status() != kPodDeploying &&
        job->depolying_containers_.find(request->containerid() != 
        job->depolying_containers_.end())) {
        job->deploying_pods_.erase(request->containerid());
    }
    //
    if (pod_it->second->update_time <= request->update_time()) {
        Status rlt_code;
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
    (::baidu::galaxy::proto::FetchTaskRequest)arg;
    std::map<std::string, PodInfo*>::iterator pod_it = job->pods_.find(request->podid());
    if (pod_it == job->pods_.end()) {
        return kTerminate;
    }
    PodInfo* podinfo = pod_it.second;
    job->pods_.erase(podinfo->id());
    delete podinfo;
    return kTerminate;
}

Status JobManager::HandleFetch(const ::baidu::galaxy::proto::FetchTaskRequest* request,
                             ::baidu::galaxy::proto::FetchTaskResponse* response) {
    MutexLock lock(&mutex_);
    std::map<std::string, Job*>::iterator job_it = jobs_.find(request->jobid());
    if (job_it == jobs_.end()) {
        ErrorCode error_code;
        error_code.set_status(kJobNotFound);
        error_code.set_reason("Jobid not found");
        response->set_error_code(error_code);
        return kJobNotFound;
    }
    Job* job = job_it->second;
    std::string fsm_key = BuildFsmKey(job->status_, kFetch);
    std::map<std::string, FsmTrans*> fsm_it = fsm_.find(fsm_key);
    if (fsm_it != fsm_.end()) {
        Status rlt = fsm_it.second.trans_func_(job);
        if (kOk != rlt) {
            return rlt;
        }
        job->status_ = fsm_it.second.next_status_;
    } else {
        return kStatusConflict;
    }
    std::map<JobStatus, DispatchFunc>::iterator dispatch_it = dispatch_.find(job->status_);
    if (dispatch_it == dispatch_.end()) {
        return kError;
    }
    Status rlt = dispatch_it.second(Job);
    ErrorCode error_code;
    error_code.set_status(rlt);
    response->set_error_code(error_code);
    if (kError == rlt) {
        return rlt;
    }
    response->set_update_time(pobinfo_->update_time());
    response->set_job(job_it.desc_);
    return kOk;
}

void JobManager::ReloadJobInfo(const JobInfo& job_info) {
    Job* job = new Job();
    job->state_ = job_info.state();
    job->desc_.CopyFrom(job_info.desc());
    std::string job_id = job_info.jobid();
    job->id_ = job_id;
    job->update_state_ = job_info.update_state();
    job->latest_version = job_info.latest_version();
    for (int32_t i = 0; i < job_info.pod_descs_size(); i++) {
        const PodDescriptor& desc = job_info.pod_descs(i);
        job->pod_desc_[desc.version()] = desc;
    }
    if (job_info.has_create_time()) {
        job->create_time = job_info.create_time();
    } 

    if (job_info.has_update_time()) {
        job->update_time = job_info.update_time();
    }
    JobIndex index;
    index.id_ = job_id;
    index.name_ = job->desc_.name();
    MutexLock lock(&mutex_);
    const JobSetNameIndex& name_index = job_index_->get<name_tag>();
    if (name_index.find(index.name_) != name_index.end()) {
        LOG(WARNING, "job with name %s has being existing, ignore it", index.name_.c_str());
    } else {
        job_index_->insert(index);
    }
    jobs_[job_id] = job;
    trace_pool_.DelayTask(FLAGS_master_job_trace_interval, 
                          boost::bind(&JobManager::TraceJobStat, this, job_id));
}

bool JobManager::GetJobIdByName(const std::string& job_name, 
                                std::string* jobid) {
    if (jobid == NULL) {
        return false;
    }
    MutexLock lock(&mutex_);
    const JobSetNameIndex& name_index = job_index_->get<name_tag>();
    JobSetNameIndex::const_iterator it = name_index.find(job_name);
    if (it == name_index.end()) {
        return false;
    }
    *jobid = it->id_;
    return true;
}

void JobManager::GetJobsOverview(JobOverviewList* jobs_overview) {
    MutexLock lock(&mutex_);
    optional int64 update_time = 10;
    std::map<string, Job*>::iterator job_it = jobs_.begin();
    for (; job_it != jobs_.end(); ++job_it) {
        const std::string jobid = job_it->first;
        Job* job = job_it->second;
        JobOverview* overview = jobs_overview->Add();
        overview->mutable_desc()->CopyFrom(job->desc_);
        overview->set_jobid(jobid);
        overview->set_status(job->state_);
        uint32_t state_stat[kPodFinished + 1] = 0;
        for (std::map<std::string, PodInfo*>::iterator it = job->pods_.begin();
            it != job->pods_.end(); it++) {
            const PodInfo* pod = it->second;
            state_stat[pod->state()]++;
        }
        overview->set_running_num(state_stat[kPodServing]);
        overview->set_pending_num(state_stat[kPodPending]);
        overview->set_deploying_num(state_stat[kPodPending] + state_stat[kPodStarting);
        overview->set_death_num(state_stat[kPodFinished] + state_stat[kPodFailed]);
        overview->set_create_time(job->create_time);
        overview->set_update_time(job->update_time);
    }
}

Status JobManager::GetJobInfo(const JobId& jobid, JobInfo* job_info) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        LOG(WARNING, "get job info failed, no such job: %s", jobid.c_str());
        return kJobNotFound;
    }
    Job* job = job_it->second;
    job_info->set_jobid(jobid);
    job_info->set_status(job->state_);
    job_info->set_create_time(job->create_time_);
    job_info->set_update_time(job->update_time_);
    job_info->set_version(job->curent_version_);
    job_info->mutable_desc()->CopyFrom(job->desc_);
    std::map<PodId, PodStatus*>::iterator pod_it = job->pods_.begin();
    for (; pod_it != job->pods_.end(); ++pod_it) {
        PodStatus* pod = pod_it->second;
        job_info->add_pods()->CopyFrom(*pod);
    }
    return kOk;
}

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
    std::map<Version, PodDescriptor>::const_iterator it = job->pod_desc_.begin();
    for(; it != job->pod_desc_.end(); ++it) {
        PodDescriptor* pod_desc = job_info.add_pod_descs();
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

bool JobManager::DeleteFromNexus(const JobId& job_id) {
    std::string job_key = FLAGS_nexus_root_path + FLAGS_jobs_store_path 
                          + "/" + job_id;
    ::galaxy::ins::sdk::SDKError err;
    bool delete_ok = nexus_->Delete(job_key, &err);
    if (!delete_ok) {
        LOG(WARNING, "fail to delete job %s fxwrom nexus err msg %s", 
          job_id.c_str(),
          ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
    }
    return delete_ok;
}

Status JobManager::GetPods(const std::string& jobid, 
                           PodOverviewList* pods) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        return kJobNotFound;
    }
    Job* job = job_it->second;
    std::map<PodId, PodStatus*>::iterator pod_it = job->pods_.begin();
    for (; pod_it != job->pods_.end(); ++pod_it) {
        PodStatus* pod_status = pod_it->second;
        std::map<Version, PodDescriptor>::iterator desc_it = job->pod_desc_.find(pod_status->version());
        if (desc_it == job->pod_desc_.end()) {
            continue;
        }
        PodOverview* pod = pods->Add();
        pod->set_jobid(pod_status->jobid());
        pod->set_podid(pod_status->podid());
        pod->set_stage(pod_status->stage());
        pod->set_state(pod_status->state());
        pod->set_version(pod_status->version());
        pod->set_endpoint(pod_status->endpoint());
        pod->mutable_used()->CopyFrom(pod_status->resource_used());
        pod->mutable_assigned()->CopyFrom(desc_it->second.requirement());
        pod->set_pending_time(pod_status->pending_time());
        pod->set_sched_time(pod_status->sched_time());
        pod->set_start_time(pod_status->start_time());
    }
    return kOk;
}

}
}
