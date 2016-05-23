// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <timer.h>

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_stop_command_timeout);
DECLARE_int32(task_manager_loop_wait_interval);
DECLARE_int32(task_manager_task_max_fail_retry_times);
DECLARE_string(appworker_default_user);

namespace baidu {
namespace galaxy {

TaskManager::TaskManager() :
    mutex_(),
    background_pool_(FLAGS_task_manager_background_thread_pool_size) {
}

TaskManager::~TaskManager() {
    background_pool_.Stop(false);
}

int TaskManager::CreateTask(const TaskEnv& task_env,
                            const TaskDescription& task_desc) {
    MutexLock lock(&mutex_);
    Task* task = new Task();
    task->env = task_env;
    task->task_id = task_env.task_id;
    task->desc.CopyFrom(task_desc);
    task->status = proto::kTaskPending;
    tasks_.insert(std::make_pair(task_env.task_id, task));
    return 0;
}

// create all task deploy processes,
// for downloading image package and data packages
int TaskManager::DeployTask(const std::string& task_id) {
    MutexLock lock(&mutex_);
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
        ProcessEnv env;
        env.user = FLAGS_appworker_default_user;
        env.envs.push_back("JOB_ID=" + task->env.job_id);
        env.envs.push_back("POD_ID=" + task->env.pod_id);
        env.envs.push_back("TASK_ID=" + task->env.task_id);
        std::map<std::string, std::string>::iterator c_it =\
            task->env.cgroup_paths.begin();
        for (; c_it != task->env.cgroup_paths.end(); ++c_it) {
            env.cgroup_paths.push_back(c_it->second);
        }
        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING) << "command execute fail, command: " << context.cmd;
            return -1;
        }
        task->packages_size++;
    }
    if (task->desc.has_data_package()) {
        for (int i = 0; i< task->desc.data_package().packages_size(); i++) {
            DownloadProcessContext context;
            context.process_id = task->task_id + "_deploy_"\
                + boost::lexical_cast<std::string>(i + 1);
            context.src_path = task->desc.data_package().packages(i).src_path();
            context.dst_path = task->desc.data_package().packages(i).dst_path();
            context.version = task->desc.data_package().packages(i).version();
            context.work_dir = task->task_id;
            context.cmd = "wget -O " + context.dst_path + " " + context.src_path;
            ProcessEnv env;
            env.user = FLAGS_appworker_default_user;
            env.envs.push_back("JOB_ID=" + task->env.job_id);
            env.envs.push_back("POD_ID=" + task->env.pod_id);
            env.envs.push_back("TASK_ID=" + task->env.task_id);
            if (0 != process_manager_.CreateProcess(env, &context)) {
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
        ProcessEnv env;
        env.user = FLAGS_appworker_default_user;
        env.envs.push_back("JOB_ID=" + task->env.job_id);
        env.envs.push_back("POD_ID=" + task->env.pod_id);
        env.envs.push_back("TASK_ID=" + task->env.task_id);
        std::map<std::string, std::string>::iterator c_it =\
            task->env.cgroup_paths.begin();
        for (; c_it != task->env.cgroup_paths.end(); ++c_it) {
            env.cgroup_paths.push_back(c_it->second);
        }
        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING) << "command execute fail,command: " << context.cmd;
            return -1;
        }
    }
    task->status = proto::kTaskRunning;
    return 0;
}

int TaskManager::StopTask(const std::string& task_id) {
    LOG(INFO) << "stop task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create stop process for task: " << task_id;
    Task* task = it->second;
    if (!task->desc.has_exe_package()) {
        return -1;
    }
    task->timeout_point = common::timer::now_time() + FLAGS_task_manager_stop_command_timeout;
    ProcessContext context;
    context.process_id = task->task_id + "_stop";
    context.cmd = task->desc.exe_package().stop_cmd();
    context.work_dir = task->task_id;
    ProcessEnv env;
    env.user = FLAGS_appworker_default_user;
    env.envs.push_back("JOB_ID=" + task->env.job_id);
    env.envs.push_back("POD_ID=" + task->env.pod_id);
    env.envs.push_back("TASK_ID=" + task->env.task_id);
    std::map<std::string, std::string>::iterator c_it =\
        task->env.cgroup_paths.begin();
    for (; c_it != task->env.cgroup_paths.end(); ++c_it) {
        env.cgroup_paths.push_back(c_it->second);
    }
    if (0 != process_manager_.CreateProcess(env, &context)) {
        LOG(WARNING) << "command execute fail, command: " << context.cmd;
        return -1;
    }
    task->status = proto::kTaskStopping;
    return 0;
}

int TaskManager::CheckTask(const std::string& task_id, Task& task) {
    MutexLock lock(&mutex_);
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
                process_status = process.status;
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
    case proto::kTaskStopping:
    {
        process_id = task_id + "_stop";
        if (0 != process_manager_.QueryProcess(process_id, process)) {
            LOG(INFO) << "query stop process: " << process_id << " fail";
            return -1;
        }
        if (process.status != proto::kProcessRunning) {
            return -1;
        } else {
            int32_t now_time = common::timer::now_time();
            if (it->second->timeout_point < now_time) {
                LOG(WARNING) << "stop process timeout";
                return -1;
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

int TaskManager::CleanTask(const std::string& task_id) {
    LOG(WARNING) << "clean task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        LOG(INFO) << "task: " << task_id << " not exist";
        return -1;
    }

    std::string process_id;
    switch (it->second->status) {
        case proto::kTaskDeploying:
            LOG(WARNING) << "clean deploying task";
            for (int i = 0; i < it->second->packages_size; i++) {
                process_id = task_id + "_deploy_" + boost::lexical_cast<std::string>(i);
                process_manager_.KillProcess(process_id);
            }
            break;
        case proto::kTaskRunning:
            LOG(WARNING) << "clean running task";
            process_id = task_id + "_main";
            process_manager_.KillProcess(process_id);
            break;
        default:
            break;
    }

    it->second->status = proto::kTaskTerminated;
    return 0;
}

int TaskManager::ClearTasks() {
    LOG(WARNING) << "clear all tasks";
    std::map<std::string, Task*>::iterator it = tasks_.begin();
    for (; it != tasks_.end(); ++it) {
       tasks_.erase(it);
    }
    process_manager_.ClearProcesses();
    return 0;
}

}   // ending namespace galaxy
}   // ending namespace baidu
