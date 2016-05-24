// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef  BAIDU_GALAXY_POD_MANAGER_H
#define  BAIDU_GALAXY_POD_MANAGER_H

#include <map>
#include <string>

#include <thread_pool.h>
#include <mutex.h>

#include "protocol/appmaster.pb.h"
#include "task_manager.h"

namespace baidu {
namespace galaxy {

typedef proto::PodDescription PodDescription;
typedef proto::PodStatus PodStatus;

struct PodEnv {
    std::string job_id;
    std::string pod_id;
    std::vector<std::string> task_ids;
    std::vector<std::string> cgroup_subsystems;
    std::vector<std::map<std::string, std::string> > task_cgroup_paths;
};

/**
 * when pod.stage == kPodTerminating,
 * and pod.status == kPodTerminated,
 * lset pod.status = kPodPending
 */
enum PodStage {
    kPodStageCreating,
    kPodStageRebuilding,
    kPodStageReloading,
    kPodStageTerminating
};

struct Pod {
    std::string pod_id;
    PodStatus status;
    PodStatus reload_status;
    PodStage stage;
    PodEnv env;
    PodDescription desc;
};

class PodManager {
public:
    PodManager();
    ~PodManager();
    int SetPodEnv(const PodEnv& pod_env);
    int SetPodDescription(const PodDescription& pod_desc);
    int TerminatePod();
    int RebuildPod();
    int ReloadPod();
    int QueryPod(Pod& pod);

private:
    int DoCreatePod();
    int DoDeployPod();
    int DoStartPod();
    int DoStopPod();
    int DoClearPod();

    int DoReloadDeployPod();
    int DoReloadStartPod();

    void LoopChangePodStatus();
    void LoopChangePodReloadStatus();
    void PendingPodCheck();
    void ReadyPodCheck();
    void DeployingPodCheck();
    void StartingPodCheck();
    void StoppingPodCheck();
    void RunningPodCheck();
    void ReloadingPodCheck();

private:
    Mutex mutex_;
    Pod pod_;
    TaskManager task_manager_;
    ThreadPool background_pool_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif  // BAIDU_GALAXY_POD_MANAGER_H
