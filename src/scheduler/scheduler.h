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

struct PodScaleUpCell {
	PodDescriptor* pod;
	JobInfo* job;
	uint32_t schedule_count;
	uint32_t feasible_limit;
	Resource resource;
	std::vector<std::string> pod_ids;
	std::vector<AgentInfo*> feasible;
	std::map<float, AgentInfo*> sorted;

	PodScaleUpCell();


    bool FeasibilityCheck(const AgentInfo* agent_info);

	int32_t Score();

	float ScoreAgent(const AgentInfo* agent_info,
                       const PodDescriptor* desc);

    int32_t Propose(std::vector<ScheduleInfo*>* propose);

    bool VolumeFit(std::vector<Volume>& unassigned,
    		       std::vector<Volume>& required);

};

struct PodScaleDownCell {
	PodDescriptor* pod;
	JobInfo* job;
	uint32_t scale_down_count;
	std::map<std::string, AgentInfo*> pod_agent_map;
	std::map<float, std::string> sorted_pods;

	PodScaleDownCell();

	int32_t Score();

	float ScoreAgent(const AgentInfo* agent_info,
                       const PodDescriptor* desc);

    int32_t Propose(std::vector<ScheduleInfo*>* propose);

};

class Scheduler {

public:
    Scheduler(){}
    ~Scheduler() {}

    /*
     * @brief 调度算入口: scale up
     * @param
     *  pending_jobs [IN] : 待调度任务，scale up
     *  propose [OUT] : 调度结果
     * @return
     *   返回propose数量
     * @note
     *  调用者负责销毁 propose
     *
     */
    int32_t ScheduleScaleUp(std::vector<JobInfo*>& pending_jobs,
                     std::vector<ScheduleInfo*>* propose);

    /*
     * @brief 调度算入口: scale down
     * @param
     *  pending_jobs [IN] : 待调度任务，scale down
     *  propose [OUT] : 调度结果,
     * @return
     *   返回propose数量
     * @note
     *  调用者负责销毁 propose
     *
     */
    int32_t ScheduleScaleDown(std::vector<JobInfo*>& reducing_jobs,
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
    			std::vector<PodScaleUpCell*>* pending_pods);

    int32_t ChooseReducingPod(std::vector<JobInfo*>& reducing_jobs,
				std::vector<PodScaleDownCell*>* reducing_pods);

    int32_t ChooseRecourse(std::vector<AgentInfo*>* resources_to_alloc);

    int32_t CalcSources(const PodDescriptor& pod, Resource* resource);

    std::map<std::string, AgentInfo*> resources_;
};


} // galaxy
}// baidu
#endif
