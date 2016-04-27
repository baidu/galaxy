// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_TASK_MANAGER_H
#define BAIDU_GALAXY_TASK_MANAGER_H

#include <map>

#include <mutex.h>
#include <thread_pool.h>

#include "protocol/galaxy.pb.h"
#include "task_collector.h"

namespace baidu {
namespace galaxy {

class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    int CreateTask();
    int StopTask();
    int UpdateTask();

protected:
    Mutex mutex_task_infos_;
    std::map<std::string, proto::TaskInfo*> task_infos_;
    std::map<std::string, TaskCollector*> task_collectors_;
    ThreadPool background_thread_pool_;
    ThreadPool killer_thread_pool_;
};


}   // ending namespace galaxy
}   // ending namespace baidu


#endif  // BAIDU_GALAXY_TASK_MANAGER_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
