// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_collector.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "protocol/galaxy.pb.h"

DECLARE_int32(task_collector_collect_interval);

namespace baidu {
namespace galaxy {

TaskCollector::TaskCollector(TaskInfo* task_info) :
        background_pool_(1),
        task_info_(task_info) {
    LOG(INFO) << "task collector start collect, task: " << task_info->taskid();
    background_pool_.DelayTask(FLAGS_task_collector_collect_interval,
        boost::bind(&TaskCollector::Collect, this)
    );
}

TaskCollector::~TaskCollector() {
    if (task_info_ != NULL) {
        delete task_info_;
    }
}

void TaskCollector::Collect() {
    LOG(INFO) << "task collector collect";
}

}   // ending namespace galaxy
}   // ending namespace baidu
