// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pod_manager.h"

#include "stdlib.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/appmaster.pb.h"

DECLARE_int32(pod_manager_check_pod_interval);
DECLARE_int32(pod_manager_change_pod_status_interval);

namespace baidu {
namespace galaxy {

PodManager::PodManager() :
    mutex_(),
    background_pool_(10) {

    pod_.status = proto::kPodPending;
    background_pool_.DelayTask(
        FLAGS_pod_manager_change_pod_status_interval,
        boost::bind(&PodManager::LoopChangePodStatus, this)
    );
}

PodManager::~PodManager() {
    background_pool_.Stop(false);
}

int PodManager::SetPodEnv(const PodEnv& pod_env) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "update pod env";
    pod_.env = pod_env;
    return 0;
}

int PodManager::SetPodDescription(const PodDescription& pod_desc) {
    MutexLock lock(&mutex_);
    LOG(INFO) << "update pod description";
    pod_.desc.CopyFrom(pod_desc);
    return 0;
}

int PodManager::TerminatePod() {
    MutexLock lock(&mutex_);
    if (proto::kPodPending == pod_.status) {
        LOG(WARNING) << "pod is pendding";
        exit(-1);
    }
    if (pod_.status == proto::kPodStopping
        || pod_.status == proto::kPodTerminated) {
        LOG(WARNING) << "pod is stopping";
        return 0;
    }

   pod_.stage = kPodStageTerminating;
    LOG(WARNING)\
        << "terminate pod"
        << ", current pod status: " << PodStatus_Name(pod_.status);
    if (0 == DoStopPod()) {
        pod_.status = proto::kPodStopping;
    }
    return 0;
}

int PodManager::RebuildPod() {
    MutexLock lock(&mutex_);
    int tasks_size = pod_.desc.tasks().size();
    if (tasks_size != (int)(pod_.env.task_ids.size())) {
        LOG(WARNING)\
            << "container cgroup size: " << pod_.env.task_ids.size()\
            << ", mismatch with task size: " << tasks_size;
        exit(-1);
    }
    TaskDescription task = pod_.desc.tasks(0);
    LOG(WARNING) << task.exe_package().start_cmd();

    pod_.stage = kPodStageRebuilding;
    if (0 == DoStopPod()) {
        pod_.status = proto::kPodStopping;
    }
    return 0;
}

int PodManager::ReloadPod() {
    MutexLock lock(&mutex_);
    int tasks_size = pod_.desc.tasks().size();
    if (tasks_size != (int)(pod_.env.task_ids.size())) {
        LOG(WARNING)\
            << "container cgroup size: " << pod_.env.task_ids.size()\
            << ", mismatch with task size: " << tasks_size;
        exit(-1);
    }
    pod_.desc.CopyFrom(pod_.desc);
    pod_.stage = kPodStageReloading;
    return 0;
}

int PodManager::GetPod(Pod& pod) {
    MutexLock lock(&mutex_);
    pod.pod_id = pod_.pod_id;
    pod.env = pod_.env;
    pod.desc.CopyFrom(pod_.desc);
    pod.status = pod_.status;
    return 0;
}

int PodManager::ClearPod() {
    MutexLock lock(&mutex_);
    task_manager_.ClearTasks();
    return 0;
}

int PodManager::DoCreatePod() {
    mutex_.AssertHeld();
    if (proto::kPodPending != pod_.status) {
        return -1;
    }
    int tasks_size = pod_.desc.tasks().size();
    if (tasks_size != (int)(pod_.env.task_ids.size())) {
        LOG(WARNING)\
            << "container cgroup size: " << pod_.env.task_ids.size()\
            << ", mismatch with task size: " << tasks_size;
        exit(-1);
    }
    pod_.pod_id = pod_.env.pod_id;
    pod_.status = proto::kPodReady;
    pod_.stage = kPodStageCreating;
    LOG(INFO) << "create pod, " << tasks_size << " tasks to be created";
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        TaskEnv env;
        env.job_id = pod_.env.job_id;
        env.pod_id = pod_.env.pod_id;
        env.task_id = task_id;
        env.cgroup_subsystems = pod_.env.cgroup_subsystems;
        env.cgroup_paths = pod_.env.task_cgroup_paths[i];
        int ret = task_manager_.CreateTask(env, pod_.desc.tasks(i));
        if (0 != ret) {
            LOG(WARNING) << "create task " << i << " fail";
            DoClearPod();
            return ret;
        }
        LOG(WARNING) << "create task " << i << " ok";
    }
    LOG(INFO) << "create pod ok";
    return 0;
}

int PodManager::DoClearPod() {
    mutex_.AssertHeld();
    task_manager_.ClearTasks();
    return 0;
}

// do task deploy processes creating
int PodManager::DoDeployPod() {
    mutex_.AssertHeld();
    int tasks_size = pod_.desc.tasks().size();
    LOG(INFO) << "deploy pod, total " << tasks_size << " tasks to be deployed";

    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        if (0 != task_manager_.DeployTask(task_id)) {
            LOG(WARNING) << "create deploy process for task: " << task_id << " fail";
            DoClearPod();
            return -1;
        }
        LOG(INFO) << "create deploy process for task: " << task_id << " ok";
    }
    return 0;
}

// do task main process creating
int PodManager::DoStartPod() {
   mutex_.AssertHeld();
   int tasks_size = pod_.desc.tasks().size();
   LOG(INFO) << "start pod, total " << tasks_size << " tasks to be start";
   for (int i = 0; i < tasks_size; i++) {
       std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
       int ret = task_manager_.StartTask(task_id);
       if (0 != ret) {
           LOG(WARNING) << "create main process for task " << i << " fail";
           return ret;
       }
       LOG(INFO) << "create main process for task " << i << " ok";
   }
   return 0;
}

// do task stop process creating
int PodManager::DoStopPod() {
    mutex_.AssertHeld();
    int tasks_size = pod_.desc.tasks().size();
    LOG(INFO) << "stop pod, total " << tasks_size << " tasks to be stop";
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        if (0 != task_manager_.StopTask(task_id)) {
            LOG(WARNING) << "create stop process for task " << i << " fail";
            task_manager_.CleanTask(task_id);
        }
        LOG(INFO) << "create stop process for task " << i << " ok";
    }
    return 0;
}

void PodManager::LoopChangePodStatus() {
    MutexLock lock(&mutex_);
    switch (pod_.status) {
        LOG(INFO) << "pod status: " << proto::PodStatus_Name(pod_.status);
        case proto::kPodPending:
            PendingPodCheck();
            break;
        case proto::kPodReady:
            ReadyPodCheck();
            break;
        case proto::kPodDeploying:
            DeployingPodCheck();
            break;
        case proto::kPodStarting:
            StartingPodCheck();
            break;
        case proto::kPodRunning:
            RunningPodCheck();
            break;
        case proto::kPodFailed:
            FailedPodCheck();
            break;
        case proto::kPodFinished:
            FinishedPodCheck();
            break;
        case proto::kPodStopping:
            StoppingPodCheck();
            break;
        case proto::kPodTerminated:
            LOG(WARNING) <<  "pod has been terminated";
            if (kPodStageTerminating == pod_.stage) {
                exit(0);
            } else {
                LOG(WARNING) << "pod status change to kPodReady";
                DoClearPod();
                pod_.status = proto::kPodPending;
                DoCreatePod();
            }
            break;
        default:
            break;
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_change_pod_status_interval,
        boost::bind(&PodManager::LoopChangePodStatus, this)
    );
    return;
}

void PodManager::PendingPodCheck() {
    mutex_.AssertHeld();
    if (0 != pod_.desc.tasks().size()
        && 0 == DoCreatePod()) {
        pod_.status = proto::kPodReady;
    }
    return;
}

// create all pod deploy processes,
// if created ok, pod status change to deploying
void PodManager::ReadyPodCheck() {
    mutex_.AssertHeld();
    if (0 == DoDeployPod()) {
        pod_.status = proto::kPodDeploying;
        LOG(INFO) << "pod status change to kPodDeploying";
    }
    return;
}

// check all deploy processes' status,
// if all deployed ok, pod status change to starting
// if one deployed error, pod status change to failed
void PodManager::DeployingPodCheck() {
    mutex_.AssertHeld();
    int tasks_size = pod_.desc.tasks().size();
    TaskStatus task_status = proto::kTaskStarting;
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        Task task;
        if (0 != task_manager_.CheckTask(task_id, task)) {
            return;
        }
        LOG(INFO) << "task: " << task_id\
            << ", status: " << proto::TaskStatus_Name(task.status);
        if (task.status == proto::kTaskFailed) {
            task_status = proto::kTaskFailed;
            break;
        }
        if (task.status < task_status) {
            task_status = task.status;
        }
    }
    if (proto::kTaskStarting == task_status) {
        LOG(INFO) << "pod status change to kPodStarting";
        pod_.status = proto::kPodStarting;
    }
    if (proto::kTaskFailed == task_status) {
        LOG(INFO) << "pod status change to kPodFailed";
        pod_.status = proto::kPodFailed;
    }
    return;
}

// create all pod main processes,
// if all created ok, pod status changed to running
void PodManager::StartingPodCheck() {
    mutex_.AssertHeld();
    if (0 == DoStartPod()) {
        LOG(INFO) << "pod status change to kPodRunning";
        pod_.status = proto::kPodRunning;
    } else {
        LOG(INFO) << "pod status change to kPodFailed";
        pod_.status = proto::kPodFailed;
    }
    return;
}

// check all running processes' status,
// if all exit ok, pod status change to terminated,
// if one exit error, pod status change to failed
void PodManager::RunningPodCheck() {
    mutex_.AssertHeld();
    int tasks_size = pod_.desc.tasks().size();
    TaskStatus task_status = proto::kTaskFinished;
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        Task task;
        if (0 != task_manager_.CheckTask(task_id, task)) {
            return;
        }
        if (proto::kTaskFailed == task.status) {
            task_status = proto::kTaskFailed;
            break;
        }
        if (task.status < task_status) {
            task_status = task.status;
        }
    }

    if (proto::kTaskRunning != task_status) {
        if (proto::kTaskFinished == task_status) {
            LOG(INFO) << "pod status change to kPodFinished";
            pod_.status = proto::kPodFinished;
        } else {
            LOG(INFO) << "pod status change to kPodFailed";
            pod_.status = proto::kPodFailed;
        }
    }
    return;
}

// clean all tasks and set pod status to kPodReady and increase fail_count
void PodManager::FailedPodCheck() {
    mutex_.AssertHeld();
    LOG(INFO) << "error exit";
    return;
}

void PodManager::FinishedPodCheck() {
    mutex_.AssertHeld();
    LOG(INFO) << "normal exit";
    exit(0);
    return;
}

void PodManager::StoppingPodCheck() {
    mutex_.AssertHeld();
    LOG(INFO) << "stopping pod check";
    int tasks_size = pod_.desc.tasks().size();
    TaskStatus task_status = proto::kTaskTerminated;
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_.pod_id + "_" + boost::lexical_cast<std::string>(i);
        Task task;
        if (0 != task_manager_.CheckTask(task_id, task)) {
            task_manager_.CleanTask(task_id);
        }
        if (proto::kTaskStopping == task.status) {
            task_status = proto::kTaskStopping;
            break;
        }
    }
    if (proto::kTaskTerminated == task_status) {
        LOG(WARNING) << "pod status change to kPodTerminated";
        pod_.status = proto::kPodTerminated;
    }
    return;
}

}   // ending namespace galaxy
}   // ending namespace baidu
