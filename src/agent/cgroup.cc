// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/cgroup.h"

#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "common/logging.h"
#include "common/util.h"
namespace galaxy {

int CGroupCtrl::Create(int64_t task_id, std::map<std::string, std::string>& sub_sys_map) {
    if (_support_cg.size() <= 0) {
        LOG(WARNING, "no subsystem is support");
        return -1;
    }

    std::vector<std::string>::iterator it = _support_cg.begin();

    for (; it != _support_cg.end(); ++it) {
        std::stringstream ss ;
        ss << _cg_root << "/" << *it << "/" << task_id;
        int status = mkdir(ss.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if (status != 0) {
            LOG(FATAL, "fail to create subsystem %s ,status is %d", ss.str().c_str(), status);
            return status;
        }

        sub_sys_map[*it] = ss.str();
        LOG(INFO, "create subsystem %s successfully", ss.str().c_str());
    }

    return 0;
}


//目前不支持递归删除
//删除前应该清空tasks文件
int CGroupCtrl::Destroy(int64_t task_id) {
    if (_support_cg.size() <= 0) {
        LOG(WARNING, "no subsystem is support");
        return -1;
    }

    std::vector<std::string>::iterator it = _support_cg.begin();

    for (; it != _support_cg.end(); ++it) {
        std::stringstream ss ;
        ss << _cg_root << "/" << *it << "/" << task_id;
    }

    return 0;
}

int AbstractCtrl::AttachTask(pid_t pid) {
    std::string task_file = _my_cg_root + "/" + "tasks";
    int ret = common::util::WriteIntToFile(task_file, pid);

    if (ret < 0) {
        LOG(FATAL, "fail to attach pid  %d for %s", pid, _my_cg_root);
        return -1;
    }

    return 0;
}

int MemoryCtrl::SetLimit(int64_t limit) {
    std::string limit_file = _my_cg_root + "/" + "memory.limit_in_bytes";
    int ret = common::util::WriteIntToFile(limit_file, limit);

    if (ret < 0) {
        LOG(FATAL, "fail to set limt %lld for %s", limit, _my_cg_root.c_str());
        return -1;
    }

    return 0;
}

int MemoryCtrl::SetSoftLimit(int64_t soft_limit) {
    std::string soft_limit_file = _my_cg_root + "/" + "memory.soft_limit_in_bytes";
    int ret = common::util::WriteIntToFile(soft_limit_file, soft_limit);

    if (ret < 0) {
        LOG(FATAL, "fail to set soft limt %lld for %s", soft_limit, _my_cg_root);
        return -1;
    }

    return 0;
}


int CpuCtrl::SetCpuShare(int64_t cpu_share) {
    std::string cpu_share_file = _my_cg_root + "/" + "cpu.shares";
    int ret = common::util::WriteIntToFile(cpu_share_file, cpu_share);

    if (ret < 0) {
        LOG(FATAL, "fail to set cpu share %lld for %s", cpu_share, _my_cg_root);
        return -1;
    }

    return 0;

}
int SetCpuPeriod(int64_t cpu_period) {
    return 0;
}

int SetCpuQuota(int64_t cpu_quota) {
    return 0;
}

int ContainerTaskRunner::Prepare() {
    LOG(INFO, "prepare container for task %d", m_task_info.task_id());
    //TODO
    std::vector<std::string> support_cg;
    support_cg.push_back("memory");
    support_cg.push_back("cpu");
    _cg_ctrl = new CGroupCtrl(_cg_root, support_cg);
    std::map<std::string, std::string> sub_sys_map;
    int status = _cg_ctrl->Create(m_task_info.task_id(), sub_sys_map);

    if (status != 0) {
        LOG(FATAL, "fail to create subsystem for task %d,status %d", m_task_info.task_id(), status);
        return status;
    }

    _mem_ctrl = new MemoryCtrl(sub_sys_map["memory"]);
    _cpu_ctrl = new CpuCtrl(sub_sys_map["cpu"]);
    return 0;
}


int ContainerTaskRunner::Start() {
    LOG(INFO, "start a task with id %d", m_task_info.task_id());

    if (IsRunning() == 0) {
        LOG(WARNING, "task with id %d has been runing", m_task_info.task_id());
        return -1;
    }

    int stdout_fd, stderr_fd;
    std::vector<int> fds;
    PrepareStart(fds, &stdout_fd, &stderr_fd);
    m_child_pid = fork();

    if (m_child_pid == 0) {
        _mem_ctrl->SetLimit(1073741824);
        _cpu_ctrl->SetCpuShare(1000000);
        pid_t my_pid = getpid();
        _mem_ctrl->AttachTask(my_pid);
        _cpu_ctrl->AttachTask(my_pid);
        StartTaskAfterFork(fds, stdout_fd, stderr_fd);
    } else {
        close(stdout_fd);
        close(stderr_fd);
        return 0;
    }
}

int ContainerTaskRunner::ReStart() {
    return 0;
}
}
