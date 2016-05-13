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

typedef proto::TaskDescription TaskDescription;
typedef proto::TaskStatus TaskStatus;
typedef proto::ProcessStatus ProcessStatus;

struct Process {
    std::string process_id;
    int32_t pid;
    ProcessStatus status;
    int32_t exit_code;
};

struct Task {
    TaskDescription desc;
    TaskStatus status;
    TaskCollector collector;
    Process deploy_process;
    Process main_process;
    Process stop_process;
};

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    int CreateTask(const TaskDescription* task_desc);
    int DeleteTask(const std::string& task_id);
    void LoopWait();
    int GetProcessInfo(Process& process);

private:
    Mutex mutex_task_manager_;
    std::map<std::string, Task*> tasks_;
    ThreadPool background_pool_;
    ThreadPool killer_pool_;
};


}   // ending namespace galaxy
}   // ending namespace baidu


#endif  // BAIDU_GALAXY_TASK_MANAGER_H
