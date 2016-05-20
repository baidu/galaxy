// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BAIDU_GALAXY_JOB_MANGER_H
#define BAIDU_GALAXY_JOB_MANGER_H
#include <string>
#include <set>
#include <map>
#include <vector>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/unordered_map.hpp>
#include <thread_pool.h>
#include "ins_sdk.h"
#include "proto/agent.pb.h"
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

struct Job {
    JobStatus status_;
    std::map<std::string, PodInfo*> pods_;
    JobDescriptor desc_;
    JobId id_;
    std::map<Version, PodDescriptor> pod_desc_;
    std::set<std::string> deploying_pods_;
    std::string curent_version_;
    UpdateAction action_type_;
    int64_t create_time_;
    int64_t update_time_;
};

class JobManager {
public:
    void Start();
    Status Add(const JobId& job_id, const JobDescriptor& job_desc);
    Status Update(const JobId& job_id, const JobDescriptor& job_desc);
    Status Terminte(const JobId& jobid);
    Status AssignTask(const ::baidu::galaxy::proto::FecthTaskRequest* request,
                     ::baidu::galaxy::proto::FetchTaskResponse* response);
    void ReloadJobInfo(const JobInfo& job_info);
    JobManager();
    ~JobManager();
private:
    bool SaveToNexus(const Job* job);
    std::string BuildFsmKey(const JobStatus& status,
                            const JobEvent& event);
    FsmTrans* BuildFsmValue(const JobStatus& status,
                            const TransFunc& func);
    void BuildFsm();
    void BuildDispatch();
    void BuildAging();
    Status CheckPending(Job* job);
    Status CheckRunning(Job* job);
    Status CheckUpdating(Job* job);
    Status CheckDestroying(Job* job);
    Status CheckClear(Job* job);
    CheckJobStatus CheckJobStatus();
    Status CheckPodAlive(PodInfo* pod, Job* job);
    


private:
    std::map<std::string, Job*> jobs_;
    // agent some custom settings eg mark agent offline
    ThreadPool job_checker_;
    ThreadPool pod_checker_;
    Mutex mutex_;
    RpcClient rpc_client_;
    // nexus
    ::galaxy::ins::sdk::InsSDK* nexus_;
    //job fsm
    typedef boost::function<bool (Job* job, void* arg)> TransFunc;
    struct FsmTrans {
        JobStatus next_status_;
        TransFunc trans_func_;
    };
    typedef std::map<std::string, FsmTrans*> FSM;
    FSM fsm_;
    //job process
    typedef boost::function<Status (Job* job, void*)> DispatchFunc;
    typedef boost::function<Status (Job* job)> AgingFunc;
    std::map<JobStatus, DispatchFunc> dispatch_;
    std::map<JobStatus, AgingFunc> aging_;
};

}
}

#endif
