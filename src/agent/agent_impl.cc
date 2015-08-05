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

DECLARE_string(master_host);
DECLARE_string(master_port);
DECLARE_string(gce_gced_port);
DECLARE_int32(agent_background_threads_num);
DECLARE_int32(agent_heartbeat_interval);
DECLARE_string(agent_ip);
DECLARE_string(agent_port);

DECLARE_int32(agent_millicores);
DECLARE_int32(agent_memory);

DECLARE_string(nexus_root_path);
DECLARE_string(master_path);
DECLARE_string(nexus_servers);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() : 
    master_endpoint_(),
    gce_endpoint_(),
    lock_(),
    background_threads_(FLAGS_agent_background_threads_num),
    rpc_client_(NULL),
    endpoint_(),
    master_(NULL),
    gced_(NULL),
    resource_capacity_(),
    nexus_(NULL),
    mutex_master_endpoint_() { 
    rpc_client_ = new RpcClient();    
    endpoint_ = FLAGS_agent_ip;
    endpoint_.append(":");
    endpoint_.append(FLAGS_agent_port);
    nexus_ = new InsSDK(FLAGS_nexus_servers);
    gce_endpoint_ = "127.0.0.1:";
    gce_endpoint_.append(FLAGS_gce_gced_port);
    background_threads_.DelayTask(
            FLAGS_agent_heartbeat_interval, boost::bind(&AgentImpl::KeepHeartBeat, this));
}

AgentImpl::~AgentImpl() {
    if (rpc_client_ != NULL) {
        delete rpc_client_; 
        rpc_client_ = NULL;
    }
    delete nexus_;
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
        resp->mutable_agent()->mutable_total()->set_millicores(FLAGS_agent_millicores);
        resp->mutable_agent()->mutable_total()->set_memory(FLAGS_agent_memory);
        resp->mutable_agent()->mutable_assigned()->set_millicores(FLAGS_agent_millicores - resource_capacity_.millicores);
        resp->mutable_agent()->mutable_assigned()->set_memory(FLAGS_agent_memory - resource_capacity_.memory);
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
    PodDesc pod;
    pod.id = req->podid();
    pod.desc = req->pod();
    int ret = pod_manager_.Run(pod);
    if (ret != 0) {
        resp->set_status(kUnknown);
    } else {
        resp->set_status(kOk);
    }
    done->Run();
    return;
    
    // LaunchPodRequest gced_request;
    // LaunchPodResponse gced_response;
    // if (req->has_podid()) {
    //     gced_request.set_podid(req->podid()); 
    // }
    // if (req->has_pod()) {
    //     gced_request.mutable_pod()->CopyFrom(req->pod());
    // }

    // ResourceCapacity requirement;
    // requirement.millicores = 0; 
    // requirement.memory = 0;
    // for (int i = 0; i < req->pod().tasks_size(); i++) {
    //     requirement.millicores += 
    //         req->pod().tasks(i).requirement().millicores(); 
    //     requirement.memory += 
    //         req->pod().tasks(i).requirement().memory();
    // }

    // if (requirement.millicores > resource_capacity_.millicores
    //         || requirement.memory > resource_capacity_.memory) {
    //     resp->set_status(kQuota); 
    //     done->Run();
    //     return;
    // }

    // bool ret = rpc_client_->SendRequest(gced_,
    //                                     &Gced_Stub::LaunchPod,
    //                                     &gced_request,
    //                                     &gced_response,
    //                                     5, 1);
    // if (!ret) {
    //     resp->set_status(kRpcError); 
    //     LOG(WARNING, "run pod failed for rpc failed");
    // } else {
    //     resp->set_status(gced_response.status()); 
    //     LOG(WARNING, "run pod status %s", 
    //             Status_Name(gced_response.status()).c_str());
    // }

    // if (resp->status() == kOk) {
    //     resource_capacity_.millicores -= requirement.millicores;    
    //     resource_capacity_.memory -= requirement.memory;
    // }
    // done->Run();
    // return;
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
    MutexLock lock(&mutex_master_endpoint_);
    if (!PingMaster()) {
        LOG(WARNING, "ping master %s failed", 
                     master_endpoint_.c_str());
    }
    background_threads_.DelayTask(FLAGS_agent_heartbeat_interval,
                                  boost::bind(&AgentImpl::KeepHeartBeat, this));
    return;
}

bool AgentImpl::Init() {

    resource_capacity_.millicores = FLAGS_agent_millicores;
    resource_capacity_.memory = FLAGS_agent_memory;
    //ParseVolumeInfoFromString(FLAGS_agent_volume_disks, &(resource_capacity_.disks));
    //ParseVolumeInfoFromString(FLAGS_agent_volume_ssds, &(resource_capacity_.ssds));

    //resource_unassigned_ = resource_capacity_;
    
    if (!CheckGcedConnection()) {
        return false; 
    }
    if (!WatchMasterPath()) {
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
    mutex_master_endpoint_.AssertHeld();
    HeartBeatRequest request;
    HeartBeatResponse response;
    request.set_endpoint(endpoint_); 
    return rpc_client_->SendRequest(master_,
                                    &Master_Stub::HeartBeat,
                                    &request,
                                    &response,
                                    5, 1);    
}

static void OnMasterChange(const ::galaxy::ins::sdk::WatchParam& param, 
                           ::galaxy::ins::sdk::SDKError error) {
    AgentImpl* agent = static_cast<AgentImpl*>(param.context);
    agent->HandleMasterChange(param.value);
}

void AgentImpl::HandleMasterChange(const std::string& new_master_endpoint) {
    if (!WatchMasterPath()) {
        LOG(WARNING, "watch master again failed");
        abort();
    }
    MutexLock lock(&mutex_master_endpoint_);
    if (new_master_endpoint.empty()) {
        LOG(WARNING, "the master endpoint is deleted from nexus");
    }
    if (new_master_endpoint != master_endpoint_) {
        LOG(INFO, "master change to %s", new_master_endpoint.c_str());
        master_endpoint_ = new_master_endpoint; 
        if (master_) {
            delete master_;
        }
        if (!rpc_client_->GetStub(master_endpoint_, &master_)) {
            LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
            return;
        }  
    }
}

bool AgentImpl::WatchMasterPath() {
    std::string master_path_key = FLAGS_nexus_root_path + FLAGS_master_path;
    ::galaxy::ins::sdk::SDKError err;
    bool ok = nexus_->Watch(master_path_key, &OnMasterChange, this, &err);
    if (!ok) {
        LOG(WARNING, "fail to watch on nexus, err_code: %d", err);
    } else {
        LOG(INFO, "watch master success.");
    }
    return ok;
}

bool AgentImpl::RegistToMaster() {
    MutexLock lock(&mutex_master_endpoint_);
    std::string master_path_key = FLAGS_nexus_root_path + FLAGS_master_path;
    ::galaxy::ins::sdk::SDKError err;
    bool ok = nexus_->Get(master_path_key, &master_endpoint_, &err);
    if (!ok) {
        LOG(WARNING, "fail to get master endpoint from nexus, err_code: %d", err);
        return false;
    } else {
        LOG(INFO, "master endpoint is: %s", master_endpoint_.c_str());
    }

    if (!rpc_client_->GetStub(master_endpoint_, &master_)) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
        return false;
    }

    if (!PingMaster()) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
    }
    return true;
}

}   // ending namespace galaxy
}   // ending namespace baidu
