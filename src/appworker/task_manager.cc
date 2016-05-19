// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_killer_thread_pool_size);
DECLARE_int32(task_manager_loop_wait_interval);
DECLARE_int32(task_manager_task_max_fail_retry_times);

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
    MutexLock lock(&mutex_task_manager_);
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

// create all task deploy processes,
// for downloading image package and data packages
int TaskManager::DeployTask(const std::string& task_id) {
    MutexLock lock(&mutex_task_manager_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "deplay task start, task: " << task_id;
    Task* task = it->second;
    task->packages_size = 0;
    if (task->desc.has_exe_package()) {
        DownloadProcessContext context;
        context.process_id = task->task_id + "_deploy_0";
        context.src_path = task->desc.exe_package().package().src_path();
        context.dst_path = "image.tar.gz";
        context.version = task->desc.exe_package().package().version();
        context.work_dir = task->task_id;
        context.cmd = "wget -O " + context.dst_path + " " + context.src_path;
        if (0 != process_manager_.CreateProcess(&context)) {
            LOG(WARNING) << "command execute fail, command: " << context.cmd;
            return -1;
        }
        task->packages_size++;
    }
    if (task->desc.has_data_package()) {
        for (int i = 0; i< task->desc.data_package().packages_size(); i++) {
            DownloadProcessContext context;
            context.process_id = task->task_id + "_deploy_" + boost::lexical_cast<std::string>(i + 1);
            context.src_path = task->desc.data_package().packages(i).src_path();
            context.dst_path = task->desc.data_package().packages(i).dst_path();
            context.version = task->desc.data_package().packages(i).version();
            context.work_dir = task->task_id;
            context.cmd = "wget -O " + context.dst_path + " " + context.src_path;
            if (0 != process_manager_.CreateProcess(&context)) {
                LOG(WARNING) << "command execute fail, command: " << context.cmd;
                return -1;
            }
            task->packages_size++;
        }
    }
    task->status = proto::kTaskDeploying;
    return 0;
}

// create task main process
int TaskManager::StartTask(const std::string& task_id) {
    LOG(INFO) << "start task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create main process for task: " << task_id;
    Task* task = it->second;
    if (task->desc.has_exe_package()) {
        ProcessContext context;
        context.process_id = task->task_id + "_main";
        context.cmd = task->desc.exe_package().start_cmd();
        context.work_dir = task->task_id;
        process_manager_.DeleteProcess(context.process_id);
        if (0 != process_manager_.CreateProcess(&context)) {
            LOG(WARNING) << "command execute fail,command: " << context.cmd;
            return -1;
        }
    }
    task->status = proto::kTaskRunning;
    return 0;
}

int TaskManager::CheckTask(const std::string& task_id, Task& task) {
    MutexLock lock(&mutex_task_manager_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id << " not exist";
        return -1;
    }

    // task status switch
    std::string process_id;
    Process process;
    switch (it->second->status) {
    case proto::kTaskDeploying:
    {
        ProcessStatus process_status = proto::kProcessFinished;
        for (int i = 0; i < it->second->packages_size; i++) {
            process_id = task_id + "_deploy_" + boost::lexical_cast<std::string>(i);
            if (0 != process_manager_.QueryProcess(process_id, process)) {
                LOG(INFO) << "query deploy process: " << process_id << " fail";
                it->second->status = proto::kTaskFailed;
                return -1;
            }
            if (process.status > proto::kProcessFinished) {
                LOG(INFO) << "deploy process " << process_id <<" failed";
                it->second->status = proto::kTaskFailed;
                break;
            }
            if (process.status < process_status) {
                process_status = process.status;
            }
        }
        if (process_status == proto::kProcessFinished) {
            LOG(INFO) << "deploy task ok, all deploy processes successed";
            it->second->status = proto::kTaskStarting;
        }
        break;
    }
    case proto::kTaskRunning:
    {
        process_id = task_id + "_main";
        if (0 != process_manager_.QueryProcess(process_id, process)) {
            LOG(INFO) << "query main process: " << process_id << " fail";
            return -1;
        }
        if (process.status != proto::kProcessRunning) {
            if (0 == process.exit_code) {
                it->second->status = proto::kTaskFinished;
                LOG(INFO) << "main process finished successful";
            } else {
                 if (it->second->fail_retry_times < FLAGS_task_manager_task_max_fail_retry_times) {
                     LOG(INFO) << "task: " << task_id << " failed and retry";
                     it->second->fail_retry_times++;
                     StartTask(task_id);
                 } else {
                     LOG(INFO) << "task: " << task_id << " failed no retry";
                     it->second->status = proto::kTaskFailed;
                 }
            }
        }
        break;
     }
     default:
         break;
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
