// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scheduler/scheduler.h"

#include <math.h>
#include <algorithm>
#include <gflags/gflags.h>
#include "logging.h"
#include "utils/resource_utils.h"

DECLARE_int32(scheduler_feasibility_factor);
DECLARE_double(scheduler_cpu_used_factor);
DECLARE_double(scheduler_mem_used_factor);
DECLARE_double(scheduler_prod_count_factor);
DECLARE_double(scheduler_cpu_overload_threashold);
DECLARE_int32(scheduler_agent_overload_turns_threashold);

namespace baidu {
namespace galaxy {

double Scheduler::CalcLoad(const AgentInfo& agent) {
    double cpu_load = agent.used().millicores() * FLAGS_scheduler_cpu_used_factor /
            agent.total().millicores();
    double mem_load = agent.used().memory() * FLAGS_scheduler_mem_used_factor /
            agent.total().memory();
    double prod_load = agent.pods_size() / FLAGS_scheduler_prod_count_factor;
    return exp(cpu_load) + exp(mem_load) + exp(prod_load);
}

// @brief 按照priority降序排序
static bool JobCompare(const JobInfo* left, const JobInfo* right) {
    return left->desc().priority() > right->desc().priority();
}


static bool ScoreCompare(const std::pair<double, std::string>& left,
                         const std::pair<double, std::string>& right) {
    return left.first > right.first;
}

static bool AgentCompare(const std::pair<double, AgentInfo*>& left,
                         const std::pair<double, AgentInfo*>& right) {
    return left.first < right.first;
}

void Scheduler::Propose(const PodScaleUpCell& cell,
                        const std::string& master_addr) {
    cell.Score();
    std::vector<ScheduleInfo> propose_pods;
    cell.Propose(&propose_pods);
    MasterStub* master_stub = NULL;
    bool ok = rpc_client_.GetStub(master_addr, &master_stub);
    if (!ok) {
        LOG(WARNING, "fail to prepare master stub");
        return;
    }
    ProposeRequest* request = new ProposeRequest();
    ProposeResponse* response = new ProposeResponse();
    for (std::vector<ScheduleInfo>::iterator it = propose.begin();
         it != propose.end(); ++it) {
        ScheduleInfo* sched = pro_request.add_schedule();
        sched->CopyFrom(*it);
    }
    rpc_client_.AsyncRequest(master_stub,
                            &Master_Stub::Propose,
                            request,
                            response,
                            5, 1);
}

void Scheduler::ProposeCallBack(const ProposeRequest* request,
                                ProposeResponse* response,
                                bool failed, int) {

}

int32_t Scheduler::ScheduleScaleUp(const std::string master_addr,
                                   std::vector<JobInfo>& pending_jobs) {
    MutexLock lock(&sched_mutex_);
    std::vector<PodScaleUpCell> pending_pods;
    // order job by priority
    int32_t all_pod_needs = ChoosePods(pending_jobs, &pending_pods);
    // shuffle agent
    boost::unordered_map<std::string, AgentInfo>::iterator it = resources_->begin();
    std::vector<std::string> keys;
    for (; it != resources_.end(); ++it) {
        keys.push_back(it->first);
    }
    Shuffle(keys);
    // 计算feasibility
    std::vector<std::string>::iterator key_it = keys.begin();
    int cur_pod_count = 0;
    for (; key_it != keys.begin() &&
         cur_pod_count < all_pod_needs; ++key_it) {
        AgentInfo agent = resources_->find(*key_it).second;
        for (std::vector<PodScaleUpCell>::iterator pod_it = pending_pods.begin();
                pod_it != pending_pods.end(); ++pod_it) {
            LOG(INFO, "checking: %s on %s",
                    pod_it->job.jobid().c_str(),
                    agent.endpoint().c_str());
            if (pod_it->feasible.size() >= pod_it->feasible_limit ) {
                LOG(INFO, "feasibility checking done for %s",
                        pod_it->job.jobid().c_str());
                continue;
            } else {
                bool check = (*pod_it)->FeasibilityCheck(*res_it);
                LOG(INFO, "feasibility checking %s on %s return %d",
                        (*pod_it)->job->jobid().c_str(),
                         (*res_it)->agent_info->endpoint().c_str(), (int)check);
                if (check == true) {
                    (*pod_it)->feasible.push_back(*res_it);
                    cur_feasible_count++;
                }
                // 此处不break，说明一个Agent尽量调度多的Pod
                // TODO 此处需要再斟酌一下，对于新添入机器会出现负载较高的情况
                // 但是如果直接break的话，对于符合某个pod的所有机器对其他pod不可见的问题
                // 同样需要考虑一下
            }
        }
    }
    LOG(INFO, " PodScaleUpCell count %u", pending_pods.size());
    // 对增加实例任务进行优先级计算
    for (std::vector<PodScaleUpCell*>::iterator pod_it = pending_pods.begin();
            pod_it != pending_pods.end(); ++pod_it) {
        (*pod_it)->Score();
        uint32_t count = (*pod_it)->Propose(propose);
        propose_count += count;
        LOG(INFO, "propose jobid %s count %u", (*pod_it)->job->jobid().c_str(), count);
    }

    // 销毁PodScaleUpCell
    for (std::vector<PodScaleUpCell*>::iterator pod_it = pending_pods.begin();
            pod_it != pending_pods.end(); ++pod_it) {
        delete *pod_it;
    }
    return propose_count;
}

bool Scheduler::CalcPreemptStep(AgentInfoExtend* agent,
                               PodScaleUpCell* pods) {
    return false;
}

int32_t Scheduler::ScheduleScaleDown(std::vector<JobInfo*>& reducing_jobs,
                                     std::vector<ScheduleInfo*>* propose) {
    int propose_count = 0;
    // 计算job优先级，及其需要调度的pod数量
    std::vector<PodScaleDownCell*> reducing_pods;

    int32_t total_reducing_count = ChooseReducingPod(reducing_jobs, &reducing_pods);

    if (total_reducing_count < 0) {
        return 0;
    }

    // 对减少实例任务进行优先级计算
    for (std::vector<PodScaleDownCell*>::iterator pod_it = reducing_pods.begin();
            pod_it != reducing_pods.end(); ++pod_it) {
        (*pod_it)->Score();
        propose_count += (*pod_it)->Propose(propose);
    }

    // 销毁PodScaleDownCell
    for (std::vector<PodScaleDownCell*>::iterator pod_it = reducing_pods.begin();
            pod_it != reducing_pods.end(); ++pod_it) {
        delete *pod_it;
    }
    return propose_count;
}

int32_t Scheduler::SyncResources(const GetResourceSnapshotResponse* response) {
    MutexLock lock(&sched_mutex_);
    LOG(INFO, "sync resource from master, update agent_info count %d, deleted count %d", 
             response->agents_size(),
             response->deleted_agents_size());
    for (int32_t i = 0; i < response->deleted_agents_size(); i++) {
        boost::unordered_map<std::string, AgentInfo>::iterator it =
             resources_->find(response->deleted_agents(i));
        if (it != resources_->end()) {
            resources_->erase(it->first);
        }
    }

    for (int32_t i =0 ; i < response->agents_size(); i++) {
        const AgentInfo& agent = response->agents(i);
        boost::unordered_map<std::string, AgentInfo>::iterator it = resources_.find(agent.endpoint());
        if (it == resources_->end()) {
            resources_->insert(std::make_pair(agent.endpoint(), agent));
        }else {
            it->second.CopyFrom(agent);
        }
        LOG(INFO, "sync agent %s with version %d stat:mem total %ld, cpu total %d, mem assigned %ld, cpu assinged %d",
              agent.endpoint().c_str(), agent.version(),
              agent.total().memory(), agent.total().millicores(),
              agent.assigned().memory(), agent.assigned().millicores());
    }
    return resources_->size();
}

int32_t Scheduler::ChoosePods(const std::vector<JobInfo>& pending_jobs,
                              std::vector<PodScaleUpCell>* pending_pods) {

    // 按照Job优先级进行排序
    std::sort(pending_jobs.begin(), pending_jobs.end(), JobCompare);
    std::vector<JobInfo>::const_iterator job_it = pending_jobs.begin();
    int32_t all_pod_needs = 0;
    for (; job_it != pending_jobs.end(); ++job_it) {
        std::vector<std::string> job_pending_pods;
        uint32_t deploying_num = 0;
        for (int i = 0; i < job_it->pods_size(); i++) {
            // use pod state 
            if (job_it->pods(i).state() == kPodDeploying) {
                deploying_num ++;
                continue;
            }
            if (job_it->pods(i).stage() == kStagePending) {
                job_pending_pods.push_back(job_it->pods(i).podid());
            }
        }
        uint32_t max_deploy_pods = job_pending_pods.size();
        if (job_it->desc().deploy_step() > 0 && 
            (uint32_t)job_it->desc().deploy_step() < max_deploy_pods) { 
            max_deploy_pods = job_it->desc().deploy_step();
        }
        LOG(INFO, "job %s deploy stat: pending_size %u, deploying_num %u, max_deploy_pods %u",
          job_it->jobid().c_str(), job_pending_pods.size(), deploying_num, max_deploy_pods);
        if (job_pending_pods.size() <= 0 
           || deploying_num >= max_deploy_pods) {
            continue;
        }
        std::vector<std::string> need_schedule;
        for (uint32_t i = 0; i < job_pending_pods.size() && deploying_num < max_deploy_pods; i++) {
            need_schedule.push_back(job_pending_pods[i]);
            deploying_num++;
        }
        PodDescriptor pod_desc;
        bool find_desc = false;
        for (int i = 0; i < job->pod_descs_size(); ++i) {
            if (job_it->pod_descs(i).version() != job_it->latest_version()) {
                continue;
            }
            pod_desc.CopyFrom(job_it->pod_descs(i));
            find_desc = true;
            break;
        }
        if (!find_desc) {
            LOG(WARNING, "fail to get job %s latest version %s pod descripto",
              job_it->jobid().c_str(), job_it->latest_version().c_str());
            continue;
        }
        PodScaleUpCell need_schedule_cell;
        need_schedule_cell.pod = pod_desc;
        need_schedule_cell.job = *job_it;
        need_schedule_cell.pod_ids = need_schedule;
        need_schedule_cell.feasible_limit = need_schedule.size() * FLAGS_scheduler_feasibility_factor;
        all_pod_needs += need_schedule_cell->feasible_limit;
        pending_pods->push_back(need_schedule_cell); 
        LOG(INFO, "scheduler choosed %u pod to deploy for job %s" , need_schedule.size(), job_it->jobid().c_str());
    }
    return all_pod_needs;
}

int32_t Scheduler::ChooseReducingPod(std::vector<JobInfo*>& reducing_jobs,
                                     std::vector<PodScaleDownCell*>* reducing_pods) {
    // 获取scale down的任务
    int reducing_count = 0;
    std::vector<JobInfo*>::iterator job_it = reducing_jobs.begin();
    for (; job_it != reducing_jobs.end(); ++job_it) {
        PodScaleDownCell* cell = new PodScaleDownCell();
        cell->pod = (*job_it)->mutable_desc()->mutable_pod();
        cell->job = *job_it;
        for (int i = 0; i < (*job_it)->pods_size(); ++i) {
            boost::unordered_map<std::string, AgentInfoExtend*>::iterator agt_it =
                    resources_.find((*job_it)->pods(i).endpoint());
            if (agt_it == resources_.end()) {
                LOG(INFO, "scale down pod %s dose not belong to agent %s",
                        (*job_it)->pods(i).podid().c_str(),
                        (*job_it)->pods(i).endpoint().c_str());
            }
            else {
                LOG(INFO, "scale down pod %s  agent %s",
                        (*job_it)->pods(i).podid().c_str(),
                        (*job_it)->pods(i).endpoint().c_str());
                cell->pod_agent_map.insert(
                        std::make_pair((*job_it)->pods(i).podid(), agt_it->second->agent_info));
            }
        }
        // 计算需要scale_down数目
        int32_t scale_down_count = (*job_it)->pods_size() - cell->job->desc().replica();
        cell->scale_down_count = scale_down_count;
        reducing_count += scale_down_count;
        LOG(INFO, "job %s scale down count %d with pod_size %d replica %d", (*job_it)->jobid().c_str(), scale_down_count, (*job_it)->pods_size(), cell->job->desc().replica());
        reducing_pods->push_back(cell);
    }
    return reducing_count;
}

void Scheduler::BuildSyncRequest(GetResourceSnapshotRequest* request) {
    MutexLock lock(&sched_mutex_);
    boost::unordered_map<std::string, AgentInfoExtend*>::iterator it =
      resources_.begin();
    for (; it != resources_.end(); ++it) {
        DiffVersion* version = request->add_versions();
        version->set_version(it->second->agent_info->version());
        version->set_endpoint(it->second->agent_info->endpoint());
    }
}

PodScaleUpCell::PodScaleUpCell():pod(NULL), job(NULL),
        schedule_count(0), feasible_limit(0) {}

bool PodScaleUpCell::FeasibilityCheck(const AgentInfoExtend& agent_info_extend) {

    // check label 
    // TODO for simple only check first label   
    if (pod.labels_size() > 0) {
        std::string label = pod.labels(0);
        if (!label.empty() && agent_info_extend.labels_set.find(label) 
                                            == agent_info_extend.labels_set.end()) {
            LOG(INFO, "agent %s with version %d does not fit job %s label check %s", 
                      agent_info_extend.agent_info.endpoint().c_str(),
                      agent_info_extend.agent_info.version(),
                      job.jobid().c_str(),
                      label.c_str());
            return false;
        }
    }
    std::vector<Resource> alloc_set;
    bool ok = ResourceUtils::AllocResource(pod.requirement(),
                                           alloc_set,
                                           &agent_info_extend.agent_info);

    return ok; 
}

void PodScaleUpCell::Score() {
    std::vector<AgentInfo>::iterator agt_it = feasible.begin();
    for(; agt_it != feasible.end(); ++agt_it) {
        const AgentInfo& agent = *agt_it;
        double score = ScoreAgent(agent, pod);
        sorted.push_back(std::make_pair(score, agent.endpoint());
    }
    std::sort(sorted.begin(), sorted.end(), AgentCompare);
}

int32_t PodScaleUpCell::Propose(std::vector<ScheduleInfo>* propose) {
    int propose_count = 0;
    std::vector<std::pair<double, std::string> >::iterator sorted_it = sorted.begin();
    for (size_t i = 0; i < pod_ids.size(); ++i) {
        if (sorted_it == sorted.end()) {
            break;
        }
        else {
            ScheduleInfo sched;
            sched.set_endpoint(sorted_it->second);
            sched.set_podid(pod_ids[i]);
            sched.set_jobid(job.jobid());
            sched.set_action(kLaunch);
            propose->push_back(sched);
            LOG(DEBUG, "propose[%d] %s:%s on %s", propose_count,
                    sched->jobid().c_str(),
                    sched->podid().c_str(),
                    sched->endpoint().c_str());
            ++propose_count;
            ++sorted_it;
        }
    }
    return propose_count;
}

double PodScaleUpCell::ScoreAgent(const AgentInfo& agent_info,
                                  const PodDescriptor& /*desc*/) {
    double score = Scheduler::CalcLoad(agent_info);
    LOG(DEBUG, "score %s %lf", agent_info.endpoint().c_str(), score);
    return score;
}

PodScaleDownCell::PodScaleDownCell() : pod(NULL), job(NULL), scale_down_count(0) {}

int32_t PodScaleDownCell::Score() {
    std::map<std::string, AgentInfo*>::iterator pod_agt_it = pod_agent_map.begin();
    for(; pod_agt_it != pod_agent_map.end(); ++pod_agt_it) {
        double score = ScoreAgent(pod_agt_it->second, pod);
        sorted_pods.push_back(std::make_pair(score, pod_agt_it->first));
    }
    std::sort(sorted_pods.begin(), sorted_pods.end(), ScoreCompare);
    return 0;
}

double PodScaleDownCell::ScoreAgent(const AgentInfo* agent_info,
                   const PodDescriptor* /*desc*/) {
    // 计算机器当前使用率打分
    double score = Scheduler::CalcLoad(agent_info);
    LOG(DEBUG, "score %s %lf", agent_info->endpoint().c_str(), score);
    return -1 * score;
}

int32_t PodScaleDownCell::Propose(std::vector<ScheduleInfo*>* propose) {
    int propose_count = 0;
    std::map<std::string, AgentInfo*>::iterator pod_agent_it;
    std::vector<std::pair<double, std::string> >::iterator sorted_it = sorted_pods.begin();
    for (size_t i = 0; i < scale_down_count; ++i) {
        if (sorted_it == sorted_pods.end()) {
            break;
        }
        else {
            pod_agent_it = pod_agent_map.find(sorted_it->second);
            if (pod_agent_it == pod_agent_map.end()) {
                continue;
            }
            ScheduleInfo* sched = new ScheduleInfo();
            sched->set_endpoint(pod_agent_it->second->endpoint());
            sched->set_podid(sorted_it->second);
            sched->set_jobid(job->jobid());
            sched->set_action(kTerminate);
            propose->push_back(sched);
            LOG(INFO, "scale down propose[%d] %s:%s on %s", propose_count,
                    sched->jobid().c_str(),
                    sched->podid().c_str(),
                    sched->endpoint().c_str());
            ++propose_count;
            ++sorted_it;
        }
    }
    return propose_count;
}

void AgentInfoExtend::CalcExtend() {
    unassigned.CopyFrom(agent_info.total());
    std::vector<Resource> alloc;
    bool ok = ResourceUtils::AllocResource(agent_info.assigned(), alloc, &agent_info);
    if (!ok) {
        LOG(WARNING, "fail to calc agent %s unassigned info extend", agent_info.endpoint().c_str());
        unassigned.set_millicores(0);
    }
    labels_set.clear();
    for (int i = 0; i < agent_info.tags_size(); i++) {
        labels_set.insert(agent_info.tags(i)); 
    }
}

int32_t Scheduler::UpdateAgent(const AgentInfo* agent_info) {
    boost::unordered_map<std::string, AgentInfoExtend*>::iterator agt_it =
      resources_.find(agent_info->endpoint());
    if (agt_it != resources_.end()) {
        LOG(INFO, "update agent %s", agent_info->endpoint().c_str());
        agt_it->second->agent_info->CopyFrom(*agent_info);
        agt_it->second->CalcExtend();
        return 0;
    }
    else {
        LOG(INFO, "update agent %s not exists", agent_info->endpoint().c_str());
        return 1;
    }
}

int32_t Scheduler::ScheduleUpdate(std::vector<JobInfo*>& update_jobs,
                                  std::vector<ScheduleInfo*>* propose) {
    std::vector<JobInfo*>::iterator job_it = update_jobs.begin();
    for(; job_it != update_jobs.end(); ++job_it) {
        JobInfo* job_info = *job_it; 
        HandleJobUpdate(job_info, propose);
    }
    return 0;
}

void Scheduler::HandleJobUpdate(JobInfo* job_info,
                                std::vector<ScheduleInfo*>* propose) {
    std::vector<PodStatus*> need_update_pods;
    int32_t updating_count = 0;
    for (int32_t i = 0; i < job_info->pods_size(); i++) {
        PodStatus* pod = job_info->mutable_pods(i);
        if (pod->version() != job_info->latest_version()) {
            need_update_pods.push_back(pod); 
        }
        if ( pod->stage() == kStageDeath
           || pod->state() == kPodDeploying) {
            updating_count++;
        }
    }
    LOG(INFO, "job %s update stat: updating_count %d, update_step_size %d, need_update_size %u",
        job_info->jobid().c_str(), updating_count, job_info->desc().deploy_step(), need_update_pods.size());
    for (size_t i = 0; i < need_update_pods.size() && updating_count < job_info->desc().deploy_step() ; i++ ) {
        updating_count++;
        ScheduleInfo* sched_info = new ScheduleInfo();
        PodStatus* pod = need_update_pods[i];
        sched_info->set_endpoint(pod->endpoint());
        LOG(INFO, "choose agent %s update pod %s of job %s", pod->endpoint().c_str(), pod->podid().c_str(), pod->jobid().c_str());
        sched_info->set_jobid(pod->jobid());
        sched_info->set_podid(pod->podid());
        sched_info->set_action(kTerminate);
        propose->push_back(sched_info);
    }
}

void Scheduler::BuildSyncJobRequest(GetJobDescriptorRequest* request) {
    MutexLock lock(&sched_mutex_);
    boost::unordered_map<std::string, JobDescriptor>::iterator it = jobs_->begin();
    for (; it !=  jobs_->end(); ++it) {
        JobIdDiff* diff = request->mutable_jobs()->Add();
        diff->set_jobid(it->first);
        diff->set_version(it->second.pod().version());
    }
}

void Scheduler::SyncJobDescriptor(const GetJobDescriptorResponse* response) {
    MutexLock lock(&sched_mutex_);
    for (int i = 0; i < response->jobs_size(); ++i) {
        LOG(INFO, "update job %s desc", response->jobs(i).jobid().c_str());
        boost::unordered_map<std::string, JobDescriptor>::iterator it = jobs_->find(response->jobs(i).jobid());
        if (it == jobs_->end()) {
            jobs_->insert(std::make_pair(response->jobs(i),jobid(), response->jobs(i).desc()));
        }else {
            it->second = response->jobs(i).desc();
        }
    }
    for (int i = 0; i < response->deleted_jobs_size(); ++i) {
        LOG(INFO, "delete job %s desc", response->deleted_jobs(i).c_str());
        jobs_->erase(response->deleted_jobs(i));
    }
}

}// galaxy
}// baidu
