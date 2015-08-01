// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "job_manager.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <gflags/gflags.h>
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "master_util.h"

DECLARE_int32(agent_timeout);

namespace baidu {
namespace galaxy {

void JobManager::Add(const JobDescriptor& job_desc) {
    Job* job = new Job();
    job->state_ = kJobNormal;
    job->desc_.CopyFrom(job_desc);
    std::string job_id = MasterUtil::GenerateJobId(job_desc);
    MutexLock lock(&mutex_);
    for(int i = 0; i < job_desc.replica(); i++) {
        std::string pod_id = MasterUtil::GeneratePodId(job_desc);
        PodStatus* pod_status = new PodStatus();
        pod_status->set_podid(pod_id);
        pod_status->set_jobid(job_id);
        job->pods_[pod_id] = pod_status;
        pending_pods_[job_id][pod_id] = pod_status;
    }   
    jobs_[job_id] = job;
}

Status JobManager::Suspend(const JobId jobid) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        return kJobNotFound;
    }
    Job* job = job_it->second;
    if (job->state_ != kJobNormal) {
        return kUnknown;
    }
    job->state_ = kJobSuspend;

    assert(suspend_pods_.find(jobid) == suspend_pods_.end());

    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    it = pending_pods_.find(jobid);
    if (it != deploy_pods_.end()) {
        std::map<PodId, PodStatus*>& job_suspend_pods = suspend_pods_[jobid];
        std::map<PodId, PodStatus*>& job_pending_pods = it->second;
        std::map<PodId, PodStatus*>::iterator pod_it;
        for (pod_it = job_pending_pods.begin(); pod_it != job_pending_pods.end(); ++pod_it) {
            PodStatus* pod = pod_it->second;
            SuspendPod(pod);
            job_suspend_pods[pod->podid()] = pod;
        }
        pending_pods_.erase(it);
    }

    it = deploy_pods_.find(jobid);
    if (it != deploy_pods_.end()) {
        std::map<PodId, PodStatus*>& job_suspend_pods = suspend_pods_[jobid];
        std::map<PodId, PodStatus*>& job_deploy_pods = it->second;
        std::map<PodId, PodStatus*>::iterator pod_it;
        for (pod_it = job_deploy_pods.begin(); pod_it != job_deploy_pods.end(); ++pod_it) {
            PodStatus* pod = pod_it->second;
            SuspendPod(pod);
            job_suspend_pods[pod->podid()] = pod;
        }
        deploy_pods_.erase(it);
    }
    return kOk;
}

void JobManager::SuspendPod(PodStatus* pod) {
    mutex_.AssertHeld();
    PodState state = pod->state();
    if (state == kPodPending) {
        pod->set_state(kPodSuspend);
    } else if (state == kPodDeploy) {
        const std::string& hostname = pod->hostname();
        AgentInfo* agent = agents_[hostname];
        ReclaimResource(*pod, agent);
        pod->set_state(kPodSuspend);
        pod->set_hostname("");
    }
}

Status JobManager::Resume(const JobId jobid) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        return kJobNotFound;
    }
    Job* job = job_it->second;
    if (job->state_ != kJobSuspend) {
        return kUnknown;
    }
    job->state_ = kJobNormal;

    assert(pending_pods_.find(jobid) == pending_pods_.end());
    assert(deploy_pods_.find(jobid) == deploy_pods_.end());

    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    it = suspend_pods_.find(jobid);
    if (it != suspend_pods_.end()) {
        std::map<PodId, PodStatus*>& job_pending_pods = pending_pods_[jobid];
        std::map<PodId, PodStatus*>& job_suspend_pods = it->second;
        std::map<PodId, PodStatus*>::iterator pod_it;
        for (pod_it = job_suspend_pods.begin(); pod_it != job_suspend_pods.end(); ++pod_it) {
            PodStatus* pod = pod_it->second;
            ResumePod(pod);
            job_pending_pods[pod->podid()] = pod;
        }
        deploy_pods_.erase(it);
    }
    return kOk;
}

void JobManager::ResumePod(PodStatus* pod) {
    mutex_.AssertHeld();
    PodState state = pod->state();
    if (state == kPodSuspend) {
        pod->set_state(kPodPending);
    }
}

void JobManager::GetPendingPods(JobInfoList* pending_pods) {
    MutexLock lock(&mutex_);
    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    for (it = pending_pods_.begin(); it != pending_pods_.end(); ++it) {
        JobInfo* job_info = pending_pods->Add();
        JobId job_id = it->first;
        job_info->set_jobid(job_id);
        const JobDescriptor& job_desc = jobs_[job_id]->desc_;
        job_info->mutable_job()->CopyFrom(job_desc);

        const std::map<PodId, PodStatus*> & job_pending_pods = it->second;
        std::map<PodId, PodStatus*>::const_iterator jt;
        for (jt = job_pending_pods.begin(); jt != job_pending_pods.end(); ++jt) {
            PodStatus* pod_status = jt->second;
            PodStatus* new_pod_status = job_info->add_tasks();
            new_pod_status->CopyFrom(*pod_status);
        }
    }
}

Status JobManager::Propose(const ScheduleInfo& sche_info) {
    MutexLock lock(&mutex_);
    const std::string& jobid = sche_info.jobid();
    const std::string& podid = sche_info.jobid();
    const std::string& endpoint = sche_info.jobid();

    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    it = pending_pods_.find(jobid);
    if (it == pending_pods_.end()) {
        return kJobNotFound;
    }
    std::map<PodId, PodStatus*>& job_pending_pods = it->second;
    std::map<PodId, PodStatus*>::iterator jt = job_pending_pods.find(podid);
    if (jt == job_pending_pods.end()) {
        return kPodNotFound;
    }
    std::map<AgentAddr, AgentInfo*>::iterator at = agents_.find(endpoint);
    if (at == agents_.end()) {
        return kAgentNotFound;
    }

    PodStatus* pod = jt->second;
    AgentInfo* agent = at->second;
    Status feasible_status = AcquireResource(*pod, agent);
    if (feasible_status != kOk) {
        return feasible_status;
    }

    pod->set_hostname(sche_info.endpoint());
    pod->set_state(kPodDeploy);
    job_pending_pods.erase(jt);
    if (job_pending_pods.size() == 0) {
        pending_pods_.erase(it);
    }

    deploy_pods_[jobid][podid] = pod;
    return kOk;
}

Status JobManager::AcquireResource(const PodStatus& pod, AgentInfo* agent) {
    mutex_.AssertHeld();
    Resource pod_requirement;
    GetPodRequirement(pod, &pod_requirement);
    const Resource& unassigned = agent->unassigned();
    if (!MasterUtil::FitResource(pod_requirement, unassigned)) {
        return kQuota;
    }
    MasterUtil::SubstractResource(pod_requirement, agent->mutable_unassigned());
    MasterUtil::AddResource(pod_requirement, agent->mutable_assigned());
    return kOk;
}

void JobManager::ReclaimResource(const PodStatus& pod, AgentInfo* agent) {
    mutex_.AssertHeld();
    Resource pod_requirement;
    GetPodRequirement(pod, &pod_requirement);
    MasterUtil::SubstractResource(pod_requirement, agent->mutable_assigned());
    MasterUtil::AddResource(pod_requirement, agent->mutable_unassigned());
}

void JobManager::GetPodRequirement(const PodStatus& pod, Resource* requirement) {
    Job* job = jobs_[pod.jobid()];
    const PodDescriptor& pod_desc = job->desc_.pod();
    CalculatePodRequirement(pod_desc, requirement);
}

void JobManager::CalculatePodRequirement(const PodDescriptor& pod_desc,
                                         Resource* pod_requirement) {
    for (int32_t i = 0; i < pod_desc.tasks_size(); i++) {
        const TaskDescriptor& task_desc = pod_desc.tasks(i);
        const Resource& task_requirement = task_desc.requirement();
        pod_requirement->set_millicores(pod_requirement->millicores() + task_requirement.millicores());
        pod_requirement->set_memory(pod_requirement->memory() + task_requirement.memory());
        for (int32_t j = 0; j < task_requirement.ports_size(); j++) {
            pod_requirement->add_ports(task_requirement.ports(j));
        }
    }
}

void JobManager::KeepAlive(const std::string& agent_addr) {
    MutexLock lock(&mutex_timer_);
    int64_t timer_id;
    if (agent_timer_.find(agent_addr) != agent_timer_.end()) {
        timer_id = agent_timer_[agent_addr];
        bool cancel_ok = pool_.CancelTask(timer_id);    
        if (!cancel_ok) {
            return;
        }
    }
    timer_id = pool_.DelayTask(FLAGS_agent_timeout,
                               boost::bind(&JobManager::HandleAgentOffline,
                                           this,
                                           agent_addr));
    agent_timer_[agent_addr] = timer_id;
}

void JobManager::HandleAgentOffline(const std::string agent_addr) {
    MutexLock lock(&mutex_);
    if (agents_.find(agent_addr) == agents_.end()) {
        return;
    }
    AgentInfo* agent_info = agents_[agent_addr];
    for (int i = 0; i < agent_info->pods_size(); i++) {
        PodStatus* pod_status = agent_info->mutable_pods(i);
        ReschedulePod(pod_status);
        const JobId& job_id = pod_status->jobid();
        const PodId& pod_id = pod_status->podid();    
        pending_pods_[job_id][pod_id] = pod_status;
    }
    delete agent_info;
    agents_.erase(agent_addr);
}

void JobManager::ReschedulePod(PodStatus* pod_status) {
    assert(pod_status);
    assert(pod_status->state() == kPodRunning);
    mutex_.AssertHeld();
    pod_status->set_state(kPodPending);
    pod_status->set_hostname("");
    pod_status->mutable_resource_used()->Clear();
    for (int i = 0; i < pod_status->status_size(); i++) {
        pod_status->mutable_status(i)->Clear();
    }
}

}
}
