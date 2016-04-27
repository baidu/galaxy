// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_collector.h"
#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {

TaskCollector::TaskCollector() {
}

TaskCollector::TaskCollector(proto::TaskInfo* task_info) :
    task_info_(task_info) {
}

TaskCollector::~TaskCollector() {
    if (task_info_ != NULL) {
        delete task_info_;
    }
}


}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */

