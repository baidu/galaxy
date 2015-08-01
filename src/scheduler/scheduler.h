// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SCHEDULER_H
#define BAIDU_GALAXY_SCHEDULER_H
#include <map>
#include <string>
#include "proto/master.pb.h"


namespace baidu {
namespace galaxy {

struct PodScheduleCell {
	PodDescriptor* pod;
	std::vector<std::string> pod_ids;
	JobInfo* job;
	uint32_t schedule_count;
	uint32_t feasible_limit;
	Resource resource;
	std::vector<AgentInfo*> feasible;
	std::map<float, AgentInfo*> sorted;

	PodScheduleCell();


    bool FeasibilityCheck(const AgentInfo* agent_info);

	int32_t Score();

	float ScoreAgent(const AgentInfo* agent_info,
                       const PodDescriptor* desc);

    int32_t Propose(std::vector<ScheduleInfo*>* propose);

    bool VolumeFit(std::vector<Volume>& unassigned,
    		       std::vector<Volume>& required);

};

bool VolumeCompare(const Volume& volume1, const Volume& volume2) {
	return volume1.quota() < volume2.quota();
}

/*
 * @brief 按照priority排序
 *
 * */
bool  PodCompare(const PodScheduleCell* cell1, const PodScheduleCell* cell2) {
	return cell1->job->job().priority() > cell2->job->job().priority();
}

class Scheduler {

public:
    Scheduler(){}
    ~Scheduler() {}

    /*
     * @brief 调度算入口
     * @param
     *  pending_jobs [IN] : 待调度任务，包括scale up/scale down
     *  propose [OUT] : 调度结果,
     * @note
     *  调用者负责销毁 propose
     *
     */
    int32_t Schedule(std::vector<JobInfo*>& pending_jobs,
                     std::vector<ScheduleInfo*>* propose);

    /*
     * @brief 从master同步资源数据
     * @param
     * 	response [IN] : Agent全量状态
     *
     */
    int32_t SyncResources(const GetResourceSnapshotResponse* response);

    /**
     * @brief master通知AgentInfo信息过期，需要更新
     *
     */
    int32_t UpdateAgent(const AgentInfo* agent_info);

private:

    int32_t ChoosePendingPod(std::vector<JobInfo*>& pending_jobs,
    			std::vector<PodScheduleCell*>* pending_pods,
    		std::vector<PodScheduleCell*>* reducing_pods);

    int32_t CalcSources(const PodDescriptor& pod, Resource* resource);

    std::vector<AgentInfo*> resources_;

};


} // galaxy
}// baidu
#endif
