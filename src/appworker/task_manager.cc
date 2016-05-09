// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_killer_thread_pool_size);
DECLARE_int32(task_manager_loop_wait_interval);

namespace baidu {
namespace galaxy {

TaskManager::TaskManager() :
        mutex_task_manager_(),
        background_pool_(FLAGS_task_manager_background_thread_pool_size),
        killer_pool_(FLAGS_task_manager_killer_thread_pool_size) {
    background_pool_.DelayTask(
        FLAGS_task_manager_loop_wait_interval,
        boost::bind(&TaskManager::LoopWait, this)
    );
}

TaskManager::~TaskManager() {
    background_pool_.Stop(false);
    killer_pool_.Stop(false);
}

void TaskManager::LoopWait() {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "loop check";
    background_pool_.DelayTask(
        FLAGS_task_manager_loop_wait_interval,
        boost::bind(&TaskManager::LoopWait, this)
    );
}


}   // ending namespace galaxy
}   // ending namespace baidu
