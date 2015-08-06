// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SCHEDULER_IO_H
#define BAIDU_GALAXY_SCHEDULER_IO_H


#include <map>
#include <string>
#include <assert.h>
#include "proto/master.pb.h"
#include "scheduler/scheduler.h"
#include "master/master_watcher.h"
#include "rpc/rpc_client.h"
#include <mutex.h>

namespace baidu {
namespace galaxy {
class SchedulerIO {

public:
    SchedulerIO() : rpc_client_(),
                    master_stub_(NULL),
                    scheduler_() {
    }
    ~SchedulerIO(){}
    void HandleMasterChange(const std::string& new_master_endpoint);
    bool Init();
    void Loop();
private:
    std::string master_addr_;
    RpcClient rpc_client_;
    Master_Stub* master_stub_;
    Scheduler scheduler_;
    MasterWatcher master_watcher_;
    Mutex master_mutex_;
};

} // galaxy
}// baidu
#endif
