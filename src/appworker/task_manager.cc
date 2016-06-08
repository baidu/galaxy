// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_manager.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <timer.h>
#include "utils.h"

DECLARE_int32(task_manager_background_thread_pool_size);
DECLARE_int32(task_manager_task_stop_command_timeout);
DECLARE_int32(task_manager_task_check_command_timeout);
DECLARE_int32(task_manager_loop_wait_interval);
DECLARE_int32(task_manager_task_max_fail_retry_times);
DECLARE_string(appworker_default_user);

namespace baidu {
namespace galaxy {

int MakeProcessEnv(Task* task, ProcessEnv& env);

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
// for downloading exe package and data packages
int TaskManager::DeployTask(const std::string& task_id) {
    MutexLock lock(&mutex_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "deplay task start, task: " << task_id;
    Task* task = it->second;
    task->packages_size = 0;

    if (task->desc.has_exe_package()) {
        DownloadProcessContext context;
        context.process_id = task->task_id + "_deploy_0";
        context.src_path = task->desc.exe_package().package().source_path();
        context.dst_path = task->desc.exe_package().package().dest_path();
        context.version = task->desc.exe_package().package().version();
        context.work_dir = task->env.workspace_path;
        context.package = context.work_dir + "/" + context.process_id
                          + "." + context.version + ".tar.gz";

        // process env
        ProcessEnv env;
        MakeProcessEnv(task, env);

        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING) << "command execute fail, command: " << context.cmd;
            return -1;
        }

        task->packages_size++;
    }

    if (task->desc.has_data_package()) {
        for (int i = 0; i < task->desc.data_package().packages_size(); i++) {
            DownloadProcessContext context;
            context.process_id = task->task_id + "_deploy_"\
                                 + boost::lexical_cast<std::string>(i + 1);
            context.src_path = task->desc.data_package().packages(i).source_path();
            context.dst_path = task->desc.data_package().packages(i).dest_path();
            context.version = task->desc.data_package().packages(i).version();
            context.work_dir = task->env.workspace_path;
            context.package = context.work_dir + "/" + context.process_id
                              + "." + context.version + ".tar.gz";

            ProcessEnv env;
            MakeProcessEnv(task, env);

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
int TaskManager::DoStartTask(const std::string& task_id) {
    LOG(INFO) << "start task: " << task_id;
    mutex_.AssertHeld();
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create main process for task: " << task_id;
    Task* task = it->second;

    if (task->desc.has_exe_package()) {
        ProcessContext context;
        context.process_id = task->task_id + "_main";
        context.cmd = task->desc.exe_package().start_cmd();
        context.work_dir = task->env.workspace_path;

        ProcessEnv env;
        MakeProcessEnv(task, env);

        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING) << "command execute fail, command: " << context.cmd;
            return -1;
        }
    }

    task->status = proto::kTaskRunning;
    return 0;

}

int TaskManager::StartTask(const std::string& task_id) {
    MutexLock loc(&mutex_);
    return DoStartTask(task_id);
}

int TaskManager::StopTask(const std::string& task_id) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "stop task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create stop process for task: " << task_id;
    Task* task = it->second;
    // save current status, for cleaning
    task->prev_status = task->status;

    if (!task->desc.has_exe_package()) {
        return -1;
    }

    task->timeout_point = common::timer::now_time()\
                          + FLAGS_task_manager_task_stop_command_timeout;
    ProcessContext context;
    context.process_id = task->task_id + "_stop";
    context.cmd = task->desc.exe_package().stop_cmd();
    context.work_dir = task->env.workspace_path;

    ProcessEnv env;
    MakeProcessEnv(task, env);

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
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    // task status switch
    std::string process_id;
    Process process;

    switch (it->second->status) {
    case proto::kTaskDeploying: {
        ProcessStatus process_status = proto::kProcessFinished;

        for (int i = 0; i < it->second->packages_size; i++) {
            process_id = task_id + "_deploy_" + boost::lexical_cast<std::string>(i);

            if (0 != process_manager_.QueryProcess(process_id, process)) {
                LOG(WARNING) << "query deploy process: " << process_id << " fail";
                it->second->status = proto::kTaskFailed;
                return -1;
            }

            if (process.status > proto::kProcessFinished) {
                LOG(WARNING) << "deploy process " << process_id << " failed";
                it->second->status = proto::kTaskFailed;
                process_status = process.status;
                break;
            }

            if (process.status < process_status) {
                process_status = process.status;
            }
        }

        if (process_status == proto::kProcessFinished) {
            LOG(INFO) << "deploy all processes successed";
            it->second->status = proto::kTaskStarting;
        }

        break;
    }

    case proto::kTaskRunning: {
        process_id = task_id + "_main";

        if (0 != process_manager_.QueryProcess(process_id, process)) {
            LOG(WARNING) << "query main process: " << process_id << " fail";
            return -1;
        }

        do {
            if (process.status == proto::kProcessRunning) {
                break;
            }

            if (0 == process.exit_code) {
                it->second->status = proto::kTaskFinished;
                LOG(INFO) << "main process finished successful";
                break;
            }

            if (it->second->fail_retry_times < FLAGS_task_manager_task_max_fail_retry_times) {
                LOG(INFO) << "task: " << task_id << " failed and retry";
                it->second->fail_retry_times++;
                DoStartTask(task_id);
                break;
            }

            LOG(INFO) << "task: " << task_id << " failed no retry";
            it->second->status = proto::kTaskFailed;
        } while (0);

        break;
    }

    case proto::kTaskStopping: {
        process_id = task_id + "_stop";

        if (0 != process_manager_.QueryProcess(process_id, process)) {
            LOG(WARNING) << "query stop process: " << process_id << " fail";
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
    task.fail_retry_times = it->second->fail_retry_times;

    return 0;
}

int TaskManager::CleanTask(const std::string& task_id) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "clean task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    std::string process_id;

    switch (it->second->prev_status) {
    case proto::kTaskDeploying:
        LOG(INFO) << "clean deploying task";

        for (int i = 0; i < it->second->packages_size; i++) {
            process_id = task_id + "_deploy_"\
                         + boost::lexical_cast<std::string>(i);
            process_manager_.KillProcess(process_id);
        }

        break;

    case proto::kTaskRunning:
        LOG(INFO) << "clean running task";
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
    MutexLock lock(&mutex_);
    LOG(INFO) << "clear all tasks";
    std::map<std::string, Task*>::iterator it = tasks_.begin();

    for (; it != tasks_.end(); ++it) {

        if (NULL != it->second) {
            delete it->second;
        }

        tasks_.erase(it);
    }

    process_manager_.ClearProcesses();

    return 0;
}

int TaskManager::DeployReloadTask(const std::string& task_id,
                                  const TaskDescription& task_desc) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "reload deploy task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    // 1.replace desc
    Task* task = it->second;
    task->desc.CopyFrom(task_desc);
    LOG(INFO) << "task desc replaced ok, task: " << task_id;

    // 2.deploy data packages
    LOG(INFO) << "reload deplay task start, task: " << task_id;
    task->packages_size = 0;

    if (task->desc.has_data_package()) {
        for (int i = 0; i < task->desc.data_package().packages_size(); i++) {
            DownloadProcessContext context;
            context.process_id = task->task_id + "_reload_deploy_"\
                                 + boost::lexical_cast<std::string>(i + 1);
            context.src_path = task->desc.data_package().packages(i).source_path();
            context.dst_path = task->desc.data_package().packages(i).dest_path();
            context.version = task->desc.data_package().packages(i).version();
            context.work_dir = task->env.workspace_path;
            context.package = context.work_dir + "/" + context.process_id
                                  + "." + context.version +  ".tar.gz";

            // process env
            ProcessEnv env;
            MakeProcessEnv(task, env);

            if (0 != process_manager_.CreateProcess(env, &context)) {
                LOG(WARNING)
                        << "command execute fail, "
                        << "command: " << context.cmd << ", "
                        << "dir: " << context.work_dir;
                return -1;
            }
        }
    }

    task->reload_status = proto::kTaskDeploying;

    return 0;
}

int TaskManager::StartReloadTask(const std::string& task_id) {
    MutexLock lock(&mutex_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create reload main process for task: " << task_id;
    Task* task = it->second;

    if (task->desc.has_data_package()) {
        ProcessContext context;
        context.process_id = task->task_id + "_reload_main";
        context.cmd = task->desc.data_package().reload_cmd();
        context.work_dir = task->env.workspace_path;

        ProcessEnv env;
        MakeProcessEnv(task, env);

        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING)
                    << "command execute fail, "
                    << "command: " << context.cmd << ", "
                    << "dir: " << context.work_dir;
            return -1;
        }

        task->reload_status = proto::kTaskRunning;
    } else {
        task->reload_status = proto::kTaskFinished;
    }

    return 0;
}

// check reload task status
int TaskManager::CheckReloadTask(const std::string& task_id,
                                 Task& task) {
    MutexLock lock(&mutex_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    std::string process_id;
    Process process;

    switch (it->second->reload_status) {
    case proto::kTaskDeploying: {
        ProcessStatus process_status = proto::kProcessFinished;

        for (int i = 0; i < it->second->packages_size; i++) {
            process_id = task_id + "_reload_deploy_" + boost::lexical_cast<std::string>(i + 1);

            if (0 != process_manager_.QueryProcess(process_id, process)) {
                LOG(WARNING) << "query deploy process: " << process_id << " fail";
                it->second->reload_status = proto::kTaskFailed;
                return -1;
            }

            if (process.status > proto::kProcessFinished) {
                LOG(WARNING) << "deploy process " << process_id << " failed";
                it->second->reload_status = proto::kTaskFailed;
                process_status = process.status;
                break;
            }

            if (process.status < process_status) {
                process_status = process.status;
            }
        }

        if (process_status == proto::kProcessFinished) {
            LOG(WARNING) << "deploy all processes successed";
            it->second->reload_status = proto::kTaskStarting;
        }

        break;
    }

    case proto::kTaskRunning: {
        process_id = task_id + "_reload_main";

        if (0 != process_manager_.QueryProcess(process_id, process)) {
            LOG(WARNING) << "query reload main process: " << process_id << " fail";
            return -1;
        }

        do {
            if (process.status == proto::kProcessRunning) {
                break;
            }

            if (0 == process.exit_code) {
                it->second->reload_status = proto::kTaskFinished;
                LOG(INFO) << "main process finished successful";
                break;
            }

            it->second->reload_status = proto::kTaskFailed;
        } while (0);

        break;
    }

    default:
        break;
    }

    task.reload_status = it->second->reload_status;

    return 0;
}

int TaskManager::StartTaskHealthCheck(const std::string& task_id) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "start task health check, task: " << task_id;
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "start create health check process for task: " << task_id;
    Task* task = it->second;

    if (task->desc.has_exe_package()
            && task->desc.exe_package().has_check_cmd()) {
        ProcessContext context;
        context.process_id = task->task_id + "_check";
        context.cmd = task->desc.exe_package().check_cmd();
        context.work_dir = task->env.workspace_path;

        ProcessEnv env;
        MakeProcessEnv(task, env);

        if (0 != process_manager_.CreateProcess(env, &context)) {
            LOG(WARNING) << "command execute fail, command: " << context.cmd;
            return -1;
        }

        task->check_timeout_point = common::timer::now_time()\
                                    + FLAGS_task_manager_task_check_command_timeout;
    }

    return 0;
}

int TaskManager::CheckTaskHealth(const std::string& task_id, Task& task) {
    MutexLock lock(&mutex_);
    std::map<std::string, Task*>::iterator it = tasks_.find(task_id);

    if (it == tasks_.end()) {
        LOG(WARNING) << "task: " << task_id << " not exist";
        return -1;
    }

    LOG(INFO) << "check health task, task: " << task_id;

    if (!(it->second->desc.has_exe_package()
            && it->second->desc.exe_package().has_check_cmd())) {
        task.status = proto::kTaskFinished;
        return 0;
    }

    std::string process_id = task_id + "_check";
    Process process;

    if (0 != process_manager_.QueryProcess(process_id, process)) {
        LOG(WARNING) << "query health process: " << process_id << " fail";
        return -1;
    }

    if (process.status == proto::kProcessRunning) {
        int32_t now_time = common::timer::now_time();
        if (it->second->check_timeout_point < now_time) {
            LOG(INFO) << "check command timeout";
            process_manager_.KillProcess(process_id);
            task.status = proto::kTaskFailed;
        } else {
            task.status = proto::kTaskRunning;
        }
    } else {
        if (0 == process.exit_code) {
            task.status = proto::kTaskFinished;
            LOG(INFO) << "check process finished";
        } else {
            task.status = proto::kTaskFailed;
            LOG(INFO) << "check process failed";
        }
    }

    return 0;
}

int TaskManager::ClearTaskHealthCheck(const std::string& task_id) {
    MutexLock lock(&mutex_);

    LOG(INFO) << "clear task health check, task: " << task_id;
    std::string process_id = task_id + "_check";
    process_manager_.KillProcess(process_id);

    return 0;
}

int MakeProcessEnv(Task* task, ProcessEnv& env) {
    env.user = task->env.user;
    env.envs.push_back("GALAXY_JOB_ID=" + task->env.job_id);
    env.envs.push_back("GALAXY_POD_ID=" + task->env.pod_id);
    env.envs.push_back("GALAXY_TASK_ID=" + task->env.task_id);
    env.envs.push_back("GALAXY_ABS_PATH=" + task->env.workspace_abspath);
    std::map<std::string, std::string>::iterator p_it = task->env.ports.begin();

    for (; p_it != task->env.ports.end(); ++p_it) {
        LOG(INFO) << "GALAXY_PORT_" + p_it->first + "=" + p_it->second;
        env.envs.push_back("GALAXY_PORT_" + p_it->first + "=" + p_it->second);
    }

    std::map<std::string, std::string>::iterator c_it = \
            task->env.cgroup_paths.begin();

    for (; c_it != task->env.cgroup_paths.end(); ++c_it) {
        env.cgroup_paths.push_back(c_it->second);
    }

    return 0;
}

}   // ending namespace galaxy
}   // ending namespace baidu
