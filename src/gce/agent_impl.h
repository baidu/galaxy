// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AGENT_IMPL_H_
#define AGENT_IMPL_H_

#include <boost/noncopyable.hpp>
#include "proto/agent.pb.h"

namespace baidu {
namespace galaxy {

class AgentImpl : public Agent, 
                  boost::noncopyable {
public:
    AgentImpl(); 

    ~AgentImpl(); 

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
};

} // ending namespace galaxy
} // ending namespace baidu

#endif
