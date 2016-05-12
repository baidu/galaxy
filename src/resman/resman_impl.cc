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
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

DECLARE_string(nexus_root);
DECLARE_string(nexus_addr);
DECLARE_int32(agent_timeout);
DECLARE_int32(agent_query_interval);
DECLARE_int32(container_group_max_replica);
DECLARE_double(safe_mode_percent);

const std::string sAgentPrefix = "/agent";
const std::string sUserPrefix = "/user";
const std::string sContainerGroupPrefix = "/container_group";

namespace baidu {
namespace galaxy {

ResManImpl::ResManImpl() : scheduler_(new sched::Scheduler()),
                           safe_mode_(true) {
    nexus_ = new InsSDK(FLAGS_nexus_addr);
}

ResManImpl::~ResManImpl() {
    delete scheduler_;
    delete nexus_;
}

bool ResManImpl::Init() {
    bool load_ok = false;
    load_ok = LoadObjects(sAgentPrefix, agents_);
    if (!load_ok) {
        LOG(WARNING) << "fail to load agent meta";
        return false;
    }
    load_ok = LoadObjects(sUserPrefix, users_);
    if (!load_ok) {
        LOG(WARNING) << "fail to load user meta";
        return false;
    }
    load_ok = LoadObjects(sContainerGroupPrefix, container_groups_);
    if (!load_ok) {
        LOG(WARNING) << "failt to load container groups meta";
        return false;
    }
    std::map<std::string, proto::ContainerGroupMeta>::const_iterator it;
    for (it = container_groups_.begin(); it != container_groups_.end(); it++) {
        const proto::ContainerGroupMeta& container_group_meta = it->second;
        LOG(INFO) << "scheduler reaload " << container_group_meta.id();
        scheduler_->Reload(container_group_meta);
    }
    return true;
}

void ResManImpl::EnterSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::EnterSafeModeRequest* request,
                               ::baidu::galaxy::proto::EnterSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {
    MutexLock lock(&mu_);
    if (!safe_mode_) {
        safe_mode_ = true;
        scheduler_->Stop();
        response->mutable_error_code()->set_status(proto::kOk);
    } else {
        response->mutable_error_code()->set_status(proto::kError);
        response->mutable_error_code()->set_reason("already in safe mode");
    }
    done->Run();
}

void ResManImpl::LeaveSafeMode(::google::protobuf::RpcController* controller,
                               const ::baidu::galaxy::proto::LeaveSafeModeRequest* request,
                               ::baidu::galaxy::proto::LeaveSafeModeResponse* response,
                               ::google::protobuf::Closure* done) {
    MutexLock lock(&mu_);
    if (safe_mode_) {
        safe_mode_ = false;
        scheduler_->Start();
        response->mutable_error_code()->set_status(proto::kOk);
    } else {
        response->mutable_error_code()->set_status(proto::kError);
        response->mutable_error_code()->set_reason("invalid op, cluster is not in safe mode");
    }
    done->Run();
}

void ResManImpl::Status(::google::protobuf::RpcController* controller,
                        const ::baidu::galaxy::proto::StatusRequest* request,
                        ::baidu::galaxy::proto::StatusResponse* response,
                        ::google::protobuf::Closure* done) {

}

void ResManImpl::QueryAgent(const std::string& agent_endpoint, bool is_first_query) {
    MutexLock lock(&mu_);
    std::map<std::string, AgentStat>::iterator agent_it;
    agent_it = agent_stats_.find(agent_endpoint);
    if (agent_it == agent_stats_.end()) {
        LOG(WARNING) << "no need to query on expired agent: " << agent_endpoint;
        return;
    }
    AgentStat& agent = agent_it->second;
    int32_t now_tm = common::timer::now_time();
    if (agent.last_heartbeat_time + FLAGS_agent_timeout < now_tm) {
        LOG(INFO) << "this agent maybe dead:" << agent_endpoint;
        agent.status = proto::kAgentDead;
        scheduler_->RemoveAgent(agent_endpoint);
        query_pool_.DelayTask(FLAGS_agent_query_interval * 1000,
            boost::bind(&ResManImpl::QueryAgent, this, agent_endpoint, true)
        );
        return;
    }
    proto::Agent_Stub* stub;
    rpc_client_.GetStub(agent_endpoint, &stub);
    boost::scoped_ptr<proto::Agent_Stub> stub_guard(stub);
    boost::function<void (const proto::QueryRequest*, 
                          proto::QueryResponse*, bool, int)> callback;
    callback = boost::bind(&ResManImpl::QueryAgentCallback, this, 
                           agent_endpoint, is_first_query,
                           _1, _2, _3, _4);
    proto::QueryRequest* request = new proto::QueryRequest();
    request->set_full_report(is_first_query);
    proto::QueryResponse* response = new proto::QueryResponse();
    rpc_client_.AsyncRequest(stub, &proto::Agent_Stub::Query,
                             request, response, callback, 5, 1);
    VLOG(10) << "send query command to:" << agent_endpoint;
}

void ResManImpl::QueryAgentCallback(std::string agent_endpoint,
                                    bool is_first_query,
                                    const proto::QueryRequest* request,
                                    proto::QueryResponse* response,
                                    bool rpc_fail, int err) {
    boost::scoped_ptr<const proto::QueryRequest> request_guard(request);
    boost::scoped_ptr<proto::QueryResponse> response_guard(response);
    if (response->code().status() != proto::kOk || rpc_fail) {
        LOG(WARNING) << "failed to query on: " << agent_endpoint
                     << " err: " << err << ", rpc_fail:" << rpc_fail;
        query_pool_.DelayTask(FLAGS_agent_query_interval,
            boost::bind(&ResManImpl::QueryAgent, this, agent_endpoint, is_first_query)
        );
        return;
    }
    if (is_first_query) {
        MutexLock lock(&mu_);
        std::map<std::string, proto::AgentMeta>::iterator agent_it 
            = agents_.find(agent_endpoint);
        if (agent_it == agents_.end()) {
            LOG(WARNING) << "query result for expired agent:" << agent_endpoint;
            return;
        }
        proto::AgentMeta& agent_meta = agent_it->second;
        const proto::AgentInfo& agent_info = response->agent_info();
        int64_t cpu = agent_info.cpu_resource().total();
        int64_t memory = agent_info.memory_resource().total();
        std::map<sched::DevicePath, sched::VolumInfo> volums;
        for (int i = 0; i < agent_info.volum_resources_size(); i++) {
            const proto::VolumResource& vres = agent_info.volum_resources(i);
            sched::VolumInfo& vinfo = volums[vres.device_path()];
            vinfo.size = vres.volum().total();
            vinfo.medium = vres.medium();
        }
        std::set<std::string> tags;
        for (int i = 0; i < agent_meta.tags_size(); i++) {
            tags.insert(agent_meta.tags(i));
        }
        std::string pool_name = agent_meta.pool();
        sched::Agent::Ptr agent(new sched::Agent(agent_endpoint,
                                                 cpu, memory,
                                                 volums,
                                                 tags,
                                                 pool_name));
        scheduler_->RemoveAgent(agent_endpoint);
        scheduler_->AddAgent(agent, agent_info);
        LOG(INFO) << "TRACE BEGIN, first query result from:" << agent_endpoint
                  << "\n" << agent_info.DebugString()
                  << "\nTRACE END";
    } else {
        VLOG(10) << "TRACE BEGIN, query result from: " << agent_endpoint
                 << "\n" << response->agent_info().DebugString()
                 << "\nTRACE END";
        std::vector<sched::AgentCommand> commands;
        scheduler_->MakeCommand(agent_endpoint, response->agent_info(), commands);
        SendCommandsToAgent(agent_endpoint, commands);
    }

    bool leave_safe_mode_event = false;
    {
        MutexLock lock(&mu_);
        if (agent_stats_.find(agent_endpoint) == agent_stats_.end()) {
            LOG(INFO) << "this agent may be removed, no need to query again";
            return;
        }
        agent_stats_[agent_endpoint].info = response->agent_info();
        if (safe_mode_ && 
            agent_stats_.size() > (double)agents_.size() * FLAGS_safe_mode_percent) {
            LOG(INFO) << "leave safe mode";
            safe_mode_ = false;
            leave_safe_mode_event = true;
        }
    }
    if (leave_safe_mode_event) {
        scheduler_->Start();
    }
    //query again later
    query_pool_.DelayTask(FLAGS_agent_query_interval,
        boost::bind(&ResManImpl::QueryAgent, this, agent_endpoint, is_first_query)
    );
}

void ResManImpl::SendCommandsToAgent(const std::string& agent_endpoint,
                                     const std::vector<sched::AgentCommand>& commands) {
    std::vector<sched::AgentCommand>::const_iterator it;
    for (it = commands.begin(); it != commands.end(); it++) {
        const sched::AgentCommand& cmd = *it;
        proto::Agent_Stub* stub;
        rpc_client_.GetStub(agent_endpoint, &stub);
        boost::scoped_ptr<proto::Agent_Stub> stub_guard(stub);
        if (cmd.action == sched::kCreateContainer) {
            proto::CreateContainerRequest* request = new proto::CreateContainerRequest();
            proto::CreateContainerResponse* response = new proto::CreateContainerResponse();
            request->set_id(cmd.container_id);
            request->mutable_container()->CopyFrom(cmd.desc);
            boost::function<void (const proto::CreateContainerRequest*,
                                  proto::CreateContainerResponse*,
                                  bool, int)> callback;
            callback = boost::bind(&ResManImpl::CreateContainerCallback, this,
                                   agent_endpoint, _1, _2, _3, _4);
            rpc_client_.AsyncRequest(stub, &proto::Agent_Stub::CreateContainer,
                                     request, response, callback, 5, 1);
            LOG(INFO) << "send create command, container: "
                      << cmd.container_id << ", agent:"
                      << agent_endpoint;
            VLOG(10) << "TRACE BEGIN container: " << cmd.container_id;
            VLOG(10) <<  cmd.desc.DebugString();
            VLOG(10) << "TRACE END";
        } else if (cmd.action == sched::kDestroyContainer) {
            proto::RemoveContainerRequest* request = new proto::RemoveContainerRequest();
            proto::RemoveContainerResponse* response = new proto::RemoveContainerResponse();
            request->set_id(cmd.container_id);
            boost::function<void (const proto::RemoveContainerRequest*,
                                  proto::RemoveContainerResponse*,
                                  bool, int)> callback;
            callback = boost::bind(&ResManImpl::RemoveContainerCallback, this,
                                   agent_endpoint, _1, _2, _3, _4);
            rpc_client_.AsyncRequest(stub, &proto::Agent_Stub::RemoveContainer,
                                     request, response, callback, 5, 1);
            LOG(INFO) << "send remove command, container: "
                      << cmd.container_id << ", agent:"
                      << agent_endpoint;
        }
    }
}

void ResManImpl::KeepAlive(::google::protobuf::RpcController* controller,
                           const ::baidu::galaxy::proto::KeepAliveRequest* request,
                           ::baidu::galaxy::proto::KeepAliveResponse* response,
                           ::google::protobuf::Closure* done) {
    MutexLock lock(&mu_);
    const std::string agent_ep = request->endpoint();
    bool agent_is_registered = false;
    bool agent_first_heartbeat = false;

    if (agents_.find(agent_ep) != agents_.end()) {
        agent_is_registered = true;
    }
    if (agent_stats_.find(agent_ep) == agent_stats_.end()) {
        agent_first_heartbeat = true;
    }
    if (!agent_is_registered) {
        LOG(WARNING) << "this agent is not registered, please check: " << agent_ep;
        done->Run();
        return;
    }
    if (agent_first_heartbeat) {
        LOG(INFO) << "first heartbeat of: " << agent_ep;
    }
    AgentStat agent = agent_stats_[agent_ep];
    if (agent.status != proto::kAgentOffline) {
        agent.status = proto::kAgentAlive;
    }
    agent.last_heartbeat_time = common::timer::now_time();
    if (agent_first_heartbeat) {
        query_pool_.AddTask(
            boost::bind(&ResManImpl::QueryAgent, this, agent_ep, true)
        );
    }
    done->Run();
}

void ResManImpl::CreateContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::CreateContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::CreateContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {
    proto::ContainerGroupMeta container_group_meta;
    container_group_meta.set_name(request->name());
    container_group_meta.set_user_name(request->user().user());
    container_group_meta.set_replica(request->replica());
    container_group_meta.set_status(proto::kContainerGroupNormal);
    container_group_meta.mutable_desc()->CopyFrom(request->desc());
    std::string container_group_id = scheduler_->Submit(request->name(), 
                                                        request->desc(), 
                                                        request->replica(), 
                                                        request->desc().priority());
    if (container_group_id == "") {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kCreateContainerGroupFail);
        err->set_reason("container group id conflict");
        done->Run();
        return;
    }
    container_group_meta.set_id(container_group_id);
    bool ret = SaveObject(sContainerGroupPrefix + "/" + container_group_id, 
                          container_group_meta);
    if (!ret) {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kCreateContainerGroupFail);
        err->set_reason("fail to save container group meta in nexus");
        scheduler_->Kill(container_group_id);
    } else {
        response->mutable_error_code()->set_status(proto::kOk);
        response->set_id(container_group_id);
        {
            MutexLock lock(&mu_);
            container_groups_[container_group_id] = container_group_meta;
        }
    }
    done->Run();
}

void ResManImpl::RemoveContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::RemoveContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::RemoveContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {

    proto::ContainerGroupMeta new_meta;
    {
        MutexLock lock(&mu_);
        std::map<std::string, proto::ContainerGroupMeta>::iterator it;
        it = container_groups_.find(request->id());
        if (it == container_groups_.end()) {
            response->mutable_error_code()->set_status(proto::kRemoveContainerGroupFail);
            response->mutable_error_code()->set_reason("no such group");
            done->Run();
            return;
        }
        new_meta = it->second;
        new_meta.set_status(proto::kContainerGroupTerminated);
    }
    
    bool ret = SaveObject(sContainerGroupPrefix + "/" + new_meta.id(), 
                          new_meta);
    if (!ret) {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kRemoveContainerGroupFail);
        err->set_reason("fail to save container group meta in nexus");
    } else {
        {
            MutexLock lock(&mu_);
            container_groups_[new_meta.id()] = new_meta;
        }
        scheduler_->Kill(new_meta.id());
        response->mutable_error_code()->set_status(proto::kOk);
    }
    done->Run();
}

void ResManImpl::UpdateContainerGroup(::google::protobuf::RpcController* controller,
                                      const ::baidu::galaxy::proto::UpdateContainerGroupRequest* request,
                                      ::baidu::galaxy::proto::UpdateContainerGroupResponse* response,
                                      ::google::protobuf::Closure* done) {
    proto::ContainerGroupMeta new_meta;
    bool replica_changed = false;
    {
        MutexLock lock(&mu_);
        std::map<std::string, proto::ContainerGroupMeta>::iterator it;
        it = container_groups_.find(request->id());
        if (it == container_groups_.end()) {
            response->mutable_error_code()->set_status(proto::kUpdateContainerGroupFail);
            response->mutable_error_code()->set_reason("no such group");
            done->Run();
            return;
        }
        if (request->replica() > (size_t)FLAGS_container_group_max_replica) {
            response->mutable_error_code()->set_status(proto::kUpdateContainerGroupFail);
            response->mutable_error_code()->set_reason("invalid replica");
            done->Run();
            return;
        }
        new_meta = it->second;
        new_meta.set_update_interval(request->interval());
        new_meta.mutable_desc()->CopyFrom(request->desc());
        if (new_meta.replica() != request->replica()) {
            new_meta.set_replica(request->replica());
            replica_changed = true;
        }
    }
    std::string new_version;
    bool version_changed = scheduler_->Update(new_meta.id(),
                                              new_meta.desc(),
                                              new_meta.update_interval(),
                                              new_version);
    if (version_changed) {
        new_meta.mutable_desc()->set_version(new_version);
    }
    if (replica_changed) {
        scheduler_->ChangeReplica(new_meta.id(),
                                  new_meta.replica());
    }
    {
        MutexLock lock(&mu_);
        container_groups_[new_meta.id()] = new_meta;
    }
    bool save_ok = SaveObject(sContainerGroupPrefix + "/" + new_meta.id(), 
                              new_meta);
    if (!save_ok) {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kUpdateContainerGroupFail);
        err->set_reason("fail to save container group meta in nexus");    
    } else {
        response->mutable_error_code()->set_status(proto::kOk);
    }
    done->Run();
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
    proto::AgentMeta agent_meta;
    agent_meta.set_endpoint(request->endpoint());
    agent_meta.set_pool(request->pool());
    bool ret = SaveObject(sAgentPrefix + "/" + agent_meta.endpoint(), agent_meta);
    if (!ret) {
        proto::ErrorCode* err = response->mutable_error_code();
        err->set_status(proto::kAddAgentFail);
        err->set_reason("fail to save agent in nexus");
    } else {
        {
            MutexLock lock(&mu_);
            agents_[agent_meta.endpoint()] = agent_meta;
        }
        response->mutable_error_code()->set_status(proto::kOk);
    }
    done->Run();
}

void ResManImpl::RemoveAgent(::google::protobuf::RpcController* controller,
                             const ::baidu::galaxy::proto::RemoveAgentRequest* request,
                             ::baidu::galaxy::proto::RemoveAgentResponse* response,
                             ::google::protobuf::Closure* done) {
    LOG(INFO) << "remove agent:" << request->endpoint();
    MutexLock lock(&mu_);
    if (agents_.find(request->endpoint()) == agents_.end()) {
        response->mutable_error_code()->set_status(proto::kError);
        response->mutable_error_code()->set_reason("agent not exist");
        done->Run();
        return;
    }
    agents_.erase(request->endpoint());
    agent_stats_.erase(request->endpoint());
    scheduler_->RemoveAgent(request->endpoint());
    bool remove_ok = RemoveObject(sAgentPrefix + "/" + request->endpoint());
    if (!remove_ok) {
        response->mutable_error_code()->set_status(proto::kError);
        response->mutable_error_code()->set_reason("fail to delete meta from nexus");
    } else {
        response->mutable_error_code()->set_status(proto::kOk);
    }
    done->Run();
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

bool ResManImpl::RemoveObject(const std::string& key) {
    ::galaxy::ins::sdk::SDKError err;
    std::string full_key = FLAGS_nexus_root + key;
    bool ret = nexus_->Delete(full_key, &err);
    if (!ret) {
        LOG(WARNING) << "nexus error:" << err;
    }
    return ret;
}

template <class ProtoClass> 
bool ResManImpl::SaveObject(const std::string& key,
                            const ProtoClass& obj) {
    std::string raw_buf;
    if (!obj.SerializeToString(&raw_buf)) {
        LOG(WARNING) << "save object to protobuf fail";
        return false;
    }
    ::galaxy::ins::sdk::SDKError err;
    std::string full_key = FLAGS_nexus_root + key;
    bool ret = nexus_->Put(full_key, raw_buf, &err);
    if (!ret) {
        LOG(WARNING) << "nexus error: " << err;
    }
    return ret;
}

template <class ProtoClass>
bool ResManImpl::LoadObjects(const std::string& prefix,
                             std::map<std::string, ProtoClass>& objs) {
    std::string full_prefix = FLAGS_nexus_root + prefix;
    ::galaxy::ins::sdk::ScanResult* result  
        = nexus_->Scan(full_prefix + "/", full_prefix + "/\xff");
    boost::scoped_ptr< ::galaxy::ins::sdk::ScanResult > result_guard(result);
    size_t prefix_len = full_prefix.size() + 1;
    while (!result->Done()) {
        const std::string& full_key = result->Key();
        const std::string& raw_obj_buf = result->Value();
        LOG(INFO) << "try load " << full_key;
        std::string key = full_key.substr(prefix_len);
        ProtoClass& obj = objs[key];
        bool parse_ok = obj.ParseFromString(raw_obj_buf);
        if (!parse_ok) {
            LOG(WARNING) << "parse protobuf object fail ";
            return false;
        }
    }
    return result->Error() == ::galaxy::ins::sdk::kOK;
}

void ResManImpl::CreateContainerCallback(std::string agent_endpoint,
                                         const proto::CreateContainerRequest* request,
                                         proto::CreateContainerResponse* response,
                                         bool fail, int err) {
    boost::scoped_ptr<const proto::CreateContainerRequest> request_guard(request);
    boost::scoped_ptr<proto::CreateContainerResponse> response_guard(response);
    if (fail) {
        LOG(WARNING) << "rpc fail of creating container, err: " << err
                     << ", agent: " << agent_endpoint; 
        return;
    }
    if (response->code().status() != proto::kOk) {
        LOG(WARNING) << "fail to create contaienr, reason:" 
                     << response->code().reason()
                     << ", agent:" << agent_endpoint;
        return;
    }
}

void ResManImpl::RemoveContainerCallback(std::string agent_endpoint,
                                         const proto::RemoveContainerRequest* request,
                                         proto::RemoveContainerResponse* response,
                                         bool fail, int err) {
    boost::scoped_ptr<const proto::RemoveContainerRequest> request_guard(request);
    boost::scoped_ptr<proto::RemoveContainerResponse> response_guard(response);
    if (fail) {
        LOG(WARNING) << "rpc fail of remote container, err: " << err
                     << ", agent:" << agent_endpoint;
        return;
    }
    if (response->code().status() != proto::kOk) {
        LOG(WARNING) << "fail to create contaienr, reason:" 
                     << response->code().reason()
                     << ", agent:" << agent_endpoint;
        return;
    }
}

} //namespace galaxy
} //namespace baidu
