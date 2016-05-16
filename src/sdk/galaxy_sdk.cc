// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "galaxy_sdk.h"
#include "src/rpc/rpc_client.h"
#include "ins_sdk.h"

//nexus
DECLARE_string(nexus_addr);
DECLARE_string(nexus_root);
DECLARE_string(appmaster_nexus_path);
DECLARE_string(appmaster_endpoint);

namespace baidu {
namespace galaxy {
namespace sdk {

ResourceManager::ResourceManager(const std::string& nexus_key) 
 										: rpc_client_(NULL),
 										  res_stub_(NULL) {
	rpc_client_ = new RpcClient();
    full_key_ = FLAGS_nexus_root + nexus_key;
    nexus_ = new ::galaxy::ins::sdk::InsSDK(FLAGS_nexus_addr);
}

ResourceManager::~ResourceManager() {
	delete rpc_client_;
    if (NULL != res_stub_) {
        delete res_stub_;
    }
    delete nexus_;
}

bool ResourceManager::GetStub() {
    std::string end_point;
    ::galaxy::ins::sdk::InsSDK::SDKError err;
    bool ok = nexus_->Get(full_key_, &endpoint, &err);
    if (!ok || err != ::galaxy::ins::sdk::kOK) {
        fprintf(stderr, "get appmaster endpoint from nexus failed: %s\n",
                ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
        return false;
    }
    if(!rpc_client_.GetStub(endpoint, &res_stub_)) {
        fprintf(stderr, "connect resmanager fail, resmanager: %s\n", endpoint_.c_str());
        return false;
    }
    return true;
}

bool ResourceManager::Login(const std::string& user, const std::string& password) {
	return false;
}

bool ResourceManager::EnterSafeMode(const EnterSafeModeRequest& request, EnterSafeModeResponse* response) {
    return false;
}

bool ResourceManager::LeaveSafeMode(const LeaveSafeModeRequest& request, LeaveSafeModeResponse* response) {
    return false;
}

bool ResourceManager::Status(const StatusRequest& request, StatusResponse* response) {
    return false;
}
bool ResourceManager::CreateContainerGroup(const CreateContainerGroupRequest& request, CreateContainerGroupResponse* response) {
    return rpc_client_->SendRequest(res_stub_, &::proto::ResMan_Stub::CreateContainerGroup,
                                        &request, response, 5, 1);
}
bool ResourceManager::RemoveContainerGroup(const RemoveContainerGroupRequest& request, RemoveContainerGroupResponse* response) {
    return rpc_client_->SendRequest(res_stub_, &::proto::ResMan_Stub::RemoveContainerGroup,
                                        &request, response, 5, 1);
}

bool ResourceManager::UpdateContainerGroup(const UpdateContainerGroupRequest& request, UpdateContainerGroupResponse* response) {
    return rpc_client_->SendRequest(res_stub_, &::proto::ResMan_Stub::UpdateContainerGroup,
                                        &request, response, 5, 1);
}

bool ResourceManager::ListContainerGroups(const ListContainerGroupsRequest& request, ListContainerGroupsResponse* response) {
    return rpc_client_->SendRequest(res_stub_, &::proto::ResMan_Stub::ListContainerGroups,
                                        &request, response, 5, 1);
}

bool ResourceManager::ShowContainerGroup(const ShowContainerGroupRequest& request, ShowContainerGroupResponse* response) {
    return rpc_client_->SendRequest(res_stub_, &::proto::ResMan_Stub::ShowContainerGroups,
                                        &request, response, 5, 1);
}

bool ResourceManager::AddAgent(const AddAgentRequest& request, AddAgentResponse* response) {
    return false;
}

bool ResourceManager::RemoveAgent(const RemoveAgentRequest& request, RemoveAgentResponse* response) {
    return false;
}

bool ResourceManager::OnlineAgent(const OnlineAgentRequest& request, OnlineAgentResponse* response) {
    return false;
}

bool ResourceManager::OfflineAgent(const OfflineAgentRequest& request, OfflineAgentResponse* response) {
    return false;
}

bool ResourceManager::ListAgents(const ListAgentsRequest& request, ListAgentsResponse* response) {
    return false;
}

bool ResourceManager::CreateTag(const CreateTagRequest& request, CreateTagResponse* response) {
    return false;
}

bool ResourceManager::ListTags(const ListTagsRequest& request, ListTagsResponse* response) {
    return false;
}

bool ResourceManager::ListAgentsByTag(const ListAgentsByTagRequest& request, ListAgentsByTagResponse* response) {
    return false;
}

bool ResourceManager::GetTagsByAgent(const GetTagsByAgentRequest& request, GetTagsByAgentResponse* response) {
    return false;
}

bool ResourceManager::AddAgentToPool(const AddAgentToPoolRequest& request, AddAgentToPoolResponse* response) {
    return false;
}

bool ResourceManager::RemoveAgentFromPool(const RemoveAgentFromPoolRequest& request, RemoveAgentFromPoolResponse* response) {
    return false;
}

bool ResourceManager::ListAgentsByPool(const ListAgentsByPoolRequest& request, ListAgentsByPoolResponse* response) {
    return false;
}

bool ResourceManager::GetPoolByAgent(const GetPoolByAgentRequest& request, GetPoolByAgentResponse* response) {
    return false;
}

bool ResourceManager::AddUser(const AddUserRequest& request, AddUserResponse* response) {
    return false;
}

bool ResourceManager::RemoveUser(const RemoveUserRequest& request, RemoveUserResponse* response) {
    return false;
}

bool ResourceManager::ListUsers(const ListUsersRequest& request, ListUsersResponse* response) {
    return false;
}

bool ResourceManager::ShowUser(const ShowUserRequest& request, ShowUserResponse* response) {
    return false;
}

bool ResourceManager::GrantUser(const GrantUserRequest& request, GrantUserResponse* response) {
    return false;
}

bool ResourceManager::AssignQuota(const AssignQuotaRequest& request, AssignQuotaResponse* response) {
    return false;
}

AppMaster::AppMaster(const std::string& nexus_key) 
                               : rpc_client_(NULL),
                               : appmaster_stub_(NULL) {
    full_key_ = FLAGS_nexus_root + nexus_key; 
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
    std::string end_point;
    ::galaxy::ins::sdk::InsSDK::SDKError err;
    bool ok = nexus_->Get(full_key_, &endpoint, &err);
    if (!ok || err != ::galaxy::ins::sdk::kOK) {
        fprintf(stderr, "get appmaster endpoint from nexus failed: %s\n", 
                ::galaxy::ins::sdk::InsSDK::StatusToString(err).c_str());
        return false;
    }
    if(!rpc_client_.GetStub(endpoint, &appmaster_stub_)) {
        fprintf(stderr, "connect appmaster fail, appmaster: %s\n", endpoint_.c_str());
        return false;
    }
    return true;
}

bool AppMaster::SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::SubmitJob,
                                        &request, response, 5, 1);
}

bool AppMaster::UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::UpdateJob,
                                        &request, response, 5, 1);
}

bool AppMaster::StopJob(const StopJobRequest& request, StopJobResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::StopJob,
                                        &request, response, 5, 1);
}

bool AppMaster::RemoveJob(const RemoveJobRequest& request, RemoveJobResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::RemoveJob,
            &request, response, 5, 1);
}

bool AppMaster::ListJobs(const ListJobsRequest& request, ListJobsResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::ListJobs,
                                        &request, response, 5, 1);
}

bool AppMaster::ShowJob(const ShowJobRequest& request, ShowJobResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::ShowJob,
                                        &request, response, 5, 1);
}

bool AppMaster::ExecuteCmd(const ExecuteCmdRequest& request, ExecuteCmdResponse* response) {
    return rpc_client_->SendRequest(appmaster_stub_, &AppMaster_Stub::ExecuteCmd,
                                    &request, response, 5, 1);
}

} //namespace sdk
} //namespace galaxy
} //namespace baidu
