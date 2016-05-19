// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "agent_impl.h"
#include "rpc/rpc_client.h"
#include "boost/thread/mutex.hpp"
#include "protocol/resman.pb.h"

#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/utsname.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <snappy.h>

DECLARE_string(agent_ip);
DECLARE_string(agent_port);
DECLARE_int32(keepalive_interval);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() :
    heartbeat_pool_(1),
    master_rpc_(new baidu::galaxy::RpcClient()),
    rm_watcher_(new baidu::galaxy::MasterWatcher()),
    resman_stub_(NULL),
    agent_endpoint_(FLAGS_agent_ip + ":" + FLAGS_agent_port),
    running_(false)
{
}

AgentImpl::~AgentImpl()
{
    running_ = false;
}

void AgentImpl::HandleMasterChange(const std::string& new_master_endpoint)
{
    if (new_master_endpoint.empty()) {
        LOG(WARNING) << "endpoint of RM is deleted from nexus";
    }
    if (new_master_endpoint != master_endpoint_) {
        boost::mutex::scoped_lock lock(rpc_mutex_);
        LOG(INFO) << "RM changes to " << new_master_endpoint.c_str();
        master_endpoint_ = new_master_endpoint;

        if (resman_stub_) {
            delete resman_stub_;
            resman_stub_ = NULL;
        }

        if (!master_rpc_->GetStub(master_endpoint_, &resman_stub_)) {
            LOG(WARNING) << "connect RM failed: " << master_endpoint_.c_str();
            return;
        }
    }
}

void AgentImpl::Setup()
{
    running_ = true;
    if (!rm_watcher_->Init(boost::bind(&AgentImpl::HandleMasterChange, this, _1))) {
        LOG(FATAL) << "init res manager watch failed, agent will exit ...";
        exit(1);
    }
    heartbeat_pool_.AddTask(boost::bind(&AgentImpl::KeepAlive, this, FLAGS_keepalive_interval));
}

void AgentImpl::KeepAlive(int internal_ms)
{

    baidu::galaxy::proto::KeepAliveRequest request;
    baidu::galaxy::proto::KeepAliveResponse response;
    request.set_endpoint(agent_endpoint_);

    boost::mutex::scoped_lock lock(rpc_mutex_);
    if (!master_rpc_->SendRequest(resman_stub_,
            &galaxy::proto::ResMan_Stub::KeepAlive,
            &request,
            &response,
            5,
            1)) {
        LOG(WARNING) << "keep alive failed";
    }

    heartbeat_pool_.DelayTask(internal_ms, boost::bind(&AgentImpl::KeepAlive, this, internal_ms));
}

void AgentImpl::CreateContainer(::google::protobuf::RpcController* controller,
        const ::baidu::galaxy::proto::CreateContainerRequest* request,
        ::baidu::galaxy::proto::CreateContainerResponse* response,
        ::google::protobuf::Closure* done)
{
    //
}


void AgentImpl::RemoveContainer(::google::protobuf::RpcController* controller,
        const ::baidu::galaxy::proto::RemoveContainerRequest* request,
        ::baidu::galaxy::proto::RemoveContainerResponse* response,
        ::google::protobuf::Closure* done)
{
}


void AgentImpl::ListContainers(::google::protobuf::RpcController* controller,
        const ::baidu::galaxy::proto::ListContainersRequest* request,
        ::baidu::galaxy::proto::ListContainersResponse* response,
        ::google::protobuf::Closure* done)
{
}


void AgentImpl::Query(::google::protobuf::RpcController* controller,
        const ::baidu::galaxy::proto::QueryRequest* request,
        ::baidu::galaxy::proto::QueryResponse* response,
        ::google::protobuf::Closure* done)
{
}


}
}
