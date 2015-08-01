// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scheduler/scheduler.h"

#include <algorithm>
#include "logging.h"

namespace baidu {
namespace galaxy {

const static int feasibility_factor = 2;

const static float cpu_used_factor = 2;

const static float cpu_assigned_factor = 2;

const static float prod_count_factor = 2;

const static float non_prod_count_factor = 2;

const static float mem_used_factor = 2;

const static float mem_assigned_factor = 2;

const static float disk_used_factor = 2;

const static float disk_assigned_factor = 2;

PodScheduleCell::PodScheduleCell():pod(NULL), job(NULL),
		schedule_count(0), feasible_limit(0) {}

bool PodScheduleCell::FeasibilityCheck(const AgentInfo* agent_info) {
	// 对于prod任务(kLongRun,kSystem)，根据unassign值check
	// 对于non-prod任务(kBatch)，根据free值check
	if (job->job().type() == kLongRun ||
			job->job().type() == kSystem) {
		// 判断CPU是否满足
		if (agent_info->unassigned().millicores() < resource.millicores()) {
			return false;
		}
		// 判断mem
		if (agent_info->unassigned().memory() < resource.memory()) {
			return false;
		}
	}
	else if (job->job().type() == kBatch) {
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

bool PodScheduleCell::VolumeFit(std::vector<Volume>& unassigned,
	     	 	 	 	        std::vector<Volume>& required) {
	std::sort(unassigned.begin(), unassigned.end(), VolumeCompare);
	std::sort(required.begin(), required.end(), VolumeCompare);
	size_t fit_index = 0;
	for (size_t i = 0; i < unassigned.size()
					&& fit_index < required.size(); i++) {
		if (required[fit_index].quota() >= unassigned[i].quota()) {
				fit_index ++;
		}
		if (fit_index  < (required.size() - 1)) {
			return false;
		}
	}
	return true;
}

int32_t PodScheduleCell::Score() {
	std::vector<AgentInfo*>::iterator agt_it = feasible.begin();
	for(; agt_it != feasible.end(); ++agt_it) {
		float score = ScoreAgent(*agt_it, pod);
		sorted.insert(std::make_pair(score, *agt_it));
	}
	return 0;
}

int32_t PodScheduleCell::Propose(std::vector<ScheduleInfo*>* propose) {
	int propose_count = 0;
	std::map<float, AgentInfo*>::iterator sorted_it = sorted.begin();
	for (size_t i = 0; i < pod_ids.size(); ++i) {
		if (sorted_it == sorted.end()) {
			break;
		}
		else {
			ScheduleInfo* sched = new ScheduleInfo();
			sched->set_endpoint(sorted_it->second->endpoint());
			sched->set_podid(pod_ids[i]);
			propose->push_back(sched);
			propose_count ++;
			++sorted_it;
		}
	}
	return propose_count;
}


float PodScheduleCell::ScoreAgent(const AgentInfo* agent_info,
                   const PodDescriptor* desc) {
	int prod_count = 0;
	int non_prod_count = 0;
	// 计算机器当前使用率打分
	float score = cpu_used_factor * agent_info->used().millicores() +
			 cpu_assigned_factor * agent_info->assigned().millicores() +
			mem_used_factor * agent_info->used().memory() +
			mem_assigned_factor * agent_info->assigned().memory() +
			prod_count_factor * prod_count +
			non_prod_count_factor * non_prod_count;
	return score;
}

int32_t Scheduler::SyncResources(const GetResourceSnapshotResponse* response) {
   LOG(INFO, "sync resource from master");
   std::vector<AgentInfo*>::iterator agt_it = resources_.begin();
   for (; agt_it != resources_.end(); agt_it++) {
       delete *agt_it;
   }
   resources_.clear();

   for (int32_t i =0 ; i < response->agents_size(); i++) {
       AgentInfo* agent = new AgentInfo();
       agent->CopyFrom(response->agents(i));
       resources_.push_back(agent);
   }

   return resources_.size();
}

int32_t Scheduler::Schedule(std::vector<JobInfo*>& pending_jobs,
                 std::vector<ScheduleInfo*>* propose) {
	// 计算job优先级，及其需要调度的pod数量
	std::vector<PodScheduleCell*> pending_pods;
	std::vector<PodScheduleCell*> reducing_pods;

	// pod 依照优先级进行排序
	int32_t total_feasible_count = ChoosePendingPod(pending_jobs, &pending_pods, &reducing_pods);


	// TODO: shuffle resources_

	// 计算feasibility
	std::vector<AgentInfo*>::iterator res_it = resources_.begin();
	int cur_feasible_count = 0;
	for (; res_it != resources_.end() &&
			cur_feasible_count < total_feasible_count; ++res_it) {
		for (std::vector<PodScheduleCell*>::iterator pod_it = pending_pods.begin();
				pod_it != pending_pods.end(); ++pod_it) {
			if ((*pod_it)->feasible.size() >= (*pod_it)->feasible_limit ) {
				continue;
			} else {
				if ((*pod_it)->FeasibilityCheck(*res_it) == true) {
					(*pod_it)->feasible.push_back(*res_it);
					cur_feasible_count++;
				}
				// 此处不break，说明一个Agent尽量调度多的Pod
			}
		}
	}

	// 对增加实例任务进行优先级计算
	for (std::vector<PodScheduleCell*>::iterator pod_it = pending_pods.begin();
			pod_it != pending_pods.end(); ++pod_it) {
		(*pod_it)->Score();
		(*pod_it)->Propose(propose);
	}

	// 对减少实例任务进行优先级计算
	for (std::vector<PodScheduleCell*>::iterator pod_it = reducing_pods.begin();
			pod_it != reducing_pods.end(); ++pod_it) {
		(*pod_it)->Score();
		(*pod_it)->Propose(propose);
	}

	return 0;
}

int32_t Scheduler::ChoosePendingPod(std::vector<JobInfo*>& pending_jobs,
								    std::vector<PodScheduleCell*>* pending_pods,
									std::vector<PodScheduleCell*>* reducing_pods) {
	// 获取scale up的任务
	std::vector<JobInfo*>::iterator job_it = pending_jobs.begin();
	for (; job_it != pending_jobs.end(); ++job_it) {
		PodScheduleCell* cell = new PodScheduleCell();
		cell->pod = (*job_it)->mutable_job()->mutable_pod();
		cell->job = *job_it;
		cell->feasible_limit = (*job_it)->tasks_size() * feasibility_factor;
		for (int i = 0; i < (*job_it)->tasks_size(); ++i) {
			cell->pod_ids.push_back((*job_it)->tasks(i).podid());
		}
		CalcSources(*(cell->pod), &(cell->resource));
		pending_pods->push_back(cell);
	}
	std::sort(pending_pods->begin(), pending_pods->end(), PodCompare);

	// 获取scale down的任务


	return 0;
}

int32_t CalcSources(const PodDescriptor& pod, Resource* resource) {
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
	return 0;
}

int32_t Scheduler::UpdateAgent(const AgentInfo* agent_info) {
	return 0;
}

}// galaxy
}// baidu
