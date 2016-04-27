// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_TASK_COLLECTOR_H
#define BAIDU_GALAXY_TASK_COLLECTOR_H

#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {

class TaskCollector {
public:
    TaskCollector();
    TaskCollector(proto::TaskInfo* task_info);
    ~TaskCollector();

private:
    proto::TaskInfo* task_info_;
};


}   // ending namespace galaxy
}   // ending namespace baidu

#endif  //_SRC_AGENT_RESOURCE_COLLECTOR_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
