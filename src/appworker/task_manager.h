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
#include "process_manager.h"

namespace baidu {
namespace galaxy {

typedef proto::TaskDescription TaskDescription;
typedef proto::TaskStatus TaskStatus;
typedef proto::Package Package;
typedef proto::ProcessStatus ProcessStatus;

struct Task {
    std::string task_id;
    TaskDescription desc;
    TaskStatus status;
};

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    int CreateTask(const std::string& task_id, const TaskDescription& task_desc);
    int DeleteTask(const std::string& task_id);
    int DeployTask(const std::string& task_id);
    int StartTask(const std::string& task_id);
    int CheckTask(const std::string& task_id, Task& task);
    int ClearTasks();

private:
    Mutex mutex_task_manager_;
    std::map<std::string, Task*> tasks_;
    ProcessManager process_manager_;
    ThreadPool background_pool_;
    ThreadPool killer_pool_;
};

}   // ending namespace galaxy
}   // ending namespace baidu


#endif  // BAIDU_GALAXY_TASK_MANAGER_H
