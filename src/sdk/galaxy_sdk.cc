// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "galaxy_sdk.h"
#include "src/rpc/rpc_client.h"
#include "ins_sdk.h"

namespace baidu {
namespace galaxy {
namespace sdk {

ResourceManager::ResourceManager(const std::string& nexus_root) 
 										: rpc_client_(NULL),
 										  nexus_root_(nexus_root) {
	rpc_client_ = new RpcClient();
}

ResourceManager::~ResourceManager() {
	delete rpc_client_;
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
    return false;
}

bool ResourceManager::RemoveContainerGroup(const RemoveContainerGroupRequest& request, RemoveContainerGroupResponse* response) {
    return false;
}

bool ResourceManager::UpdateContainerGroup(const UpdateContainerGroupRequest& request, UpdateContainerGroupResponse* response) {
    return false;
}

bool ResourceManager::ListContainerGroups(const ListContainerGroupsRequest& request, ListContainerGroupsResponse* response) {
    return false;
}

bool ResourceManager::ShowContainerGroup(const ShowContainerGroupRequest& request, ShowContainerGroupResponse* response) {
    return false;
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

AppMaster::AppMaster(const std::string& nexus_root) 
                               : rpc_client_(NULL),
                                 nexus_root_(nexus_root) {
	rpc_client_ = new RpcClient();
}

AppMaster::~AppMaster() {
	delete rpc_client_;
}


bool AppMaster::SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response) {
    return false;
}

bool AppMaster::UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response) {
    return false;
}

bool AppMaster::StopJob(const StopJobRequest& request, StopJobResponse* response) {
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

} //namespace sdk
} //namespace galaxy
} //namespace baidu
