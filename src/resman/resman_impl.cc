// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "resman_impl.h"
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/utsname.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>

DECLARE_string(nexus_root);
DECLARE_string(nexus_addr);

const std::string sAgentPrefix = "/agent";
const std::string sUserPrefix = "/user";

namespace baidu {
namespace galaxy {

ResManImpl::ResManImpl() : scheduler_(new sched::Scheduler()) {
    scheduler_->Start();
    nexus_ = new InsSDK(FLAGS_nexus_addr);
}

ResManImpl::~ResManImpl() {
    delete scheduler_;
    delete nexus_;
}

void ResManImpl::EnterSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::EnterSafeModeRequest* request,
                               ::baidu::galaxy::proto::EnterSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {

}

void ResManImpl::LeaveSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::LeaveSafeModeRequest* request,
                               ::baidu::galaxy::proto::LeaveSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {

}

void ResManImpl::Status(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::proto::StatusRequest* request,
                        ::baidu::galaxy::proto::StatusResponse* response,
                        ::google::protobuf::Closure* done) {

}

void ResManImpl::KeepAlive(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::proto::KeepAliveRequest* request,
                           ::baidu::galaxy::proto::KeepAliveResponse* response,
                           ::google::protobuf::Closure* done) {

}

void ResManImpl::CreateContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::CreateContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::CreateContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {

}

void ResManImpl::RemoveContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::RemoveContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::RemoveContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {

}

void ResManImpl::UpdateContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::UpdateContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::UpdateContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {

}

void ResManImpl::ListContainerGroups(::google::protobuf::RpcController* controller,
                                     const ::baidu::galaxy::proto::ListContainerGroupsRequest* request,
                                     ::baidu::galaxy::proto::ListContainerGroupsResponse* response,
                                     ::google::protobuf::Closure* done) {

}

void ResManImpl::ShowContainerGroup(::google::protobuf::RpcController* controller,
                                    const ::baidu::galaxy::proto::ShowContainerGroupRequest* request,
                                    ::baidu::galaxy::proto::ShowContainerGroupResponse* response,
                                    ::google::protobuf::Closure* done) {

}

void ResManImpl::AddAgent(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::proto::AddAgentRequest* request,
                          ::baidu::galaxy::proto::AddAgentResponse* response,
                          ::google::protobuf::Closure* done) {
    proto::AgentData agent_data;
    agent_data.set_endpoint(request->endpoint());
    agent_data.set_pool(request->pool());
    bool ret = SaveAgentData(agent_data.endpoint(), agent_data);
    if (!ret) {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kAddAgentFail);
        err->set_reason("fail to save agent in nexus");
    } else {
        {
            MutexLock lock(&mu_);
            agents_[agent_data.endpoint()] = agent_data;
        }
        response->mutable_error_code()->set_status(proto::kOk);
    }
    done->Run();
}

void ResManImpl::RemoveAgent(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::proto::RemoveAgentRequest* request,
                             ::baidu::galaxy::proto::RemoveAgentResponse* response,
                             ::google::protobuf::Closure* done) {

}

void ResManImpl::OnlineAgent(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::proto::OnlineAgentRequest* request,
                             ::baidu::galaxy::proto::OnlineAgentResponse* response,
                             ::google::protobuf::Closure* done) {

}

void ResManImpl::OfflineAgent(::google::protobuf::RpcController* controller,
                              const ::baidu::galaxy::proto::OfflineAgentRequest* request,
                              ::baidu::galaxy::proto::OfflineAgentResponse* response,
                              ::google::protobuf::Closure* done) {

}

void ResManImpl::ListAgents(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::ListAgentsRequest* request,
                            ::baidu::galaxy::proto::ListAgentsResponse* response,
                            ::google::protobuf::Closure* done) {

}

void ResManImpl::CreateTag(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::proto::CreateTagRequest* request,
                           ::baidu::galaxy::proto::CreateTagResponse* response,
                           ::google::protobuf::Closure* done) {

}

void ResManImpl::ListTags(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::proto::ListTagsRequest* request,
                          ::baidu::galaxy::proto::ListTagsResponse* response,
                          ::google::protobuf::Closure* done) {

}

void ResManImpl::ListAgentsByTag(::google::protobuf::RpcController* controller,
                                 const ::baidu::galaxy::proto::ListAgentsByTagRequest* request,
                                 ::baidu::galaxy::proto::ListAgentsByTagResponse* response,
                                 ::google::protobuf::Closure* done) {

}

void ResManImpl::GetTagsByAgent(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::proto::GetTagsByAgentRequest* request,
                                ::baidu::galaxy::proto::GetTagsByAgentResponse* response,
                                ::google::protobuf::Closure* done) {

}

void ResManImpl::AddAgentToPool(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::proto::AddAgentToPoolRequest* request,
                                ::baidu::galaxy::proto::AddAgentToPoolResponse* response,
                                ::google::protobuf::Closure* done) {

}

void ResManImpl::RemoveAgentFromPool(::google::protobuf::RpcController* controller,
                                     const ::baidu::galaxy::proto::RemoveAgentFromPoolRequest* request,
                                     ::baidu::galaxy::proto::RemoveAgentFromPoolResponse* response,
                                     ::google::protobuf::Closure* done) {

}

void ResManImpl::ListAgentsByPool(::google::protobuf::RpcController* controller,
                                  const ::baidu::galaxy::proto::ListAgentsByPoolRequest* request,
                                  ::baidu::galaxy::proto::ListAgentsByPoolResponse* response,
                                  ::google::protobuf::Closure* done) {

}

void ResManImpl::GetPoolByAgent(::google::protobuf::RpcController* controller,
                                const ::baidu::galaxy::proto::GetPoolByAgentRequest* request,
                                ::baidu::galaxy::proto::GetPoolByAgentResponse* response,
                                ::google::protobuf::Closure* done) {

}

void ResManImpl::AddUser(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::proto::AddUserRequest* request,
                        ::baidu::galaxy::proto::AddUserResponse* response,
                        ::google::protobuf::Closure* done) {

}

void ResManImpl::RemoveUser(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::proto::RemoveUserRequest* request,
                            ::baidu::galaxy::proto::RemoveUserResponse* response,
                            ::google::protobuf::Closure* done) {

}

void ResManImpl::ListUsers(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::proto::ListUsersRequest* request,
                           ::baidu::galaxy::proto::ListUsersResponse* response,
                           ::google::protobuf::Closure* done) {

}

void ResManImpl::ShowUser(::google::protobuf::RpcController* controller,
                          const ::baidu::galaxy::proto::ShowUserRequest* request,
                          ::baidu::galaxy::proto::ShowUserResponse* response,
                          ::google::protobuf::Closure* done) {

}

void ResManImpl::GrantUser(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::proto::GrantUserRequest* request,
                           ::baidu::galaxy::proto::GrantUserResponse* response,
                           ::google::protobuf::Closure* done) {

}

void ResManImpl::AssignQuota(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::proto::AssignQuotaRequest* request,
                             ::baidu::galaxy::proto::AssignQuotaResponse* response,
                             ::google::protobuf::Closure* done) {

}

bool ResManImpl::SaveAgentData(const std::string& endpoint, 
                               const proto::AgentData& agent) {
    std::stringstream ss;
    agent.SerializeToOstream(&ss);
    ::galaxy::ins::sdk::SDKError err;
    std::string key = FLAGS_nexus_root + sAgentPrefix + "/" + endpoint;
    bool ret = nexus_->Put(key, ss.str(), &err);
    if (!ret) {
        LOG(WARNING) << "nexus error: " << err;
    }
    return ret;
}

bool ResManImpl::SaveUserData(const std::string& user_name, 
                              const proto::UserData& user) {
    std::stringstream ss;
    user.SerializeToOstream(&ss);
    ::galaxy::ins::sdk::SDKError err;
    std::string key = FLAGS_nexus_root + sAgentPrefix + "/" + user_name;
    bool ret = nexus_->Put(key , ss.str(), &err);
    if (!ret) {
        LOG(WARNING) << "nexus error: " << err;
    }
    return ret;
}

}
}
