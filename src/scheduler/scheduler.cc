// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scheduler/scheduler.h"

#include <math.h>
#include <algorithm>
#include "logging.h"

namespace baidu {
namespace galaxy {

const static int feasibility_factor = 2;

// cpu单位为milli
const static double cpu_used_factor = 10.0f;

const static double mem_used_factor = 1.0f;

const static double prod_count_factor = 100.0f;

//const static double non_prod_count_factor = 2;


double Scheduler::CalcLoad(const AgentInfo& agent) {
    double cpu_load = agent.used().millicores() * cpu_used_factor /
            agent.total().millicores();
    double mem_load = agent.used().memory() * mem_used_factor /
            agent.total().memory();
    double prod_load = agent.pods_size() / prod_count_factor;
    // TODO: Agent增加non-prod计数
    //double non_prod_load = agent.pods_size() / prod_count_factor;
    return exp(cpu_load) + exp(mem_load) + exp(prod_load);
}

/*
 * @brief 按照磁盘可用大小升序排列
 */
static bool VolumeCompare(const Volume& volume1, const Volume& volume2) {
    return volume1.quota() < volume2.quota();
}

/*
 * @brief 按照priority降序排序
 *
 * */
static bool JobCompare(const JobInfo* left, const JobInfo* right) {
    return left->desc().priority() > right->desc().priority();
}

int32_t Scheduler::ScheduleScaleUp(std::vector<JobInfo*>& pending_jobs,
                 std::vector<ScheduleInfo*>* propose) {
    LOG(INFO, "schedule scale up turns: %lld", schedule_turns_);
    ++schedule_turns_;

    int propose_count = 0;
    // 计算job优先级，及其需要调度的pod数量
    std::vector<PodScaleUpCell*> pending_pods;

    // pod 依照优先级进行排序
    int32_t total_feasible_count = ChoosePendingPod(pending_jobs, &pending_pods);
    LOG(INFO, "feasibility checking count : %d, factor %d"
            , total_feasible_count, feasibility_factor);

    // shuffle resources_
    std::vector<AgentInfo*> resources_to_alloc;
    ChooseRecourse(&resources_to_alloc);
    LOG(INFO, "resources choosen : %u", resources_to_alloc.size());

    // 计算feasibility
    std::vector<AgentInfo*>::iterator res_it = resources_to_alloc.begin();
    int cur_feasible_count = 0;
    for (; res_it != resources_to_alloc.end() &&
            cur_feasible_count < total_feasible_count; ++res_it) {
        for (std::vector<PodScaleUpCell*>::iterator pod_it = pending_pods.begin();
                pod_it != pending_pods.end(); ++pod_it) {
            if ((*pod_it)->feasible.size() >= (*pod_it)->feasible_limit ) {
                continue;
            } else {
                if ((*pod_it)->FeasibilityCheck(*res_it) == true) {
                    (*pod_it)->feasible.push_back(*res_it);
                    cur_feasible_count++;
                    LOG(DEBUG, "feasibility checking pass: %s on %s",
                            (*pod_it)->job->jobid().c_str(),
                            (*res_it)->endpoint().c_str());
                }
                // 此处不break，说明一个Agent尽量调度多的Pod
            }
        }
    }

    // 对增加实例任务进行优先级计算
    for (std::vector<PodScaleUpCell*>::iterator pod_it = pending_pods.begin();
            pod_it != pending_pods.end(); ++pod_it) {
        (*pod_it)->Score();
        uint32_t count = (*pod_it)->Propose(propose);
        propose_count += count;
        LOG(DEBUG, "propose jobid %s count %u", (*pod_it)->job->jobid().c_str(), count);
    }

    // 销毁PodScaleUpCell
    for (std::vector<PodScaleUpCell*>::iterator pod_it = pending_pods.begin();
            pod_it != pending_pods.end(); ++pod_it) {
        delete *pod_it;
    }

    return propose_count;
}

int32_t Scheduler::ChooseRecourse(std::vector<AgentInfo*>* resources_to_alloc) {
    std::map<std::string, AgentInfo*>::iterator it = resources_.begin();
    for (; it != resources_.end(); ++it) {
        resources_to_alloc->push_back(it->second);
        LOG(DEBUG, "agent resource free: millicores %d memory %d MB #disk %u #ssd %u",
                it->second->free().millicores(), it->second->free().memory(),
                it->second->free().disks_size(), it->second->free().ssds_size());
        LOG(DEBUG, "agent resource unassigned: millicores %d memory %d MB #disk %u #ssd %u",
                it->second->unassigned().millicores(), it->second->unassigned().memory(),
                it->second->unassigned().disks_size(), it->second->unassigned().ssds_size());
    }
    return 0;
}

int32_t Scheduler::ScheduleScaleDown(std::vector<JobInfo*>& reducing_jobs,
                 std::vector<ScheduleInfo*>* propose) {
    int propose_count = 0;
    // 计算job优先级，及其需要调度的pod数量
    std::vector<PodScaleDownCell*> reducing_pods;

    int32_t total_reducing_count = ChooseReducingPod(reducing_jobs, &reducing_pods);

    assert(total_reducing_count >= 0);

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
   LOG(INFO, "sync resource from master");
   std::map<std::string, AgentInfo*>::iterator agt_it = resources_.begin();
   for (; agt_it != resources_.end(); agt_it++) {
       delete agt_it->second;
   }
   resources_.clear();

   for (int32_t i =0 ; i < response->agents_size(); i++) {
       AgentInfo* agent = new AgentInfo();
       agent->CopyFrom(response->agents(i));
       resources_.insert(std::make_pair(agent->endpoint(), agent));
   }
   return resources_.size();
}

int32_t Scheduler::ChoosePendingPod(std::vector<JobInfo*>& pending_jobs,
                                    std::vector<PodScaleUpCell*>* pending_pods) {
    // 获取scale up的任务
    int feasibility_count = 0;

    // 按照Job优先级进行排序
    std::sort(pending_jobs.begin(), pending_jobs.end(), JobCompare);
    std::vector<JobInfo*>::iterator job_it = pending_jobs.begin();
    for (; job_it != pending_jobs.end(); ++job_it) {
        PodScaleUpCell* cell = new PodScaleUpCell();
        cell->pod = (*job_it)->mutable_desc()->mutable_pod();
        cell->job = *job_it;
        cell->feasible_limit = (*job_it)->pods_size() * feasibility_factor;
        feasibility_count += cell->feasible_limit;
        for (int i = 0; i < (*job_it)->pods_size(); ++i) {
            cell->pod_ids.push_back((*job_it)->pods(i).podid());
        }
        CalcSources(*(cell->pod), &(cell->resource));
        pending_pods->push_back(cell);
    }
    return feasibility_count;
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
            std::map<std::string, AgentInfo*>::iterator agt_it =
                    resources_.find((*job_it)->pods(i).endpoint());
            if (agt_it == resources_.end()) {
                LOG(INFO, "scale down pod %s dose not belong to agent %s",
                        (*job_it)->pods(i).podid().c_str(),
                        (*job_it)->pods(i).endpoint().c_str());
            }
            else {
                cell->pod_agent_map.insert(
                        std::make_pair((*job_it)->pods(i).podid(), agt_it->second));
            }
        }
        // 计算需要scale_down数目
        int32_t scale_down_count = (*job_it)->pods_size() - cell->job->desc().replica();
        cell->scale_down_count = scale_down_count;
        reducing_count += scale_down_count;
        reducing_pods->push_back(cell);
    }
    return reducing_count;
}

PodScaleUpCell::PodScaleUpCell():pod(NULL), job(NULL),
        schedule_count(0), feasible_limit(0) {}

bool PodScaleUpCell::FeasibilityCheck(const AgentInfo* agent_info) {
    // 对于prod任务(kLongRun,kSystem)，根据unassign值check
    // 对于non-prod任务(kBatch)，根据free值check
    if (job->desc().type() == kLongRun ||
            job->desc().type() == kSystem) {
        // 判断CPU是否满足
        if (agent_info->unassigned().millicores() < resource.millicores()) {
            return false;
        }
        // 判断mem
        if (agent_info->unassigned().memory() < resource.memory()) {
            return false;
        }
    }
    else if (job->desc().type() == kBatch) {
        // 判断CPU是否满足
        if (agent_info->free().millicores() < resource.millicores()) {
            return false;
        }
        // 判断mem
        if (agent_info->free().memory() < resource.memory()) {
            return false;
        }
    }
    else {
        // 不支持类型
        return false;
    }

    // 判断ports
    if (resource.ports_size() > 0) {
        for (int i = 0; i < agent_info->used().ports_size(); i++) {
            for (int j = 0; j < resource.ports_size(); j++) {
                if (agent_info->used().ports(i) == resource.ports(j)) {
                    return false;
                }
            }
        }
    }
    // 判断disks
    if (resource.disks_size() > 0) {
        if (resource.disks_size() > agent_info->unassigned().disks_size()) {
            return false;
        }
        std::vector<Volume> unassigned;
        for (int i = 0; i < agent_info->unassigned().disks_size(); i++) {
            unassigned.push_back(agent_info->unassigned().disks(i));
        }
        std::vector<Volume> required;
        for (int j = 0; j < resource.disks_size(); j++) {
            required.push_back(resource.disks(j));
        }
        if (!VolumeFit(unassigned, required)) {
            return false;
        }
    }

    // 判断ssd
    if (resource.ssds_size() > 0) {
         if (resource.ssds_size() > agent_info->unassigned().ssds_size()) {
             return false;
         }
         std::vector<Volume> unassigned;
         for (int i = 0; i < agent_info->unassigned().ssds_size(); i++) {
             unassigned.push_back(agent_info->unassigned().ssds(i));
         }
         std::vector<Volume> required;
         for (int j = 0; j < resource.ssds_size(); j++) {
             required.push_back(resource.ssds(j));
         }
         if (!VolumeFit(unassigned, required)) {
                 return false;
         }
    }
    return true;
}

bool PodScaleUpCell::VolumeFit(std::vector<Volume>& unassigned,
                                    std::vector<Volume>& required) {
    // 磁盘需求升序排列, best fit
    std::sort(unassigned.begin(), unassigned.end(), VolumeCompare);
    std::sort(required.begin(), required.end(), VolumeCompare);
    size_t fit_index = 0;
    for (size_t i = 0; i < unassigned.size() && fit_index < required.size(); ++i) {
        if (required[fit_index].quota() <= unassigned[i].quota()) {
            ++fit_index;
        }
    }
    if (fit_index  < (required.size() - 1)) {
        return false;
    }
    return true;
}

int32_t PodScaleUpCell::Score() {
    std::vector<AgentInfo*>::iterator agt_it = feasible.begin();
    for(; agt_it != feasible.end(); ++agt_it) {
        double score = ScoreAgent(*agt_it, pod);
        sorted.insert(std::make_pair(score, *agt_it));
    }
    return 0;
}

int32_t PodScaleUpCell::Propose(std::vector<ScheduleInfo*>* propose) {
    int propose_count = 0;
    std::map<double, AgentInfo*>::iterator sorted_it = sorted.begin();
    for (size_t i = 0; i < pod_ids.size(); ++i) {
        if (sorted_it == sorted.end()) {
            break;
        }
        else {
            ScheduleInfo* sched = new ScheduleInfo();
            sched->set_endpoint(sorted_it->second->endpoint());
            sched->set_podid(pod_ids[i]);
            sched->set_action(kLaunch);
            propose->push_back(sched);
            LOG(DEBUG, "propose[%d] %s on %s", propose_count,
                    sched->podid().c_str(),
                    sched->endpoint().c_str());
            ++propose_count;
            ++sorted_it;
        }
    }
    return propose_count;
}

double PodScaleUpCell::ScoreAgent(const AgentInfo* agent_info,
                   const PodDescriptor* desc) {
    // 计算机器当前使用率打分
    double score = Scheduler::CalcLoad(*agent_info);
    LOG(DEBUG, "score %s %lf", agent_info->endpoint().c_str(), score);
    return score;
}

PodScaleDownCell::PodScaleDownCell() : pod(NULL), job(NULL), scale_down_count(0) {}

int32_t PodScaleDownCell::Score() {
    std::map<std::string, AgentInfo*>::iterator pod_agt_it = pod_agent_map.begin();
    for(; pod_agt_it != pod_agent_map.end(); ++pod_agt_it) {
        double score = ScoreAgent(pod_agt_it->second, pod);
        sorted_pods.insert(std::make_pair(score, pod_agt_it->first));
    }
    return 0;
}

double PodScaleDownCell::ScoreAgent(const AgentInfo* agent_info,
                   const PodDescriptor* desc) {
    // 计算机器当前使用率打分
    double score = Scheduler::CalcLoad(*agent_info);
    LOG(DEBUG, "score %s %lf", agent_info->endpoint().c_str(), score);
    return -1 * score;
}

int32_t PodScaleDownCell::Propose(std::vector<ScheduleInfo*>* propose) {
    int propose_count = 0;
    std::map<std::string, AgentInfo*>::iterator pod_agent_it;
    std::map<double, std::string>::iterator sorted_it = sorted_pods.begin();
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
            sched->set_action(kTerminate);
            propose->push_back(sched);
            LOG(DEBUG, "propose[%u] %s on %s", propose_count,
                    sched->podid().c_str(),
                    sched->endpoint().c_str());
            ++propose_count;
            ++sorted_it;
        }
    }
    return propose_count;
}

int32_t Scheduler::CalcSources(const PodDescriptor& pod, Resource* resource) {
    for (int j = 0; j < pod.tasks_size(); ++j) {
        int32_t millicores = resource->millicores();
        resource->set_millicores(millicores + pod.tasks(j).requirement().millicores());
        int32_t memory = resource->memory();
        resource->set_memory(memory + pod.tasks(j).requirement().memory());
        for (int h = 0; h < pod.tasks(j).requirement().ports_size(); ++h) {
            resource->add_ports(pod.tasks(j).requirement().ports(h));
        }
        for (int h = 0; h < pod.tasks(j).requirement().disks_size(); ++h) {
            Volume* volume = resource->add_disks();
            volume->CopyFrom(pod.tasks(j).requirement().disks(h));
        }
        for (int h = 0; h < pod.tasks(j).requirement().ssds_size(); ++h) {
            Volume* volume = resource->add_ssds();
            volume->CopyFrom(pod.tasks(j).requirement().ssds(h));
        }
    }
    LOG(DEBUG, "pod resource requirement: millicores %d memory %d MB #disk %u #ssd %u",
            resource->millicores(), resource->memory(),
            resource->disks_size(), resource->ssds_size());
    return 0;
}

int32_t Scheduler::UpdateAgent(const AgentInfo* agent_info) {
    std::map<std::string, AgentInfo*>::iterator agt_it = resources_.find(agent_info->endpoint());
    if (agt_it != resources_.end()) {
        LOG(INFO, "update agent %s", agent_info->endpoint().c_str());
        agt_it->second->CopyFrom(*agent_info);
        return 0;
    }
    else {
        LOG(INFO, "update agent %s not exists", agent_info->endpoint().c_str());
        return 1;
    }
}

int32_t Scheduler::SyncJobOverview(const ListJobResponse* response) {
    MutexLock lock(mutex_);
    LOG(INFO, "start to sync job overview , old job count %u", job_overview_.size());
    for (std::map<std::string, JobOverview*>::iterator job_it = job_overview_.begin();
       job_it != job_overview_.end(); ++job_it) {
           delete job_it->second
    }
    job_overview_.clear();

    for (size_t i = 0; i < response->jobs_size(); ++i) {
        JobOverview* job = new JobOverview();
        job->CopyFrom(response->jobs(i));
        job_overview_.insert(std::make_pair(job->jobid(), job));
    }
    LOG(INFO, "sync job overview successfully , job count %u", job_overview_.size());
    return job_overview_.size();
}

int32_t Scheduler::CheckAgentOverLoad(std::vector<ScheduleInfo*>* propose) {
    LOG(INFO, "start to check agent overload,  job count %u, agent count %u",
        job_overview_.size(), resources_.size());



}

}// galaxy
}// baidu
