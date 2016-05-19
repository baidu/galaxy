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

struct Pod {
    std::string pod_id;
    PodDescription desc;
    PodStatus status;
};

class PodManager {
public:
    PodManager();
    ~PodManager();
    void CreatePod(const PodDescription* pod_desc);
    void DeletePod();
    void LoopCheckPod();
    void LoopChangePodStatus();

private:
    int DoDeployPod();
    int DoStartPod();
    int DoStopPod();
    int DoCleanPod();
    void ReadyPodCheck();
    void DeployingPodCheck();
    void StartingPodCheck();
    void RunningPodCheck();
    void FailedPodCheck();

private:
    Mutex mutex_pod_manager_;
    Pod* pod_;
    TaskManager task_manager_;
    ThreadPool background_pool_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif  // BAIDU_GALAXY_POD_MANAGER_H
