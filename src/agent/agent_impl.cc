// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "agent/agent_impl.h"

#include "gflags/gflags.h"

#include "boost/bind.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/split.hpp"
#include "proto/master.pb.h"
#include "logging.h"

DECLARE_string(master_endpoint);
DECLARE_string(gce_endpoint);
DECLARE_int32(agent_background_threads_num);
DECLARE_int32(agent_heartbeat_interval);
DECLARE_string(agent_ip);
DECLARE_string(agent_port);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() : 
    master_endpoint_(FLAGS_master_endpoint),
    gce_endpoint_(FLAGS_gce_endpoint),
    lock_(),
    background_threads_(FLAGS_agent_background_threads_num),
    rpc_client_(NULL),
    endpoint_(),
    master_(NULL),
    gced_(NULL) { 
    rpc_client_ = new RpcClient();    
    endpoint_ = FLAGS_agent_ip;
    endpoint_.append(":");
    endpoint_.append(FLAGS_agent_port);

    background_threads_.DelayTask(
            FLAGS_agent_heartbeat_interval, boost::bind(&AgentImpl::KeepHeartBeat, this));
}

AgentImpl::~AgentImpl() {
    if (rpc_client_ != NULL) {
        delete rpc_client_; 
        rpc_client_ = NULL;
    }
}

void AgentImpl::Query(::google::protobuf::RpcController* /*cntl*/,
                      const ::baidu::galaxy::QueryRequest* req,
                      ::baidu::galaxy::QueryResponse* resp,
                      ::google::protobuf::Closure* done) {
    QueryPodsRequest gced_request;
    QueryPodsResponse gced_response;
    bool ret = rpc_client_->SendRequest(gced_,
                                        &Gced_Stub::QueryPods,
                                        &gced_request,
                                        &gced_response,
                                        5, 1);
    if (!ret) {
        resp->set_status(kRpcError); 
    } else {
        resp->set_status(kOk);
        for (int i = 0; i < gced_response.pods_size(); i++) {
            PodStatus* pod_status = 
                            resp->mutable_agent()->add_pods();
            pod_status->CopyFrom(gced_response.pods(i));
        }
    }
    done->Run(); 
    return;
}

void AgentImpl::RunPod(::google::protobuf::RpcController* /*cntl*/,
                       const ::baidu::galaxy::RunPodRequest* req,
                       ::baidu::galaxy::RunPodResponse* resp,
                       ::google::protobuf::Closure* done) {
     
    LaunchPodRequest gced_request;
    LaunchPodResponse gced_response;
    if (req->has_podid()) {
        gced_request.set_podid(req->podid()); 
    }
    if (req->has_pod()) {
        gced_request.mutable_pod()->CopyFrom(req->pod());
    }

    bool ret = rpc_client_->SendRequest(gced_,
                                        &Gced_Stub::LaunchPod,
                                        &gced_request,
                                        &gced_response,
                                        5, 1);
    if (!ret) {
        resp->set_status(kRpcError); 
    } else {
        resp->set_status(gced_response.status()); 
    }

    done->Run();
    return;
}

void AgentImpl::KillPod(::google::protobuf::RpcController* /*cntl*/,
                        const ::baidu::galaxy::KillPodRequest* req,
                        ::baidu::galaxy::KillPodResponse* resp,
                        ::google::protobuf::Closure* done) {
    TerminatePodRequest gced_request;
    TerminatePodResponse gced_response;
    if (req->has_podid()) {
        gced_request.set_podid(req->podid()); 
    }
    bool ret = rpc_client_->SendRequest(gced_, 
                                        &Gced_Stub::TerminatePod,
                                        &gced_request,
                                        &gced_response,
                                        5, 1);
    if (!ret) {
        resp->set_status(kRpcError); 
    } else {
        resp->set_status(gced_response.status()); 
    }
    done->Run();
    return;
}

void AgentImpl::KeepHeartBeat() {
    if (!PingMaster()) {
        LOG(WARNING, "ping master %s failed", 
                     master_endpoint_.c_str());
    }
    background_threads_.DelayTask(FLAGS_agent_heartbeat_interval,
                                  boost::bind(&AgentImpl::KeepHeartBeat, this));
    return;
}

bool AgentImpl::Init() {

    //resource_capacity_.millicores = FLAGS_agent_millicores;
    //resource_capacity_.memory = FLAGS_agent_memory;
    //ParseVolumeInfoFromString(FLAGS_agent_volume_disks, &(resource_capacity_.disks));
    //ParseVolumeInfoFromString(FLAGS_agent_volume_ssds, &(resource_capacity_.ssds));

    //resource_unassigned_ = resource_capacity_;
    
    if (!CheckGcedConnection()) {
        return false; 
    }

    if (!RegistToMaster()) {
        return false; 
    }
    return true;
}

bool AgentImpl::CheckGcedConnection() {
    return rpc_client_->GetStub(gce_endpoint_, &gced_);
}

bool AgentImpl::PingMaster() {
    HeartBeatRequest request;
    HeartBeatResponse response;
    request.set_endpoint(endpoint_); 
    return rpc_client_->SendRequest(master_,
                                    &Master_Stub::HeartBeat,
                                    &request,
                                    &response,
                                    5, 1);    
}

//void AgentImpl::ParseVolumeInfoFromString(const std::string& volume_str,
//                                          std::vector<Volume>* volumes) {
//    return;    
//}

bool AgentImpl::RegistToMaster() {
    if (!rpc_client_->GetStub(master_endpoint_, &master_)) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
        return false;
    }

    if (!PingMaster()) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
        return false;
    }
    return true;
}

}   // ending namespace galaxy
}   // ending namespace baidu
