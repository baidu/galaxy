// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_TASK_MANAGER_H
#define BAIDU_GALAXY_TASK_MANAGER_H

#include <string>
#include <map>

#include <mutex.h>
#include <thread_pool.h>

#include "protocol/galaxy.pb.h"
#include "task_collector.h"

namespace baidu {
namespace galaxy {

typedef proto::TaskInfo TashInfo;

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    int CreateTask(const TaskInfo& task_info);
    int DeleteTask(const TaskInfo& task_info);
    void LoopWait();

private:
    Mutex mutex_task_manager_;
    std::map<std::string, TaskInfo*> tasks_;
    std::map<std::string, TaskCollector*> collectors_;
    ThreadPool background_pool_;
    ThreadPool killer_pool_;
};


}   // ending namespace galaxy
}   // ending namespace baidu


#endif  // BAIDU_GALAXY_TASK_MANAGER_H
