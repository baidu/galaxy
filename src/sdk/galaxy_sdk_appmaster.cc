// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_sdk_util.h"
#include "rpc/rpc_client.h"
#include "ins_sdk.h"
#include <gflags/gflags.h>
#include "galaxy_sdk_appmaster.h"

//nexus
DECLARE_string(nexus_addr);
DECLARE_string(nexus_root);
DECLARE_string(appmaster_path);

namespace baidu {
namespace galaxy {

class RpcClient;

namespace sdk {

AppMaster::AppMaster() : rpc_client_(NULL),
                       appmaster_stub_(NULL) {
    full_key_ = FLAGS_nexus_root + FLAGS_appmaster_path; 
	rpc_client_ = new RpcClient();
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr); 
}

AppMaster::~AppMaster() {
	delete rpc_client_;
    if (NULL != appmaster_stub_) {
        delete appmaster_stub_;
    }
    delete nexus_;
}

bool AppMaster::GetStub() {
    std::string endpoint;
    ::galaxy::ins::sdk::SDKError err;
    bool ok = nexus_->Get(full_key_, &endpoint, &err);
    if (!ok || err != ::galaxy::ins::sdk::kOK) {
        fprintf(stderr, "get appmaster endpoint from nexus failed: %s\n", 
                ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
        return false;
    }
    if(!rpc_client_->GetStub(endpoint, &appmaster_stub_)) {
        fprintf(stderr, "connect appmaster fail, appmaster: %s\n", endpoint.c_str());
        return false;
    }
    return true;
}

bool AppMaster::SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response) {
    //return rpc_client_->SendRequest(appmaster_stub_, &::baidu::galaxy::proto::AppMaster_Stub::SubmitJob,
    //                                    &request, response, 5, 1);
    return false;
}

bool AppMaster::UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response) {
    //return rpc_client_->SendRequest(appmaster_stub_, &::baidu::galaxy::proto::AppMaster_Stub::UpdateJob,
    //                                    &request, response, 5, 1);
    return false;
}

bool AppMaster::StopJob(const StopJobRequest& request, StopJobResponse* response) {
    //return rpc_client_->SendRequest(appmaster_stub_, &::baidu::galaxy::proto::AppMaster_Stub::StopJob,
    //                                    &request, response, 5, 1);
    return false;
}

bool AppMaster::RemoveJob(const RemoveJobRequest& request, RemoveJobResponse* response) {
    return false;
}

bool AppMaster::ListJobs(const ListJobsRequest& request, ListJobsResponse* response) {
    return false;
}

bool AppMaster::ShowJob(const ShowJobRequest& request, ShowJobResponse* response) {
    return false;
}

bool AppMaster::ExecuteCmd(const ExecuteCmdRequest& request, ExecuteCmdResponse* response) {
    return false;
}


} //end namespace sdk
} //end namespace galaxy
} // end namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
