// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef  BAIDU_GALAXY_APPWORKER_IMPL_H
#define  BAIDU_GALAXY_APPWORKER_IMPL_H

#include <string>
#include <map>

#include <mutex.h>
#include <thread_pool.h>

#include "protocol/appmaster.pb.h"
#include "rpc/rpc_client.h"
#include "task_manager.h"
#include "ins_sdk.h"

namespace baidu {
namespace galaxy {

typedef google::protobuf::RepeatedPtrField<proto::TaskInfo> TaskInfoList;

class AppWorkerImpl {

public:
    AppWorkerImpl();
    virtual ~AppWorkerImpl();
    int Init();

private:
    void FetchTask();
    void FetchTaskCallback(const proto::FetchTaskRequest* request,
                           proto::FetchTaskResponse* response,
                           bool failed, int error);
    int RefreshAppMasterStub(proto::AppMaster_Stub& appmaster_stub);

private:
    Mutex mutex_appworker_;
    Mutex mutex_master_endpoint_;
    std::string container_id_;
    ThreadPool backgroud_thread_pool_;
    RpcClient rpc_client_;
    std::string appmaster_endpoint_;
    proto::AppMaster_Stub* appmaster_stub_;
    TaskManager task_manager_;
    InsSDK* nexus_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif  // BAIDU_GALAXY_APPWORKER_IMPL_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
