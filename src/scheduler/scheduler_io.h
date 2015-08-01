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

#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {
class SchedulerIO {

public:
    SchedulerIO(const std::string& master_addr)
				:master_addr_(master_addr),
				 rpc_client_(),
				 master_stub_(NULL),
				 scheduler_(){
    	LOG(INFO, "create scheduler io from master %s", master_addr_.c_str());
    	bool ret = rpc_client_.GetStub(master_addr_, &master_stub_);
    	assert(ret == true);
    }
    ~SchedulerIO(){}

    void Loop();
private:
    std::string master_addr_;
    RpcClient rpc_client_;
    Master_Stub* master_stub_;
    Scheduler scheduler_;

};


} // galaxy
}// baidu
#endif
