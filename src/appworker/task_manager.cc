// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_killer_thread_pool_size);
DECLARE_int32(task_manager_zombie_check_interval);

namespace baidu {
namespace galaxy {

TaskManager::TaskManager() :
    mutex_task_manager_(),
    background_thread_pool_(FLAGS_task_manager_background_thread_pool_size),
    killer_thread_pool_(FLAGS_task_manager_killer_thread_pool_size) {
    LOG(INFO) << "task manager start.";

    background_thread_pool_.DelayTask(
            FLAGS_task_manager_zombie_check_interval,
            boost::bind(&TaskManager::ZombieCheck, this));
}

TaskManager::~TaskManager() {
    background_thread_pool_.Stop(false);
    killer_thread_pool_.Stop(false);
}

int TaskManager::ZombieCheck() {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "zombie check called in task manager";
    return 0;
}

//int TaskManager::CreateTask(proto::TaskInfo) {
//    return 0;
//}

}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */

