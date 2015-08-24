// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BAIDU_GALAXY_JOB_MANGER_H
#define BAIDU_GALAXY_JOB_MANGER_H
#include <string>
#include <set>
#include <map>
#include <vector>

#include <boost/unordered_map.hpp>
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
    void GetPendingPods(JobInfoList* pending_pods,
                        int32_t max_scale_up_size,
                        JobInfoList* scale_down_pods,
                        int32_t max_scale_down_size);
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
    Status LabelAgents(const LabelCell& label_cell);
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

    void SendKillToAgent(PodStatus* pod);
    void Query();
    void QueryAgent(AgentInfo* agent);
    void QueryAgentCallback(AgentAddr endpoint, const QueryRequest* request,
                            QueryResponse* response, bool failed, int error, 
                            int64_t seq_id_at_query);
    void UpdateAgentVersion(AgentInfo* old_agent_info, 
                            const AgentInfo& new_agent_info);
    
    bool CompareAgentInfo(const AgentInfo* old_agent_info, const AgentInfo* new_agent_info);

    void ScheduleNextQuery();
    void FillPodsToJob(Job* job);
    void FillAllJobs();
    void KillPodCallback(PodStatus* pod, const KillPodRequest* request, 
                         KillPodResponse* response, 
                         bool failed, int error);

    void ProcessScaleDown(JobInfoList* scale_down_pods, 
                          int32_t max_scale_down_size);
    void ProcessScaleUp(JobInfoList* scale_up_pods,
                        int32_t max_scale_up_size);
    bool BuildPodFsm(PodStatus* pod, Job* job);
    bool HandleCleanPod(PodStatus* pod, Job* job);
    bool HandlePendingToRunning(PodStatus* pod, Job* job);
    bool HandleRunningToDeath(PodStatus* pod, Job* job);
    bool HandleRunningToRemoved(PodStatus* pod, Job* job);
    bool HandleDeathToPending(PodStatus* pod, Job* job);
    bool HandleDoNothing();
    std::string BuildHandlerKey(const PodStage& from,
                                const PodStage& to);

    void ChangeStage(const PodStage& to,
                     PodStatus* pod);
private:
    std::map<JobId, Job*> jobs_;
    typedef std::map<JobId, std::map<PodId, PodStatus*> > PodMap;
    PodMap pending_pods_;
    // all jobs that needs scale down
    std::set<JobId> scale_down_jobs_;

    std::map<AgentAddr, PodMap> pods_on_agent_;
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

    // for label on agent 
    typedef std::string LabelName;
    typedef std::set<std::string> AgentSet;
    typedef std::set<std::string> LabelSet;
    std::map<LabelName, AgentSet> labels_;
    boost::unordered_map<AgentAddr, LabelSet> agent_labels_;

    // pod fsm
    std::map<PodState, PodStage> state_to_stage_;
    typedef boost::function<bool ()> Handle;
    typedef boost::unordered_map<PodId, std::map<std::string, Handle> > FSM; 
    FSM fsm_;
    std::map<AgentAddr, int64_t> agent_sequence_ids_;
};

}
}

#endif
