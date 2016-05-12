// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef  BAIDU_GALAXY_POD_MANAGER_H
#define  BAIDU_GALAXY_POD_MANAGER_H

#include <thread_pool.h>
#include <mutex.h>

#include "protocol/appmaster.pb.h"
#include "task_manager.h"

namespace baidu {
namespace galaxy {

typedef proto::PodInfo PodInfo;
typedef proto::TaskDescription TaskDescription;

class PodManager {
public:
    PodManager();
    ~PodManager();
    void RunPod(const PodInfo* pod_info);
    void KillPod(const PodInfo* pod_info);
    void ShowPod(PodInfo* pod_info);
    void LoopPodCheck();
    void LoopPodStatusChangeCheck();

private:
    int DeployPod();
    int ReadyPodCheck();
    int DeployingPodCheck();
    int StartingPodCheck();
    int RunningPodCheck();

private:
    Mutex mutex_pod_manager_;
    PodInfo* pod_;
    TaskManager* task_manager_;
    ThreadPool background_pool_; 
};

}
}


#endif  // BAIDU_GALAXY_POD_MANAGER_H
