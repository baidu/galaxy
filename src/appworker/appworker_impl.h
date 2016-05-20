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
#include "pod_manager.h"
#include "ins_sdk.h"

namespace baidu {
namespace galaxy {

typedef ::galaxy::ins::sdk::InsSDK InsSDK;
typedef ::galaxy::ins::sdk::SDKError SDKError;
typedef google::protobuf::RepeatedPtrField<proto::TaskInfo> TaskInfoList;
typedef proto::AppMaster_Stub AppMaster_Stub;
typedef proto::FetchTaskRequest FetchTaskRequest;
typedef proto::FetchTaskResponse FetchTaskResponse;
typedef proto::ErrorCode ErrorCode;

class AppWorkerImpl {
public:
    AppWorkerImpl();
    ~AppWorkerImpl();
    void PrepareEnvs();
    void Start();

private:
    void FetchTask();
    void FetchTaskCallback(const FetchTaskRequest* request,
                           FetchTaskResponse* response,
                           bool failed, int error);
    void UpdateAppMasterStub();

private:
    Mutex mutex_appworker_;
    std::string endpoint_;
    std::string job_id_;
    std::string pod_id_;
    std::vector<std::string> task_ids_;
    std::vector<std::string> cgroup_subsystems_;
    std::vector<std::map<std::string, std::string> > task_cgroup_paths_;
    int64_t start_time_;

    RpcClient rpc_client_;
    InsSDK* nexus_;
    std::string appmaster_endpoint_;
    AppMaster_Stub* appmaster_stub_;
    PodManager pod_manager_;
    ThreadPool backgroud_pool_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif  // BAIDU_GALAXY_APPWORKER_IMPL_H
