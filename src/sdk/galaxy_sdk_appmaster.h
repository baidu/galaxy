// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SDK_APPMASTER_H
#define BAIDU_GALAXY_SDK_APPMASTER_H

#include "ins_sdk.h"
#include "protocol/appmaster.pb.h"
#include "protocol/galaxy.pb.h"
#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

class RpcClient;

namespace sdk {

class AppMaster {
public:
    AppMaster();
    ~AppMaster();
    bool GetStub();
    bool SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response);
    bool UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response);
    bool StopJob(const StopJobRequest& request, StopJobResponse* response);
    bool RemoveJob(const RemoveJobRequest& request, RemoveJobResponse* response);
    bool ListJobs(const ListJobsRequest& request, ListJobsResponse* response);
    bool ShowJob(const ShowJobRequest& request, ShowJobResponse* response);
    bool ExecuteCmd(const ExecuteCmdRequest& request, ExecuteCmdResponse* response);
private:
    ::galaxy::ins::sdk::InsSDK* nexus_;
    RpcClient* rpc_client_;
    ::baidu::galaxy::proto::AppMaster_Stub* appmaster_stub_;
    std::string full_key_;
};

} //end namespace sdk
} //end namespace client
} //end namespace baidu

#endif  // BAIDU_GALAXY_SDK_APPMASTER_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
