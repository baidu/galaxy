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
    PodDescription desc;
    PodStatus status;
    std::map<std::string, Task*> tasks;
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
    int DeployPod();
    int StartPod();
    int ReadyPodCheck();
    int DeployingPodCheck();
    int StartingPodCheck();
    int RunningPodCheck();

private:
    Mutex mutex_pod_manager_;
    Pod* pod_;
    TaskManager task_manager_;
    ThreadPool background_pool_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif  // BAIDU_GALAXY_POD_MANAGER_H
