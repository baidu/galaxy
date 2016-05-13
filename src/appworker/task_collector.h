// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_TASK_COLLECTOR_H
#define BAIDU_GALAXY_TASK_COLLECTOR_H

#include <string>

#include <thread_pool.h>

namespace baidu {
namespace galaxy {

class TaskCollector {
public:
    TaskCollector();
    ~TaskCollector();
    void Collect(const std::string& task_id);

private:
    ThreadPool background_pool_;
};

}   // ending namespace galaxy
}   // ending namespace baidu


#endif  // BAIDU_GALAXY_TASK_COLLECTOR_H
