// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SCHEDULER_H
#define BAIDU_GALAXY_SCHEDULER_H
#include "proto/master.pb.h"
#include "common/thread_pool.h"

namespace baidu {
namespace galaxy {

class Scheduler {

public:
    Scheduler():thread_pool_(){}
    ~Scheduler() {}
private:
    baidu::common::ThreadPool thread_pool_;
};
} // galaxy
}// baidu
#endif
