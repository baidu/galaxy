// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_collector.h"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_int32(task_collector_collect_interval);

namespace baidu {
namespace galaxy {

TaskCollector::TaskCollector() : background_pool_(1) {
}

TaskCollector::~TaskCollector() {
}

void TaskCollector::Collect(const std::string& task_id) {
    LOG(INFO) << "task collector start collect";
    background_pool_.DelayTask(
        FLAGS_task_collector_collect_interval,
        boost::bind(&TaskCollector::Collect, this, task_id)
    );
}

}   // ending namespace galaxy
}   // ending namespace baidu
