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
}

TaskManager::~TaskManager() {
    background_pool_.Stop(false);
    killer_pool_.Stop(false);
}

int TaskManager::CreateTask(const std::string& task_id, const TaskDescription& task_desc) {
    Task* task = new Task();
    task->task_id = task_id;
    task->desc.CopyFrom(task_desc);
    task->status = proto::kTaskPending;
    tasks_.insert(std::make_pair(task_id, task));
    return 0;
}

int TaskManager::DeleteTask(const std::string& task_id) {
    return 0;
}

// create deploy process
int TaskManager::DeployTask(const std::string& task_id) {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "deplay task: " << task_id.c_str();
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id.c_str() << " not exist";
        return -1;
    }

    LOG(INFO) << "start create deploy process for task: " << task_id.c_str();
    Task* task = it->second;
    if (task->desc.has_exe_package()) {
        LOG(INFO) << task_id.c_str() << " has exe_package";
        std::string cmd = "wget -O image.tar.gz ";
        proto::Package* package = task->desc.mutable_exe_package()->mutable_package();
        cmd.append(package->source_path());
        std::string deploy_process_id = task->task_id + "_deploy";
        if (0 != process_manager_.CreateProcess(deploy_process_id, cmd)) {
            LOG(WARNING) << "command execute fail,command: " << cmd.c_str();
            return -1;
        }
    }

    task->status = proto::kTaskDeploying;
    return 0;
}

// create task main process
int TaskManager::StartTask(const std::string& task_id) {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "start task: " << task_id.c_str();
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id.c_str() << " not exist";
        return -1;
    }

    LOG(INFO) << "start create deploy process for task: " << task_id.c_str();
    Task* task = it->second;
    if (task->desc.has_exe_package()) {
        LOG(INFO) << task_id.c_str() << " has exe_package";
        std::string cmd = "wget -O image.tar.gz ";
        proto::Package* package = task->desc.mutable_exe_package()->mutable_package();
        cmd.append(package->source_path());
        std::string deploy_process_id = task->task_id + "_deploy";
        if (0 != process_manager_.CreateProcess(deploy_process_id, cmd)) {
            LOG(WARNING) << "command execute fail,command: " << cmd.c_str();
            return -1;
        }
    }
    return 0;
}

int TaskManager::CheckTask(const std::string& task_id, Task& task) {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "check task: " << task_id.c_str();
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id.c_str() << " not exist";
        return -1;
    }

    // task status switch
    Process process;
    std::string process_id;
    switch (it->second->status) {
        case proto::kTaskDeploying:
            process_id = task_id + "_deploy";
            if (0 != process_manager_.QueryProcess(process_id, process)) {
                LOG(INFO) << "query deploy process: " << process_id.c_str() << " fail";
                return -1;
            }
            if (process.status != proto::kProcessRunning) {
                if (0 == process.exit_code) {
                    it->second->status = proto::kTaskStarting;
                    LOG(INFO) << "deploy process finished successful";
                } else {
                    it->second->status = proto::kTaskFailed;
                }
            }
            break;
        case proto::kTaskRunning:
            process_id = task_id + "_main";
            if (0 != process_manager_.QueryProcess(process_id, process)) {
                LOG(INFO) << "query main process: " << process_id.c_str() << " fail";
                return -1;
            }
            if (process.status != proto::kProcessRunning) {
                if (0 == process.exit_code) {
                    it->second->status = proto::kTaskTerminated;
                    LOG(INFO) << "main process finished successful";
                } else {
                    it->second->status = proto::kTaskFailed;
                }
            }
            break;
         default:
             LOG(INFO) << "nothing to do";
     }

    task.task_id = it->second->task_id;
    task.desc.CopyFrom(it->second->desc);
    task.status = it->second->status;
    return 0;
}

int TaskManager::ClearTasks() {
    MutexLock lock(&mutex_task_manager_);
    LOG(INFO) << "clear all tasks";
    return 0;
}

}   // ending namespace galaxy
}   // ending namespace baidu
