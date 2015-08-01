// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BAIDU_GALAXY_JOB_MANGER_H
#define BAIDU_GALAXY_JOB_MANGER_H
#include <string>
#include <map>
#include <vector>

//#include <mutex.h>
#include <thread_pool.h>

#include "proto/agent.pb.h"
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

typedef std::string JobId;
typedef std::string PodId;
typedef std::string AgentAddr;
typedef google::protobuf::RepeatedPtrField<baidu::galaxy::JobInfo> JobInfoList;

struct Job {
    JobState state_;
    std::map<PodId, PodStatus*> pods_;
    JobDescriptor desc_;
};

class JobManager {
public:
    void Add(const JobDescriptor& job_desc);
    Status Suspend(const JobId jobid);
    Status Resume(const JobId jobid);
    void GetPendingPods(JobInfoList* pending_pods);
    Status Propose(const ScheduleInfo& sche_info);
    void KeepAlive(const std::string& agent_addr);
    void DeployPod();

private:
    void SuspendPod(PodStatus* pod);
    void ResumePod(PodStatus* pod);
    Status AcquireResource(const PodStatus& pod, AgentInfo* agent);
    void ReclaimResource(const PodStatus& pod, AgentInfo* agent);
    void GetPodRequirement(const PodStatus& pod, Resource* requirement);
    void CalculatePodRequirement(const PodDescriptor& pod_desc, Resource* pod_requirement);
    void HandleAgentOffline(const std::string agent_addr);
    void ReschedulePod(PodStatus* pod_status);

    void RunPod(const PodDescriptor& desc, PodStatus* pod) ;
    void RunPodCallback(PodStatus* pod, AgentAddr endpoint, const RunPodRequest* request,
                        RunPodResponse* response, bool failed, int error);

private:
    std::map<JobId, Job*> jobs_;
    typedef std::map<JobId, std::map<PodId, PodStatus*> > PodMap;
    PodMap suspend_pods_;
    PodMap pending_pods_;
    PodMap deploy_pods_;
    std::map<AgentAddr, PodMap> running_pods_;
    std::map<AgentAddr, AgentInfo*> agents_;
    std::map<AgentAddr, int64_t> agent_timer_;
    ThreadPool death_checker_;
    Mutex mutex_;   
    Mutex mutex_timer_;
    RpcClient rpc_client_;
};

}
}

#endif
