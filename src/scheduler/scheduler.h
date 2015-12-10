// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SCHEDULER_H
#define BAIDU_GALAXY_SCHEDULER_H
#include <map>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "proto/master.pb.h"

namespace baidu {
namespace galaxy {

struct PodScaleUpCell {
    PodDescriptor pod;
    JobInfo job;
    uint32_t schedule_count;
    uint32_t feasible_limit;
    std::vector<std::string> pod_ids;
    std::vector<AgentInfo> feasible;
    std::vector<AgentInfo> preemption;
    std::vector<std::pair<double, std::string> > sorted;

    PodScaleUpCell();

    bool FeasibilityCheck(const AgentInfoExtend* agent_info);

    bool PreemptionCheck(const AgentInfoExtend* agent_info);

    void Score();

    double ScoreAgent(const AgentInfo& agent_info,
                      const PodDescriptor& desc);

    int32_t Propose(std::vector<ScheduleInfo>* propose);

};

struct PodScaleDownCell {
    PodDescriptor* pod;
    JobInfo* job;
    uint32_t scale_down_count;
    std::map<std::string, AgentInfo*> pod_agent_map;
    std::vector<std::pair<double, std::string> > sorted_pods;
    PodScaleDownCell();
    int32_t Score();

    double ScoreAgent(const AgentInfo* agent_info,
                       const PodDescriptor* desc);

    int32_t Propose(std::vector<ScheduleInfo*>* propose);
};


class Scheduler {

public:
    static double CalcLoad(const AgentInfo& agent);

    Scheduler() {
        jobs_ = new boost::unordered_map<std::string, JobDescriptor>();
        resources_ = new boost::unordered_map<std::string, AgentInfo>();
    }
    ~Scheduler() {
        delete jobs_;
    }

    int32_t ScheduleScaleUp(const std::string& master_addr,
                            std::vector<JobInfo*>& pending_jobs);

    // 调度算入口: scale down
    // @note 调用者负责销毁 propose
    int32_t ScheduleScaleDown(std::vector<JobInfo>& reducing_jobs);

    int32_t ScheduleUpdate(std::vector<JobInfo*>& update_jobs,
                           std::vector<ScheduleInfo*>* propose);
    // 从master同步资源数据
    int32_t SyncResources(const GetResourceSnapshotResponse* response);

    // master通知AgentInfo信息过期，需要更新
    int32_t UpdateAgent(const AgentInfo* agent_info);

    void BuildSyncRequest(GetResourceSnapshotRequest* request);

    void BuildSyncJobRequest(GetJobDescriptorRequest* request);
    void SyncJobDescriptor(const GetJobDescriptorResponse* response);
private:

    int32_t ChoosePods(const std::vector<JobInfo>& pending_jobs,
                       std::vector<PodScaleUpCell*>* pending_pods);

    int32_t ChooseReducingPod(std::vector<JobInfo*>& reducing_jobs,
                              std::vector<PodScaleDownCell*>* reducing_pods);


    void HandleJobUpdate(JobInfo* job_info, 
                        std::vector<ScheduleInfo*>* propose);
    template<class T>
    void Shuffle(std::vector<T>& list) {
        for (size_t i = list.size(); i > 1; i--) {
            T tmp = list[i-1];
            size_t target_index = ::rand() % i ;
            list[i-1] = list[target_index];
            list[target_index] = tmp;
        }
    }

    bool CalcPreemptStep(AgentInfoExtend* agent, PodScaleUpCell* pods);
    void Propose(const std::vector<ScheduleInfo>& sched_infos, const std::string& master_addr);
    void ProposeCallBack(const ProposeRequest* request,
                         ProposeResponse* response,
                         bool failed, int);
private:
    Mutex sched_mutex_;
    boost::unordered_map<std::string, AgentInfo>* resources_;
    boost::unordered_map<std::string, JobDescriptor>* jobs_;
    RpcClient rpc_client_;
};


} // galaxy
}// baidu
#endif
