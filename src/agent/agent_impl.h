// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  GALAXY_AGENT_IMPL_H_
#define  GALAXY_AGENT_IMPL_H_

#include <string>
#include <thread_pool.h>

#include "proto/agent.pb.h"
#include "proto/master.pb.h"

namespace baidu {
namespace galaxy {

class RpcClient;

class AgentImpl : public Agent {
public:
	AgentImpl();
	~AgentImpl() {}
	void Query(::google::protobuf::RpcController* controller,
	                   const ::baidu::galaxy::QueryRequest* request,
	                   ::baidu::galaxy::QueryResponse* response,
	                   ::google::protobuf::Closure* done);
	void RunPod(::google::protobuf::RpcController* controller,
	                   const ::baidu::galaxy::RunPodRequest* request,
	                   ::baidu::galaxy::RunPodResponse* response,
	                   ::google::protobuf::Closure* done);
	void KillPod(::google::protobuf::RpcController* controller,
	                   const ::baidu::galaxy::KillPodRequest* request,
	                   ::baidu::galaxy::KillPodResponse* response,
	                   ::google::protobuf::Closure* done);
	void HeartBeat();
private:
	common::ThreadPool thread_pool_;
	Master_Stub* master_stub_;
	RpcClient* rpc_client_;
	std::string self_address_;
};

}
}

#endif  //__AGENT_IMPL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
