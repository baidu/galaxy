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

class PodManager {
public:
    PodManager();
    ~PodManager();
    int RunPod(const PodInfo* pod_info);
    int KillPod(const PodInfo* pod_info);
    int ShowPod(PodInfo* pod_info);

private:
    void CheckPod();

private:
    Mutex mutex_pod_manager_;
    PodInfo* pod_;
    TaskManager* task_manager_;
    ThreadPool background_pool_; 
};

}
}


#endif  // BAIDU_GALAXY_POD_MANAGER_H
