// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BAIDU_GALAXY_JOB_MANGER_H
#define BAIDU_GALAXY_JOB_MANGER_H
#include <string>
#include <set>
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
typedef google::protobuf::RepeatedPtrField<baidu::galaxy::AgentInfo> AgentInfoList;
typedef google::protobuf::RepeatedPtrField<baidu::galaxy::JobOverview> JobOverviewList;
typedef google::protobuf::RepeatedPtrField<baidu::galaxy::DiffVersion> DiffVersionList;
typedef google::protobuf::RepeatedPtrField<std::string> StringList;

struct Job {
    JobState state_;
    std::map<PodId, PodStatus*> pods_;
    JobDescriptor desc_;
    JobId id_;
};

class JobManager {
public:
    void Add(const JobId& job_id, const JobDescriptor& job_desc);
    Status Update(const JobId& job_id, const JobDescriptor& job_desc);
    Status Suspend(const JobId& jobid);
    Status Resume(const JobId& jobid);
    JobManager();
    ~JobManager();
    void GetPendingPods(JobInfoList* pending_pods);
    Status Propose(const ScheduleInfo& sche_info);
    void GetAgentsInfo(AgentInfoList* agents_info);
    void GetAliveAgentsInfo(AgentInfoList* agents_info);
    void GetAliveAgentsByDiff(const DiffVersionList& versions,
                              AgentInfoList* agents_info,
                              StringList* deleted_agents);
    void GetJobsOverview(JobOverviewList* jobs_overview);
    Status GetJobInfo(const JobId& jobid, JobInfo* job_info);
    void KeepAlive(const std::string& agent_addr);
    void DeployPod();
    void ReloadJobInfo(const JobInfo& job_info);
    void KillPod(PodStatus* pod);
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

    void Query();
    void QueryAgent(AgentInfo* agent);
    void QueryAgentCallback(AgentAddr endpoint, const QueryRequest* request,
                            QueryResponse* response, bool failed, int error);
    void UpdateAgentVersion(const AgentInfo* old_agent_info, 
                            AgentInfo* new_agent_info);
    
    bool CompareAgentInfo(const AgentInfo* old_agent_info, const AgentInfo* new_agent_info);

    void ScheduleNextQuery();
    void FillPodsToJob(Job* job);
    void FillAllJobs();
    void KillPodCallback(PodStatus* pod, std::string agent_addr,
                         const KillPodRequest* request, KillPodResponse* response, 
                         bool failed, int error);
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
    ThreadPool thread_pool_;
    Mutex mutex_;   
    Mutex mutex_timer_;
    RpcClient rpc_client_;
    int64_t on_query_num_;
    std::set<AgentAddr> queried_agents_;
    bool safe_mode_;
};

}
}

#endif
