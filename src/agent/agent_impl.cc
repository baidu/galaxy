// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "agent/agent_impl.h"

#include <gflags/gflags.h>
#include <util.h>
#include "rpc/rpc_client.h"

DECLARE_string(master_port);
DECLARE_string(agent_port);
DECLARE_string(master_host);
DECLARE_int32(agent_cpu_share);
DECLARE_int32(agent_mem_share);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() {
	self_address_ = common::util::GetLocalHostName() + ":" + FLAGS_agent_port;
	rpc_client_ = new RpcClient();
	if (!rpc_client_->GetStub(FLAGS_master_host + ":" + FLAGS_master_port, &master_stub_)) {
        assert(0);
    }
	thread_pool_.AddTask(boost::bind(&AgentImpl::HeartBeat, this));
}

void AgentImpl::HeartBeat() {
	HeartBeatRequest request;
	HeartBeatResponse response;
	request.set_endpoint(self_address_);
	rpc_client_->SendRequest(master_stub_, &Master_Stub::HeartBeat, &request, &response, 3, 1);
	thread_pool_.DelayTask(5000, boost::bind(&AgentImpl::HeartBeat, this));
}

void AgentImpl::Query(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::QueryRequest* request,
                       ::baidu::galaxy::QueryResponse* response,
                       ::google::protobuf::Closure* done) {
	response->set_status(kOk);
	AgentInfo* info = response->mutable_agent();
	info->set_endpoint(self_address_);
	info->set_state(kAlive);
	info->mutable_total()->set_millicores(FLAGS_agent_cpu_share);
	info->mutable_total()->set_memory(FLAGS_agent_mem_share);
	info->mutable_unassigned()->set_millicores(FLAGS_agent_cpu_share);
	info->mutable_unassigned()->set_memory(FLAGS_agent_mem_share);
	done->Run();
}

void AgentImpl::RunPod(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::RunPodRequest* request,
                       ::baidu::galaxy::RunPodResponse* response,
                       ::google::protobuf::Closure* done) {
	response->set_status(kOk);
	done->Run();

}

void AgentImpl::KillPod(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::KillPodRequest* request,
                       ::baidu::galaxy::KillPodResponse* response,
                       ::google::protobuf::Closure* done) {
	response->set_status(kOk);
	done->Run();
}

}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
