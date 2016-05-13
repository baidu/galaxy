// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pod_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/appmaster.pb.h"

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
        LOG(WARNING) << "local pod is not null, ignore run pod action";
        return;
    }
    pod_ = new Pod();
    pod_->desc.CopyFrom(*pod_desc);
    pod_->status = proto::kPodReady;
    LOG(INFO) << "run pod";
    return;
}

void PodManager::DeletePod() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL == pod_) {
        LOG(WARNING) << "no local pod running, ignore kill pod action";
        return;
    }
    LOG(INFO) << "kill pod";
    pod_->status = proto::kPodTerminated;
    return;
}

void PodManager::LoopCheckPod() {
    MutexLock lock(&mutex_pod_manager_);
    if (NULL != pod_) {
        LOG(INFO) << "loop check pod, status: " << PodStatus_Name(pod_->status).c_str();
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_check_pod_interval,
        boost::bind(&PodManager::LoopCheckPod, this)
    );
}

void PodManager::LoopChangePodStatus() {
    MutexLock lock(&mutex_pod_manager_);
    LOG(INFO) << "loop change pod status";

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
        default:
            LOG(INFO) << "pod status: " << PodStatus_Name(pod_->status)\
                << " no need change";
        }
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_change_pod_status_interval,
        boost::bind(&PodManager::LoopChangePodStatus, this)
    );
    return;
}

int PodManager::DeployPod() {
    return 0;
}

int PodManager::StartPod() {
    return 0;
}

int PodManager::ReadyPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "ready pod check";
    if (0 == DeployPod()) {
        pod_->status = proto::kPodDeploying;
    }
    return 0;
}

int PodManager::DeployingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "deploying pod check";
    // check deploy process info
    // int ret = task_manager_.GetProcessInfo(pod_->deploy_process);
    pod_->status = proto::kPodStarting;
    return 0;
}

int PodManager::StartingPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "starting pod check";
    if (0 == StartPod()) {
        pod_->status = proto::kPodRunning;
    }
    return 0;
}

int PodManager::RunningPodCheck() {
    mutex_pod_manager_.AssertHeld();
    LOG(INFO) << "running pod check";
    pod_->status = proto::kPodTerminated;
    return 0;
}

}   // ending namespace galaxy
}   // ending namespace baidu
