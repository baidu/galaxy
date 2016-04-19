// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <string>
#include <map>
#include <set>

#include "src/protocol/resman.pb.h"
#include "scheduler.h"

namespace baidu {
namespace galaxy {

class ResManImpl : public baidu::galaxy::proto::ResMan {
public:
    ResManImpl();
    ~ResManImpl();
    void EnterSafeMode(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::proto::EnterSafeModeRequest* request,
                       ::baidu::galaxy::proto::EnterSafeModeResponse* response,
                       ::google::protobuf::Closure* done);

    void LeaveSafeMode(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::proto::LeaveSafeModeRequest* request,
                       ::baidu::galaxy::proto::LeaveSafeModeResponse* response,
                       ::google::protobuf::Closure* done);

    void Status(::google::protobuf::RpcController* controller,
                const ::baidu::galaxy::proto::StatusRequest* request,
                ::baidu::galaxy::proto::StatusResponse* response,
                ::google::protobuf::Closure* done);

    void KeepAlive(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::KeepAliveRequest* request,
                   ::baidu::galaxy::proto::KeepAliveResponse* response,
                   ::google::protobuf::Closure* done);

    void AddAgent(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::AddAgentRequest* request,
                  ::baidu::galaxy::proto::AddAgentResponse* response,
                  ::google::protobuf::Closure* done);

    void RemoveAgent(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::proto::RemoveAgentRequest* request,
                     ::baidu::galaxy::proto::RemoveAgentResponse* response,
                     ::google::protobuf::Closure* done);

    void OnlineAgent(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::proto::OnlineAgentRequest* request,
                     ::baidu::galaxy::proto::OnlineAgentResponse* response,
                     ::google::protobuf::Closure* done);

    void OfflineAgent(::google::protobuf::RpcController* controller,
                      const ::baidu::galaxy::proto::OfflineAgentRequest* request,
                      ::baidu::galaxy::proto::OfflineAgentResponse* response,
                      ::google::protobuf::Closure* done);

    void ListAgents(::google::protobuf::RpcController* controller,
                    const ::baidu::galaxy::proto::ListAgentsRequest* request,
                    ::baidu::galaxy::proto::ListAgentsResponse* response,
                    ::google::protobuf::Closure* done);

    void CreateTag(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::CreateTagRequest* request,
                   ::baidu::galaxy::proto::CreateTagResponse* response,
                   ::google::protobuf::Closure* done);

    void RemoveTag(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::RemoveTagRequest* request,
                   ::baidu::galaxy::proto::RemoveTagResponse* response,
                   ::google::protobuf::Closure* done);

    void ListTags(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::ListTagsRequest* request,
                  ::baidu::galaxy::proto::ListTagsResponse* response,
                  ::google::protobuf::Closure* done);

    void AddAgentToTag(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::proto::AddAgentToTagRequest* request,
                       ::baidu::galaxy::proto::AddAgentToTagResponse* response,
                       ::google::protobuf::Closure* done);

    void RemoveAgentFromTag(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::RemoveAgentFromTagRequest* request,
                            ::baidu::galaxy::proto::RemoveAgentFromTagResponse* response,
                            ::google::protobuf::Closure* done);

    void ListAgentsByTag(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::proto::ListAgentsByTagRequest* request,
                         ::baidu::galaxy::proto::ListAgentsByTagResponse* response,
                         ::google::protobuf::Closure* done);

    void AddUser(::google::protobuf::RpcController* controller,
                 const ::baidu::galaxy::proto::AddUserRequest* request,
                 ::baidu::galaxy::proto::AddUserResponse* response,
                 ::google::protobuf::Closure* done);

    void RemoveUser(::google::protobuf::RpcController* controller,
                    const ::baidu::galaxy::proto::RemoveUserRequest* request,
                    ::baidu::galaxy::proto::RemoveUserResponse* response,
                    ::google::protobuf::Closure* done);

    void ListUsers(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::ListUsersRequest* request,
                   ::baidu::galaxy::proto::ListUsersResponse* response,
                   ::google::protobuf::Closure* done);

    void ShowUser(::google::protobuf::RpcController* controller,
                  const ::baidu::galaxy::proto::ShowUserRequest* request,
                  ::baidu::galaxy::proto::ShowUserResponse* response,
                  ::google::protobuf::Closure* done);

    void GrantUser(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::proto::GrantUserRequest* request,
                   ::baidu::galaxy::proto::GrantUserResponse* response,
                   ::google::protobuf::Closure* done);

    void AssignQuota(::google::protobuf::RpcController* controller,
                     const ::baidu::galaxy::proto::AssignQuotaRequest* request,
                     ::baidu::galaxy::proto::AssignQuotaResponse* response,
                     ::google::protobuf::Closure* done);

private:
    sched::Scheduler* scheduler_;
};

}
}

