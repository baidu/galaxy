// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_killer_thread_pool_size);

namespace baidu {
namespace galaxy {

TaskManager::TaskManager() :
    mutex_task_infos_(),
    background_thread_pool_(FLAGS_task_manager_background_thread_pool_size),
    killer_thread_pool_(FLAGS_task_manager_killer_thread_pool_size) {
    LOG(INFO) << "task manager start.";
}

TaskManager::~TaskManager() {
    background_thread_pool_.Stop(false);
    killer_thread_pool_.Stop(false);
}

//int TaskManager::CreateTask(proto::TaskInfo) {
//    return 0;
//}

}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */

