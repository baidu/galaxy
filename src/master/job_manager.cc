// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "job_manager.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <gflags/gflags.h>
#include "proto/agent.pb.h"
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "master_util.h"
#include <logging.h>

DECLARE_int32(master_agent_timeout);
DECLARE_int32(master_agent_rpc_timeout);
DECLARE_int32(master_query_period);

namespace baidu {
namespace galaxy {

JobManager::JobManager()
    : on_query_num_(0) {
    ScheduleNextQuery();
}

JobManager::~JobManager() {
}

void JobManager::Add(const JobId& job_id, const JobDescriptor& job_desc) {
    Job* job = new Job();
    job->state_ = kJobNormal;
    job->desc_.CopyFrom(job_desc);
    MutexLock lock(&mutex_);
    for(int i = 0; i < job_desc.replica(); i++) {
        PodId pod_id = MasterUtil::GeneratePodId(job_desc);
        PodStatus* pod_status = new PodStatus();
        pod_status->set_podid(pod_id);
        pod_status->set_jobid(job_id);
        job->pods_[pod_id] = pod_status;
        pending_pods_[job_id][pod_id] = pod_status;
    }
    jobs_[job_id] = job;
    LOG(INFO, "job[%s] submitted by user: %s, ", job_id.c_str(), job_desc.user().c_str());
}

void JobManager::ReloadJobInfo(const JobInfo& job_info) {
    Job* job = new Job();
    job->state_ = job_info.state();
    job->desc_.CopyFrom(job_info.desc());
    std::string job_id = job_info.jobid();
    MutexLock lock(&mutex_);
    jobs_[job_id] = job;
}

Status JobManager::Suspend(const JobId& jobid) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        LOG(INFO, "suspend failed. no such job :%s", jobid.c_str());
        return kJobNotFound;
    }
    Job* job = job_it->second;
    if (job->state_ != kJobNormal) {
        LOG(INFO, "suspend failed. job is not running: %s", jobid.c_str());
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
    LOG(INFO, "job suspended: %s", jobid.c_str());
    return kOk;
}

void JobManager::SuspendPod(PodStatus* pod) {
    mutex_.AssertHeld();
    PodState state = pod->state();
    if (state == kPodPending) {
        pod->set_state(kPodSuspend);
    } else if (state == kPodDeploy) {
        const std::string& endpoint = pod->endpoint();
        AgentInfo* agent = agents_[endpoint];
        ReclaimResource(*pod, agent);
        pod->set_state(kPodSuspend);
        pod->set_endpoint("");
    }
    LOG(INFO, "pod suspended: %s", pod->podid().c_str());
}

Status JobManager::Resume(const JobId& jobid) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.find(jobid);
    if (job_it == jobs_.end()) {
        LOG(INFO, "resume failed, no such job: %s", jobid.c_str());
        return kJobNotFound;
    }
    Job* job = job_it->second;
    if (job->state_ != kJobSuspend) {
        LOG(INFO, "resume failed, no such job: %s", jobid.c_str());
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
    LOG(INFO, "job resumed: %s", jobid.c_str());
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
        job_info->mutable_desc()->CopyFrom(job_desc);

        const std::map<PodId, PodStatus*> & job_pending_pods = it->second;
        std::map<PodId, PodStatus*>::const_iterator jt;
        for (jt = job_pending_pods.begin(); jt != job_pending_pods.end(); ++jt) {
            PodStatus* pod_status = jt->second;
            PodStatus* new_pod_status = job_info->add_pods();
            new_pod_status->CopyFrom(*pod_status);
        }
    }
}

Status JobManager::Propose(const ScheduleInfo& sche_info) {
    MutexLock lock(&mutex_);
    const std::string& jobid = sche_info.jobid();
    const std::string& podid = sche_info.podid();
    const std::string& endpoint = sche_info.endpoint();

    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    it = pending_pods_.find(jobid);
    if (it == pending_pods_.end()) {
        LOG(INFO, "propose fail, no such job: %s", jobid.c_str());
        return kJobNotFound;
    }
    std::map<PodId, PodStatus*>& job_pending_pods = it->second;
    std::map<PodId, PodStatus*>::iterator jt = job_pending_pods.find(podid);
    if (jt == job_pending_pods.end()) {
        LOG(INFO, "propse fail, no such pod: %s", podid.c_str());
        return kPodNotFound;
    }
    std::map<AgentAddr, AgentInfo*>::iterator at = agents_.find(endpoint);
    if (at == agents_.end()) {
        LOG(INFO, "propose fail, no such agent: %s", endpoint.c_str());
        return kAgentNotFound;
    }

    PodStatus* pod = jt->second;
    AgentInfo* agent = at->second;
    Status feasible_status = AcquireResource(*pod, agent);
    if (feasible_status != kOk) {
        LOG(INFO, "propose fail, no resource, error code:[%d]", feasible_status);
        return feasible_status;
    }

    pod->set_endpoint(sche_info.endpoint());
    pod->set_state(kPodDeploy);
    job_pending_pods.erase(jt);
    if (job_pending_pods.size() == 0) {
        pending_pods_.erase(it);
    }

    deploy_pods_[jobid][podid] = pod;
    LOG(INFO, "propose success, %s will be run on %s",
        podid.c_str(), endpoint.c_str());
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
    if (agent_addr == "") {
        LOG(WARNING, "ignore heartbeat with empty endpoint");
        return;
    }
    {
        MutexLock lock(&mutex_);
        if (agents_.find(agent_addr) == agents_.end()) {
            LOG(INFO, "new agent added: %s", agent_addr.c_str());
            agents_[agent_addr] = new AgentInfo();
        }
        AgentInfo* agent = agents_[agent_addr];
        agent->set_state(kAlive);
        agent->set_endpoint(agent_addr);
    }

    MutexLock lock(&mutex_timer_);
    LOG(DEBUG, "receive heartbeat from %s", agent_addr.c_str());
    int64_t timer_id;
    if (agent_timer_.find(agent_addr) != agent_timer_.end()) {
        timer_id = agent_timer_[agent_addr];
        bool cancel_ok = death_checker_.CancelTask(timer_id);
        if (!cancel_ok) {
            LOG(WARNING, "agent is offline, agent: %s", agent_addr.c_str());
        }
    }
    timer_id = death_checker_.DelayTask(FLAGS_master_agent_timeout,
                                        boost::bind(&JobManager::HandleAgentOffline,
                                                    this,
                                                    agent_addr));
    agent_timer_[agent_addr] = timer_id;
}

void JobManager::HandleAgentOffline(const std::string agent_addr) {
    LOG(WARNING, "agent is offline: %s", agent_addr.c_str());
    MutexLock lock(&mutex_);
    if (agents_.find(agent_addr) == agents_.end()) {
        LOG(INFO, "no such agent %s", agent_addr.c_str());
        return;
    }

    AgentInfo* agent_info = agents_[agent_addr];
//    for (int i = 0; i < agent_info->pods_size(); i++) {
//        PodStatus* pod_status = agent_info->mutable_pods(i);
//        ReschedulePod(pod_status);
//    }
    PodMap& agent_pods = running_pods_[agent_addr];
    PodMap::iterator it = agent_pods.begin();
    for (; it != agent_pods.end(); ++it) {
        std::map<PodId, PodStatus*>& job_agent_pods = it->second;
        std::map<PodId, PodStatus*>::iterator jt = job_agent_pods.begin();
        for (; jt != job_agent_pods.end(); ++jt) {
            PodStatus* pod = jt->second;
            ReschedulePod(pod);
        }
    }

    running_pods_.erase(agent_addr);
    agent_info->set_state(kDead);
    LOG(INFO, "agent is dead: %s", agent_addr.c_str());
}

void JobManager::ReschedulePod(PodStatus* pod_status) {
    assert(pod_status);
    assert(pod_status->state() == kPodRunning);
    mutex_.AssertHeld();

    pod_status->set_state(kPodPending);
    pod_status->set_endpoint("");
    pod_status->mutable_resource_used()->Clear();
    for (int i = 0; i < pod_status->status_size(); i++) {
        pod_status->mutable_status(i)->Clear();
    }

    const JobId& job_id = pod_status->jobid();
    const PodId& pod_id = pod_status->podid();
    pending_pods_[job_id][pod_id] = pod_status;
    LOG(INFO, "pod state rescheuled to pending, pod id:%s", pod_id.c_str());
}

void JobManager::DeployPod() {
    MutexLock lock(&mutex_);
    std::map<JobId, std::map<PodId, PodStatus*> >::iterator it;
    for (it = deploy_pods_.begin(); it != deploy_pods_.end(); ++it) {
        const JobId& jobid = it->first;
        std::map<PodId, PodStatus*>& job_deploy_pods = it->second;
        Job* job = jobs_[jobid];
        const PodDescriptor& pod_desc = job->desc_.pod();
        std::map<PodId, PodStatus*>::iterator jt;
        for (jt = job_deploy_pods.begin(); jt != job_deploy_pods.end(); ++jt) {
            const PodId& podid = jt->first;
            PodStatus* pod = jt->second;
            const std::string& endpoint = pod->endpoint();
            pod->set_state(kPodRunning);

            // TODO:: check agent health
            // AgentInfo* agent = agents_[endpoint];

            running_pods_[endpoint][jobid][podid] = pod;
            RunPod(pod_desc, pod);
        }
    }
}

void JobManager::RunPod(const PodDescriptor& desc, PodStatus* pod) {
    mutex_.AssertHeld();
    RunPodRequest* request = new RunPodRequest;
    RunPodResponse* response = new RunPodResponse;
    request->set_podid(pod->podid());
    request->mutable_pod()->CopyFrom(desc);

    Agent_Stub* stub;
    const AgentAddr& endpoint = pod->endpoint();
    rpc_client_.GetStub(endpoint, &stub);
    boost::function<void (const RunPodRequest*, RunPodResponse*, bool, int)> run_pod_callback;
    run_pod_callback = boost::bind(&JobManager::RunPodCallback, this, pod, endpoint,
                                   _1, _2, _3, _4);
    rpc_client_.AsyncRequest(stub, &Agent_Stub::RunPod, request, response,
                             run_pod_callback, FLAGS_master_agent_rpc_timeout, 0);
    delete stub;
}

void JobManager::RunPodCallback(PodStatus* pod, AgentAddr endpoint,
                                const RunPodRequest* request,
                                RunPodResponse* response,
                                bool failed, int error) {
    boost::scoped_ptr<const RunPodRequest> request_ptr(request);
    boost::scoped_ptr<RunPodResponse> response_ptr(response);
    MutexLock lock(&mutex_);
    const std::string& jobid = pod->jobid();
    const std::string& podid = pod->podid();
    if (pod->state() != kPodRunning || pod->endpoint() != endpoint) {
        LOG(INFO, "ignore run pod callback of pod [%s %s] on [%s]",
            jobid.c_str(), podid.c_str(), endpoint.c_str());
        return;
    }

    Status status = response->status();
    if (failed || status != kOk) {
        LOG(INFO, "run pod [%s %s] on [%s] fail: %d", jobid.c_str(),
            podid.c_str(), endpoint.c_str(), status);
        assert(pod == running_pods_[endpoint][jobid][podid]);
        running_pods_[endpoint][jobid].erase(podid);
        if (running_pods_[endpoint][jobid].size() == 0) {
            running_pods_[endpoint].erase(jobid);
            if (running_pods_[endpoint].size() == 0) {
                running_pods_.erase(endpoint);
            }
        }
        ReschedulePod(pod);
        return;
    }
    LOG(INFO, "run pod [%s %s] on [%s] success", jobid.c_str(),
        podid.c_str(), endpoint.c_str());
}

void JobManager::ScheduleNextQuery() {
    thread_pool_.DelayTask(FLAGS_master_query_period, boost::bind(&JobManager::Query, this));
}

void JobManager::Query() {
    MutexLock lock(&mutex_);
    assert(on_query_num_ == 0);
    std::map<AgentAddr, AgentInfo*>::iterator it;
    for (it = agents_.begin(); it != agents_.end(); ++it) {
        AgentInfo* agent = it->second;
        QueryAgent(agent);
    }
    LOG(INFO, "query %lld agents", on_query_num_);
    if (on_query_num_ == 0) {
        ScheduleNextQuery();
    }
}

void JobManager::QueryAgent(AgentInfo* agent) {
    const AgentAddr& endpoint = agent->endpoint();
    if (agent->state() != kAlive) {
        LOG(DEBUG, "ignore dead agent [%s]", endpoint.c_str());
        return;
    }

    QueryRequest* request = new QueryRequest;
    QueryResponse* response = new QueryResponse;

    Agent_Stub* stub;
    rpc_client_.GetStub(endpoint, &stub);
    boost::function<void (const QueryRequest*, QueryResponse*, bool, int)> query_callback;
    query_callback = boost::bind(&JobManager::QueryAgentCallback, this,
                                 endpoint, _1, _2, _3, _4);

    LOG(INFO, "query agent [%s]", endpoint.c_str());
    rpc_client_.AsyncRequest(stub, &Agent_Stub::Query, request, response,
                             query_callback, FLAGS_master_agent_rpc_timeout, 0);
    delete stub;
    on_query_num_++;
}

void JobManager::QueryAgentCallback(AgentAddr endpoint, const QueryRequest* request,
                                    QueryResponse* response, bool failed, int error) {
    boost::scoped_ptr<const QueryRequest> request_ptr(request);
    boost::scoped_ptr<QueryResponse> response_ptr(response);
    MutexLock lock(&mutex_);
    if (--on_query_num_ == 0) {
        ScheduleNextQuery();
    }

    std::map<AgentAddr, AgentInfo*>::iterator it = agents_.find(endpoint);
    if (it == agents_.end()) {
        LOG(FATAL, "query agent [%s] not exist", endpoint.c_str());
        return;
    }
    Status status = response->status();
    if (failed || status != kOk) {
        LOG(INFO, "query agent [%s] fail: %d", endpoint.c_str(), status);
        return;
    }
    LOG(INFO, "query agent [%s] success", endpoint.c_str());

    AgentInfo* agent = it->second;
    const AgentInfo& report_agent_info = response->agent();
    agent->CopyFrom(report_agent_info);

    PodMap agent_running_pods = running_pods_[endpoint]; // this is a copy
    for (int32_t i = 0; i < report_agent_info.pods_size(); i++) {
        const PodStatus& report_pod_info = report_agent_info.pods(i);
        const JobId& jobid = report_pod_info.jobid();
        const PodId& podid = report_pod_info.podid();
        if (agent_running_pods.find(jobid) == agent_running_pods.end()) {
            LOG(WARNING, "report non-exist pod [%s %s]", jobid.c_str(), podid.c_str());
            continue;
        }
        if (agent_running_pods[jobid].find(podid) == agent_running_pods[jobid].end()) {
            LOG(WARNING, "report non-exist pod [%s %s]", jobid.c_str(), podid.c_str());
            continue;
        }
        PodStatus* pod = agent_running_pods[jobid][podid];
        // only copy dynamic information
        pod->mutable_status()->CopyFrom(report_pod_info.status());
        pod->mutable_resource_used()->CopyFrom(report_pod_info.resource_used());
        LOG(DEBUG, "update pod [%s %s]", jobid.c_str(), podid.c_str());

        agent_running_pods[jobid].erase(podid);
        if (agent_running_pods[jobid].size() == 0) {
            agent_running_pods.erase(jobid);
        }
    }

    // reschedule un-report pods
    PodMap::iterator pod_it = agent_running_pods.begin();
    for (; pod_it != agent_running_pods.end(); ++pod_it) {
        const JobId& jobid = pod_it->first;
        std::map<PodId, PodStatus*>& pods = pod_it->second;
        std::map<PodId, PodStatus*>::iterator pod_it = pods.begin();
        for (; pod_it != pods.end(); ++pod_it) {
            const PodId& podid = pod_it->first;
            LOG(WARNING, "dead pod [%s %s]", jobid.c_str(), podid.c_str());
            PodStatus* pod = pod_it->second;
            running_pods_[endpoint][jobid].erase(podid);
            ReschedulePod(pod);
        }
    }
}

void JobManager::GetAgentsInfo(AgentInfoList* agents_info) {
    MutexLock lock(&mutex_);
    std::map<AgentAddr, AgentInfo*>::iterator it;
    for (it = agents_.begin(); it != agents_.end(); ++it) {
        AgentInfo* agent = it->second;
        agents_info->Add()->CopyFrom(*agent);
    }
}

void JobManager::GetAliveAgentsInfo(AgentInfoList* agents_info) {
    MutexLock lock(&mutex_);
    std::map<AgentAddr, AgentInfo*>::iterator it;
    for (it = agents_.begin(); it != agents_.end(); ++it) {
        AgentInfo* agent = it->second;
        if (agent->state() != kAlive) {
            continue;
        }
        agents_info->Add()->CopyFrom(*agent);
    }
}

void JobManager::GetJobsOverview(JobOverviewList* jobs_overview) {
    MutexLock lock(&mutex_);
    std::map<JobId, Job*>::iterator job_it = jobs_.begin();
    for (; job_it != jobs_.end(); ++job_it) {
        const JobId& jobid = job_it->first;
        Job* job = job_it->second;
        JobOverview* overview = jobs_overview->Add();
        overview->mutable_desc()->CopyFrom(job->desc_);
        overview->set_jobid(jobid);
        overview->set_state(job->state_);

        uint32_t running_num = 0;
        std::map<PodId, PodStatus*>& pods = job->pods_;
        std::map<PodId, PodStatus*>::iterator pod_it = pods.begin();
        for (; pod_it != pods.end(); ++pod_it) {
            // const PodId& podid = pod_it->first;
            const PodStatus* pod = pod_it->second;
            if (pod->state() == kPodRunning) {
                running_num++;
                MasterUtil::AddResource(pod->resource_used(), overview->mutable_resource_used());
            }
        }
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
    job_info->mutable_desc()->CopyFrom(job->desc_);
    std::map<PodId, PodStatus*>::iterator pod_it = job->pods_.begin();
    for (; pod_it != job->pods_.end(); ++pod_it) {
        PodStatus* pod = pod_it->second;
        job_info->add_pods()->CopyFrom(*pod);
    }
    return kOk;
}

}
}
