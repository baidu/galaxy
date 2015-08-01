// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "agent/agent_impl.h"

#include <gflags/gflags.h>
#include "rpc/rpc_client.h"

DECLARE_string(master_port);
DECLARE_string(master_host);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() {
	rpc_client_ = new RpcClient();
	if (!rpc_client_->GetStub(FLAGS_master_host + ":" + FLAGS_master_port, &master_stub_)) {
        assert(0);
    }
	thread_pool_.AddTask(boost::bind(&AgentImpl::HeartBeat, this));
}

void AgentImpl::HeartBeat() {
	HeartBeatRequest request;
	HeartBeatResponse response;
	rpc_client_->SendRequest(master_stub_, &Master_Stub::HeartBeat, &request, &response, 3, 1);
	thread_pool_.DelayTask(5000, boost::bind(&AgentImpl::HeartBeat, this));
}

void AgentImpl::Query(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::QueryRequest* request,
                       ::baidu::galaxy::QueryResponse* response,
                       ::google::protobuf::Closure* done) {
	done->Run();

}

void AgentImpl::RunPod(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::RunPodRequest* request,
                       ::baidu::galaxy::RunPodResponse* response,
                       ::google::protobuf::Closure* done) {
	done->Run();

}

void AgentImpl::KillPod(::google::protobuf::RpcController* controller,
                       const ::baidu::galaxy::KillPodRequest* request,
                       ::baidu::galaxy::KillPodResponse* response,
                       ::google::protobuf::Closure* done) {
	done->Run();
}

}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
