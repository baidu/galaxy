// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pod_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/appmaster.pb.h"

DECLARE_int32(pod_manager_pod_check_interval);
DECLARE_int32(pod_manager_pod_status_change_check_interval);

namespace baidu {
namespace galaxy {

PodManager::PodManager() :
        mutex_pod_manager_(),
        pod_(NULL),
        task_manager_(NULL),
        background_pool_(10) {
    background_pool_.DelayTask(
        FLAGS_pod_manager_pod_check_interval,
        boost::bind(&PodManager::LoopPodCheck, this)
    );
    background_pool_.DelayTask(
        FLAGS_pod_manager_pod_status_change_check_interval,
        boost::bind(&PodManager::LoopPodStatusChangeCheck, this)
    );
}

PodManager::~PodManager() {
    if (NULL != pod_) {
        delete pod_;
    }
    if (NULL != task_manager_) {
        delete task_manager_;
    }
    background_pool_.Stop(false);
}

void PodManager::RunPod(const PodInfo* pod_info) {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_) {
        return;
    }
    pod_ = new PodInfo;
    pod_->CopyFrom(*pod_info);
    pod_->set_status(proto::kPodReady);
    return;
}

void PodManager::KillPod(const PodInfo* pod_info) {
    LOG(INFO) << "kill pod: " << pod_info->podid();
    return;
}

void PodManager::LoopPodCheck() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_) {
        LOG(INFO) << "loop check pod: " << pod_->podid()\
            << ", status: " << PodStatus_Name(pod_->status()).c_str();
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_pod_check_interval,
        boost::bind(&PodManager::LoopPodCheck, this)
    );
}

void PodManager::LoopPodStatusChangeCheck() {
    MutexLock lock(&mutex_pod_manager_);
    LOG(INFO) << "loop pod status change check";

    if (NULL != pod_) {
        switch (pod_->status()) {
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
        default:
            LOG(INFO) << "pod status: " << PodStatus_Name(pod_->status())\
                << " no need change";
        }
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_pod_status_change_check_interval,
        boost::bind(&PodManager::LoopPodStatusChangeCheck, this)
    );
    return;
}

int DeployPod() {
    return 0;
}

int PodManager::ReadyPodCheck() {
    mutex_pod_manager_.AssertHeld();

    // deploy task
    TaskInfo* task = new TaskInfo;
    task->set_jobid(pod_->jobid());
    task->set_podid(pod_->podid());

    task->set_taskid("task-deploy");
    std::string workspace = std::string("/home/wanghaitao01/galaxy/") + pod_->podid();

    for (int i = 0; i < pod_->pod_desc.tasks_size(); i++) {
        TaskInfo* task_info = pod_->pod_desc.tasks(i);
        LOG(INFO) << "deploy task: " << task_info->taskid();
    }

    return 0;
}

int PodManager::DeployingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "deploying pod check";
    pod_->set_status(proto::kPodStarting);
    return 0;
}

int PodManager::StartingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "starting pod check";
    pod_->set_status(proto::kPodRunning);
    return 0;
}

int PodManager::RunningPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "running pod check";
    pod_->set_status(proto::kPodTerminated);
    return 0;
}


}   // ending namespace galaxy
}   // ending namespace baidu

