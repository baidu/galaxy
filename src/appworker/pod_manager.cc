// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pod_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/appmaster.pb.h"

DECLARE_int32(pod_manager_check_pod_interval);

namespace baidu {
namespace galaxy {

PodManager::PodManager() :
        mutex_pod_manager_(),
        pod_(NULL),
        task_manager_(NULL),
        background_pool_(1) {
    background_pool_.DelayTask(
        FLAGS_pod_manager_check_pod_interval,
        boost::bind(&PodManager::CheckPod, this)
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

int PodManager::RunPod(const PodInfo* pod_info) {
    pod_ = new PodInfo;
    pod_->CopyFrom(*pod_info);
    LOG(INFO) << "run pod: " << pod_info->podid();
    return 0;
}

int PodManager::KillPod(const PodInfo* pod_info) {
    LOG(INFO) << "kill pod: " << pod_info->podid();
    return 0;
}

void PodManager::CheckPod() {
    if (NULL == pod_) {
        LOG(INFO) << "no pod to check";
    } else {
        LOG(INFO) << "check pod: " << pod_->podid();
    }

    background_pool_.DelayTask(
        FLAGS_pod_manager_check_pod_interval,
        boost::bind(&PodManager::CheckPod, this)
    );
}


}   // ending namespace galaxy
}   // ending namespace baidu
