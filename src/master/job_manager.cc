// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "job_manager.h"

#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "master_util.h"

namespace baidu {
namespace galaxy {

void JobManager::Add(const JobDescriptor& job_desc) {
    Job* job = new Job();
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
    Status feasible_status = CheckScheduleFeasible(pod, agent);
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

Status JobManager::CheckScheduleFeasible(const PodStatus* pod, const AgentInfo* agent) {
    mutex_.AssertHeld();
    Job* job = jobs_[pod->jobid()];
    Resource pod_requirement;
    const PodDescriptor& pod_desc = job->desc_.pod();
    for (int32_t i = 0; i < pod_desc.tasks_size(); i++) {
        const TaskDescriptor& task_desc = pod_desc.tasks(i);
        const Resource& task_requirement = task_desc.requirement();
        pod_requirement.set_millicores(pod_requirement.millicores() + task_requirement.millicores());
        pod_requirement.set_memory(pod_requirement.memory() + task_requirement.memory());
        for (int32_t j = 0; j < task_requirement.ports_size(); j++) {
            pod_requirement.add_ports(task_requirement.ports(j));
        }
    }
    const Resource& unassigned = agent->unassigned();
    if (unassigned.millicores() < pod_requirement.millicores()) {
        return kQuota;
    }
    if (unassigned.memory() < pod_requirement.memory()) {
        return kQuota;
    }
    // TODO: check port & disk & ssd
    return kOk;
}

}
}
