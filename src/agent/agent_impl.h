// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <string>
#include <map>
#include <set>

#include "src/protocol/agent.pb.h"
#include "master_watcher.h"
#include "thread_pool.h"
#include "rpc/rpc_client.h"
#include "boost/scoped_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "resource/resource_manager.h"
#include "container/container.h"
#include "container/container_manager.h"
#include "health/healthy_checker.h"

namespace baidu {
namespace galaxy {

namespace proto {
class ResMan_Stub;
}

class AgentImpl : public baidu::galaxy::proto::Agent {
public:

    AgentImpl();
    virtual ~AgentImpl();

    void Setup();

    void CreateContainer(::google::protobuf::RpcController* controller,
            const ::baidu::galaxy::proto::CreateContainerRequest* request,
            ::baidu::galaxy::proto::CreateContainerResponse* response,
            ::google::protobuf::Closure* done);

    void RemoveContainer(::google::protobuf::RpcController* controller,
            const ::baidu::galaxy::proto::RemoveContainerRequest* request,
            ::baidu::galaxy::proto::RemoveContainerResponse* response,
            ::google::protobuf::Closure* done);

    void ListContainers(::google::protobuf::RpcController* controller,
            const ::baidu::galaxy::proto::ListContainersRequest* request,
            ::baidu::galaxy::proto::ListContainersResponse* response,
            ::google::protobuf::Closure* done);

    void Query(::google::protobuf::RpcController* controller,
            const ::baidu::galaxy::proto::QueryRequest* request,
            ::baidu::galaxy::proto::QueryResponse* response,
            ::google::protobuf::Closure* done);

private:
    void KeepAlive(int internal_ms);
    void HandleMasterChange(const std::string& new_master_endpoint);

private:
    baidu::common::ThreadPool heartbeat_pool_;
    boost::scoped_ptr<baidu::galaxy::RpcClient> master_rpc_;
    boost::scoped_ptr<baidu::galaxy::MasterWatcher> rm_watcher_;
    baidu::galaxy::proto::ResMan_Stub* resman_stub_;
    std::string master_endpoint_;
    const std::string agent_endpoint_;
    bool running_;
    boost::mutex rpc_mutex_;

    boost::shared_ptr<baidu::galaxy::resource::ResourceManager> rm_;
    boost::shared_ptr<baidu::galaxy::container::ContainerManager> cm_;
    boost::shared_ptr<baidu::galaxy::health::HealthChecker> health_checker_;
    int64_t start_time_;
    std::string version_;

};

}
}
