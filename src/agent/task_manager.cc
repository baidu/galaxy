// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/task_manager.h"

#include "common/logging.h"

namespace galaxy {
int TaskManager::Add(const ::galaxy::TaskInfo& task_info,
                     DefaultWorkspace *  workspace) {
    MutexLock lock(m_mutex);
    LOG(INFO, "add task with id %d", task_info.task_id());
    if (m_task_runner_map.find(task_info.task_id()) != m_task_runner_map.end()) {
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
    return ret;
}

int TaskManager::Remove(const int64_t& task_info_id) {
    MutexLock lock(m_mutex);
    if (m_task_runner_map.find(task_info_id) == m_task_runner_map.end()) {
        LOG(WARNING, "task with id %d does not exist", task_info_id);
        return 0;
    }
    TaskRunner* runner = m_task_runner_map[task_info_id];
    if(NULL == runner){
        return 0;
    }
    int status = runner->Stop();
    if(status == 0){
        LOG(INFO,"stop task %d successfully",task_info_id);
    }
    m_task_runner_map.erase(task_info_id);
    delete runner;
    return status;
}

int TaskManager::Status(std::vector< TaskStatus >& task_status_vector) {
    std::map<int64_t, TaskRunner*>::iterator it = m_task_runner_map.begin();
    for (; it != m_task_runner_map.end(); ++it) {
        TaskStatus status;
        status.set_task_id(it->first);
        if(it->second->IsRunning() == 0){
            status.set_status(RUNNING);
        }else{
            status.set_status(ERROR);
        }

        task_status_vector.push_back(status);
    }
    return 0;
}
}



