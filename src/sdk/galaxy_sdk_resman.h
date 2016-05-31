// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SDK_RESMAN_H
#define BAIDU_GALAXY_SDK_RESMAN_H

#include "ins_sdk.h"
#include "protocol/resman.pb.h"
#include "protocol/galaxy.pb.h"
#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

class RpcClient;

namespace sdk {

class ResourceManager {
public:
    ResourceManager() ;
    ~ResourceManager();
    bool GetStub();
    bool Login(const std::string& user, const std::string& password);
    bool EnterSafeMode(const EnterSafeModeRequest& request, EnterSafeModeResponse* response);
    bool LeaveSafeMode(const LeaveSafeModeRequest& request, LeaveSafeModeResponse* response);
    bool Status(const StatusRequest& request, StatusResponse* response);
    bool CreateContainerGroup(const CreateContainerGroupRequest& request, CreateContainerGroupResponse* response);
    bool RemoveContainerGroup(const RemoveContainerGroupRequest& request, RemoveContainerGroupResponse* response);
    bool UpdateContainerGroup(const UpdateContainerGroupRequest& request, UpdateContainerGroupResponse* response);
    bool ListContainerGroups(const ListContainerGroupsRequest& request, ListContainerGroupsResponse* response);
    bool ShowContainerGroup(const ShowContainerGroupRequest& request, ShowContainerGroupResponse* response);
    bool AddAgent(const AddAgentRequest& request, AddAgentResponse* response);
    bool RemoveAgent(const RemoveAgentRequest& request, RemoveAgentResponse* response);
    bool ShowAgent(const ShowAgentRequest& request, ShowAgentResponse* response);
    bool OnlineAgent(const OnlineAgentRequest& request, OnlineAgentResponse* response);
    bool OfflineAgent(const OfflineAgentRequest& request, OfflineAgentResponse* response);
    bool ListAgents(const ListAgentsRequest& request, ListAgentsResponse* response);
    bool CreateTag(const CreateTagRequest& request, CreateTagResponse* response);
    bool ListTags(const ListTagsRequest& request, ListTagsResponse* response);
    bool ListAgentsByTag(const ListAgentsByTagRequest& request, ListAgentsByTagResponse* response);
    bool GetTagsByAgent(const GetTagsByAgentRequest& request, GetTagsByAgentResponse* response);
    bool AddAgentToPool(const AddAgentToPoolRequest& request, AddAgentToPoolResponse* response);
    bool RemoveAgentFromPool(const RemoveAgentFromPoolRequest& request, RemoveAgentFromPoolResponse* response);
    bool ListAgentsByPool(const ListAgentsByPoolRequest& request, ListAgentsByPoolResponse* response);
    bool GetPoolByAgent(const GetPoolByAgentRequest& request, GetPoolByAgentResponse* response);
    bool AddUser(const AddUserRequest& request, AddUserResponse* response);
    bool RemoveUser(const RemoveUserRequest& request, RemoveUserResponse* response);
    bool ListUsers(const ListUsersRequest& request, ListUsersResponse* response);
    bool ShowUser(const ShowUserRequest& request, ShowUserResponse* response);
    bool GrantUser(const GrantUserRequest& request, GrantUserResponse* response);
    bool AssignQuota(const AssignQuotaRequest& request, AssignQuotaResponse* response);
    bool Preempt(const PreemptRequest& request, PreemptResponse* response);
private:
    ::galaxy::ins::sdk::InsSDK* nexus_;
    RpcClient* rpc_client_;
    ::baidu::galaxy::proto::ResMan_Stub* res_stub_;
    std::string full_key_;
}; //end class ResourceManager

} // end namespace sdk 
} // end namespace galaxy
} // end namespace baidu

#endif  // BAIDU_GALAXY_SDK_RESMAN_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
