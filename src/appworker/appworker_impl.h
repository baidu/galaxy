// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef  __APPWORKER_IMPL_H_
#define  __APPWORKER_IMPL_H_

#include <string>
#include <map>

#include <mutex.h>
#include <thread_pool.h>

#include "src/protocol/appmaster.pb.h"
#include "src/rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

class AppWorkerImpl {

public:
    AppWorkerImpl();
    virtual ~AppWorkerImpl();
    int Init();

private:
    void FetchTask();

private:
    Mutex lock_;
    ThreadPool backgroud_pool_;
    std::map<std::string, std::string > tasks_;
    RpcClient* rpc_client_;
//    baidu::galaxy::AppMaster_Stub* app_master_;

//    MasterWatcher* master_watcher_;
//    Mutex mutex_master_endpoint_;

};

} // ending namespace galaxy
} // ending namespace baidu


#endif  //__APPWORKER_IMPL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
