// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/workspace_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "common/logging.h"

namespace galaxy {

int WorkspaceManager::Add(const TaskInfo& task_info) {
    MutexLock lock(m_mutex);
    TaskInfo my_task_info(task_info);
    if (m_workspace_map.find(my_task_info.task_id()) != m_workspace_map.end()) {
        return 0;
    }

    DefaultWorkspace* ws = new DefaultWorkspace(my_task_info, m_data_path);
    int status = ws->Create();

    if (status == 0) {
        m_workspace_map[my_task_info.task_id()] = ws;
    }

    used_cpu_share += my_task_info.required_cpu();
    used_mem_share += my_task_info.required_mem();
    LOG(INFO, "used_cpu : %lf, used_mem : %d", used_cpu_share, used_mem_share);
    return status;
}

bool WorkspaceManager::Init() {
    const int MKDIR_MODE = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    // clear work_dir and kill tasks
    std::string dir = m_root_path + "/data";
    m_data_path = dir;
    if (access(m_root_path.c_str(), F_OK) != 0) {
        if (mkdir(m_root_path.c_str(), MKDIR_MODE) != 0) {
            LOG(WARNING, "mkdir data failed %s err[%d: %s]",
                    m_root_path.c_str(), errno, strerror(errno));
            return false;
        }
        if (mkdir(m_data_path.c_str(), MKDIR_MODE) != 0) {
            LOG(WARNING, "mkdir data failed %s err[%d: %s]",
                    m_data_path.c_str(), errno, strerror(errno));
            return false;
        }
        LOG(INFO, "init workdir %s", dir.c_str());
        return true;
    }

    if (access(dir.c_str(), F_OK) == 0) {
        std::string rm_cmd = "rm -rf " + dir;
        if (system(rm_cmd.c_str()) == -1) {
            LOG(WARNING, "rm data failed cmd %s err[%d: %s]",
                    rm_cmd.c_str(), errno, strerror(errno));
            return false;
        }
        LOG(INFO, "clear dirty data %s by cmd[%s]", dir.c_str(), rm_cmd.c_str());
    }

    if (mkdir(dir.c_str(), MKDIR_MODE) != 0) {
        LOG(WARNING, "mkdir data failed %s err[%d: %s]",
                dir.c_str(), errno, strerror(errno));
        return false;
    }
    LOG(INFO, "init workdir %s", dir.c_str());
    return true;
}

int WorkspaceManager::Remove(int64_t task_info_id) {
    MutexLock lock(m_mutex);
    if (m_workspace_map.find(task_info_id) == m_workspace_map.end()) {
        return 0;
    }

    Workspace* ws = m_workspace_map[task_info_id];

    if (ws != NULL) {
        used_cpu_share -= ws->GetTaskInfo().required_cpu();
        used_mem_share -= ws->GetTaskInfo().required_mem();

        int status =  ws->Clean();
        if (status != 0) {
            return status;
        }

        m_workspace_map.erase(task_info_id);
        delete ws;
        return 0;
    }
    return -1;

}

DefaultWorkspace* WorkspaceManager::GetWorkspace(const TaskInfo& task_info) {
    if (m_workspace_map.find(task_info.task_id()) == m_workspace_map.end()) {
        return NULL;
    }

    return m_workspace_map[task_info.task_id()];

}

double WorkspaceManager::GetUsedCpuShare() {
    return used_cpu_share;
}

int64_t WorkspaceManager::GetUsedMemShare() {
    return used_mem_share;
}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
