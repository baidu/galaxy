// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_TASK_COLLECTOR_H
#define BAIDU_GALAXY_TASK_COLLECTOR_H

#include <thread_pool.h>

#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {

typedef proto::TaskInfo TaskInfo;

class TaskCollector {
public:
    TaskCollector(TaskInfo* task_info);
    ~TaskCollector();
    void Collect();

private:
    ThreadPool background_pool_;
    TaskInfo* task_info_;
};


}   // ending namespace galaxy
}   // ending namespace baidu

#endif  // BAIDU_GALAXY_TASK_COLLECTOR_H
