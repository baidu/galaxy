// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pod_manager.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/appmaster.pb.h"

DECLARE_string(appworker_container_id);
DECLARE_int32(pod_manager_pod_max_fail_count);
DECLARE_int32(pod_manager_check_pod_interval);
DECLARE_int32(pod_manager_change_pod_status_interval);

namespace baidu {
namespace galaxy {

PodManager::PodManager() :
        mutex_pod_manager_(),
        pod_(NULL),
        background_pool_(10) {
    background_pool_.DelayTask(
        FLAGS_pod_manager_check_pod_interval,
        boost::bind(&PodManager::LoopCheckPod, this)
    );
    background_pool_.DelayTask(
        FLAGS_pod_manager_change_pod_status_interval,
        boost::bind(&PodManager::LoopChangePodStatus, this)
    );
}

PodManager::~PodManager() {
    if (NULL != pod_) {
        delete pod_;
    }
    background_pool_.Stop(false);
}

void PodManager::CreatePod(const PodDescription* pod_desc) {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_) {
        return;
    }
    pod_ = new Pod();
    pod_->pod_id = FLAGS_appworker_container_id;
    pod_->desc.CopyFrom(*pod_desc);
    pod_->status = proto::kPodReady;
    int tasks_size = pod_->desc.tasks().size();
    LOG(INFO) << "create pod, total " << tasks_size << " tasks to be created";
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
        int ret = task_manager_.CreateTask(task_id, pod_->desc.tasks(i));
        if (0 != ret) {
            LOG(WARNING) << "create task " << i << " fail";
            task_manager_.ClearTasks();
            return;
        }
        LOG(WARNING) << "create task " << i << " ok";
    }
    LOG(INFO) << "create pod ok";
    return;
}

void PodManager::DeletePod() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL == pod_) {
        return;
    }
    LOG(INFO) << "kill pod";
    pod_->status = proto::kPodFinished;
    return;
}

void PodManager::LoopCheckPod() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_ && pod_->status < proto::kPodFailed) {
        // pod running status
        int64_t read_bytes_ps = 0;
        int64_t read_io_ps = 0;
        int64_t write_bytes_ps = 0;
        int64_t write_io_ps = 0;
        int64_t cpu = 0;
        int64_t memory = 0;
        int64_t disk = 0;
        int64_t ssd = 0;
        LOG(INFO)\
            << "loop check pod, status: " << PodStatus_Name(pod_->status).c_str()\
            << ", read_bytes_ps: " << read_bytes_ps\
            << ", read_io_ps: " << read_io_ps\
            << ", write_bytes_ps: " << write_bytes_ps\
            << ", write_io_ps: " << write_io_ps\
            << ", cpu: " << cpu\
            << ", memory: " << memory\
            << ", disk: " << disk\
            << ", ssd: " << ssd;
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_check_pod_interval,
        boost::bind(&PodManager::LoopCheckPod, this)
    );
}

void PodManager::LoopChangePodStatus() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_) {
        switch (pod_->status) {
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
        default:
            break;
        }
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_change_pod_status_interval,
        boost::bind(&PodManager::LoopChangePodStatus, this)
    );
    return;
}

// do task deploy processes creating
int PodManager::DoDeployPod() {
    mutex_pod_manager_.AssertHeld();
    int tasks_size = pod_->desc.tasks().size();
    LOG(INFO) << "deploy pod, total " << tasks_size << " tasks to be deployed";

    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
        if (0 != task_manager_.DeployTask(task_id)) {
            LOG(WARNING) << "create deploy process for task: " << task_id.c_str() << " fail";
            DoCleanPod();
            return -1;
        }
        LOG(INFO) << "create deploy process for task: " << task_id.c_str() << " ok";
    }
    return 0;
}

// do task main process creating
int PodManager::DoStartPod() {
   mutex_pod_manager_.AssertHeld();
   int tasks_size = pod_->desc.tasks().size();
   LOG(INFO) << "start pod, total " << tasks_size << " tasks to be start";
   for (int i = 0; i < tasks_size; i++) {
       std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
       int ret = task_manager_.StartTask(task_id);
       if (0 != ret) {
           LOG(WARNING) << "create main process for task " << i << " fail";
           return ret;
       }
       LOG(INFO) << "create main process for task " << i << " ok";
   }
   return 0;
}

// do task stop process creating and add kill thread to DelayTask
int PodManager::DoStopPod() {
    mutex_pod_manager_.AssertHeld();
    int tasks_size = pod_->desc.tasks().size();
    LOG(INFO) << "stop pod, total " << tasks_size << " tasks to be stop";
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
        if (0 != task_manager_.StartTask(task_id)) {
            LOG(WARNING) << "create stop process for task " << i << " fail";
        }
        LOG(INFO) << "create main process for task " << i << " ok";
    }
    pod_->status = proto::kPodFinished;
    // TODO thread_pool.AddTask
    return 0;
}

int PodManager::DoCleanPod() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "clean pod";
    // TODO kill all pod processes
    return 0;
}

// create all pod deploy processes,
// if created ok, pod status change to deploying
void PodManager::ReadyPodCheck() {
    mutex_pod_manager_.AssertHeld();
    if (0 == DoDeployPod()) {
        pod_->status = proto::kPodDeploying;
        LOG(INFO) << "pod status change to kPodDeploying";
    }
    return;
}

// check all deploy processes' status,
// if all deployed ok, pod status change to starting
// if one deployed error, pod status change to failed
void PodManager::DeployingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    int tasks_size = pod_->desc.tasks().size();
    LOG(INFO) << "deploying pod check, total tasks size: " << tasks_size;
    TaskStatus task_status = proto::kTaskStarting;
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
        Task task;
        if (0 != task_manager_.CheckTask(task_id, task)) {
            return;
        }
        LOG(INFO) << "task: " << task_id.c_str()\
            << ", status: " << proto::TaskStatus_Name(task.status).c_str();
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
        pod_->status = proto::kPodStarting;
    }
    if (proto::kTaskFailed == task_status) {
        LOG(INFO) << "pod status change to kPodFailed";
        pod_->status = proto::kPodFailed;
    }
    return;
}

// create all pod main processes,
// if all created ok, pod status changed to running
void PodManager::StartingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "starting pod check";
    if (0 == DoStartPod()) {
        LOG(INFO) << "pod status change to kPodRunning";
        pod_->status = proto::kPodRunning;
    } else {
        LOG(INFO) << "pod status change to kPodFailed";
        pod_->status = proto::kPodFailed;
    }
    return;
}

// check all running processes' status,
// if all exit ok, pod status change to terminated,
// if one exit error, pod status change to failed
void PodManager::RunningPodCheck() {
    mutex_pod_manager_.AssertHeld();
    int tasks_size = pod_->desc.tasks().size();
    LOG(INFO) << "running pod check, total tasks size: " << tasks_size;
    TaskStatus task_status = proto::kTaskFinished;
    for (int i = 0; i < tasks_size; i++) {
        std::string task_id = pod_->pod_id + "_" + boost::lexical_cast<std::string>(i);
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
            pod_->status = proto::kPodFinished;
        } else {
            LOG(INFO) << "pod status change to kPodFinished";
            pod_->status = proto::kPodFinished;
        }
    }
    return;
}

// clean all tasks and set pod status to kPodReady and increase fail_count
void PodManager::FailedPodCheck() {
    mutex_pod_manager_.AssertHeld();
    if (pod_->fail_count < FLAGS_pod_manager_pod_max_fail_count) {
        LOG(INFO) << "failed pod check, clean all processes";
        pod_->fail_count += 1;
        pod_->status = proto::kPodReady;
    }
    return;
}

}   // ending namespace galaxy
}   // ending namespace baidu
