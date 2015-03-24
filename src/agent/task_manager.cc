// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/task_manager.h"

#include "common/logging.h"

namespace galaxy {
int TaskManager::Add(const ::galaxy::TaskInfo& task_info,
        const DefaultWorkspace& workspace) {
    LOG(INFO, "add task with id %d", task_info.task_id());
    m_mutex->Lock();

    if (m_task_runner_map.find(task_info.task_id()) != m_task_runner_map.end()) {
        m_mutex->Unlock();
        LOG(WARNING, "task with id %d has exist", task_info.task_id());
        return 0;
    }

    TaskRunner* runner = new CommandTaskRunner(task_info, workspace);
    int ret = runner->Start();

    if (ret == 0) {
        LOG(INFO, "add task with id %d successfully", task_info.task_id());
        m_task_runner_map[task_info.task_id()] = runner;
    } else {
        LOG(FATAL, "fail to add task with %d", task_info.task_id());
    }

    m_mutex->Unlock();
    return ret;
}

int TaskManager::Remove(const TaskInfo& task_info) {
    return 0;
}
int TaskManager::Status(std::vector< TaskStatus >& task_status_vector) {
    std::map<int64_t, TaskRunner*>::iterator it = m_task_runner_map.begin();

    for (; it != m_task_runner_map.end(); ++it) {
        TaskStatus status;
        status.set_task_id(it->first);
        status.set_status(it->second->IsRunning());
        task_status_vector.push_back(status);
    }

    return 0;
}
}



