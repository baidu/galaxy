// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "agent_impl.h"
#include "rpc/rpc_client.h"
#include "boost/thread/mutex.hpp"
#include "protocol/resman.pb.h"
#include "cgroup/subsystem_factory.h"
#include "collector/collector_engine.h"
#include "util/path_tree.h"

#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
//#include <sys/utsname.h>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <snappy.h>

DECLARE_string(agent_ip);
DECLARE_string(agent_port);
DECLARE_int32(keepalive_interval);
DECLARE_string(galaxy_root_path);

namespace baidu {
namespace galaxy {

AgentImpl::AgentImpl() :
    heartbeat_pool_(1),
    master_rpc_(new baidu::galaxy::RpcClient()),
    rm_watcher_(new baidu::galaxy::MasterWatcher()),
    resman_stub_(NULL),
    agent_endpoint_(FLAGS_agent_ip + ":" + FLAGS_agent_port),
    running_(false),
    rm_(new baidu::galaxy::resource::ResourceManager),
    cm_(new baidu::galaxy::container::ContainerManager(rm_)),
    health_checker_(new baidu::galaxy::health::HealthChecker()),
    start_time_(baidu::common::timer::get_micros())
{
    version_ = "0.0.1";
    //version_ = __DATE__ + __TIME__;
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

    baidu::galaxy::path::SetRootPath(FLAGS_galaxy_root_path);
    running_ = true;
    if (0 != rm_->Load()) {
        LOG(FATAL) << "resource manager loads resource failed";
        exit(1);
    }

    baidu::galaxy::cgroup::SubsystemFactory::GetInstance()->Setup();
    baidu::galaxy::container::ContainerStatus::Setup();
    cm_->Setup();

    health_checker_->LoadVolum(rm_);
    health_checker_->LoadCgroup(baidu::galaxy::cgroup::SubsystemFactory::GetInstance());
    health_checker_->Setup();

    if (!rm_watcher_->Init(boost::bind(&AgentImpl::HandleMasterChange, this, _1))) {
        LOG(FATAL) << "init res manager watch failed, agent will exit ...";
        exit(1);
    }
    LOG(INFO) << "init resource manager watcher successfully";


   baidu::galaxy::collector::CollectorEngine::GetInstance()->Setup();

    heartbeat_pool_.AddTask(boost::bind(&AgentImpl::KeepAlive, this, FLAGS_keepalive_interval));
    LOG(INFO) << "start keep alive thread, interval is " << FLAGS_keepalive_interval << "ms";
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
    LOG(INFO) << "recv create container request: " << request->DebugString();
    int64_t x = baidu::common::timer::get_micros();
    std::cerr <<x<< "create " << request->id() << std::endl;
    
    baidu::galaxy::container::ContainerId id(request->container_group_id(), request->id());
    baidu::galaxy::proto::ErrorCode* ec = response->mutable_code();

    baidu::galaxy::util::ErrorCode err = cm_->CreateContainer(id, request->container());
    if (0 != err.Code()) {
        ec->set_status(baidu::galaxy::proto::kError);
        ec->set_reason(err.ShortMessage());
    } else {
        ec->set_status(baidu::galaxy::proto::kOk);
        ec->set_reason("sucess");
    }

    done->Run();
}

void AgentImpl::RemoveContainer(::google::protobuf::RpcController* controller,
        const ::baidu::galaxy::proto::RemoveContainerRequest* request,
        ::baidu::galaxy::proto::RemoveContainerResponse* response,
        ::google::protobuf::Closure* done)
{

    LOG(INFO) << "recv remove container request: " << request->DebugString();
    std::cerr << "recv remove container request: " << request->DebugString() << std::endl;
    baidu::galaxy::container::ContainerId id(request->container_group_id(), request->id());
    baidu::galaxy::proto::ErrorCode* ec = response->mutable_code();
    baidu::galaxy::util::ErrorCode ret = cm_->ReleaseContainer(id);

    if (0 != ret.Code()) {
        ec->set_status(baidu::galaxy::proto::kError);
        ec->set_reason(ret.ShortMessage());
    } else {
        ec->set_status(baidu::galaxy::proto::kOk);
        ec->set_reason("sucess");
    }
    done->Run();
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

    std::cerr << "query " << std::endl;

    baidu::galaxy::proto::AgentInfo* ai = response->mutable_agent_info();
    ai->set_unhealthy(!health_checker_->Healthy());
    ai->set_start_time(start_time_);
    ai->set_version(version_);


    bool full_report = false;
    if (request->has_full_report() && request->full_report()) {
        full_report = true;
    }
    std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> > cis;
    cm_->ListContainers(cis, full_report);

    for (size_t i = 0; i < cis.size(); i++) {
        ai->add_container_info()->CopyFrom(*(cis[i]));
    }

    int64_t cpu_used = 0L;
    int64_t memory_used = 0L;
    int64_t memory_volum_used = 0L;
    // calculate used resource

    std::map<std::string, int64_t> volum_used;
    for (size_t i = 0; i < cis.size(); i++) {
        cpu_used += cis[i]->cpu_used();
        memory_used += cis[i]->memory_used();

        for (int j = 0; j < cis[i]->volum_used_size(); j++) {
            if (cis[i]->volum_used(j).medium() == baidu::galaxy::proto::kTmpfs) {
                memory_volum_used += cis[i]->volum_used(j).used_size();
            }

            std::string dp = cis[i]->volum_used(j).device_path();
            std::map<std::string, int64_t> ::iterator iter = volum_used.find(dp);
            if (iter == volum_used.end()) {
                volum_used[dp] = cis[i]->volum_used(j).used_size();
            } else {
                iter->second += cis[i]->volum_used(j).used_size();
            }

        }
    }
    
    baidu::galaxy::proto::Resource* cpu_resource = ai->mutable_cpu_resource();
    cpu_resource->CopyFrom(*(rm_->GetCpuResource()));
    cpu_resource->set_used(cpu_used);

    baidu::galaxy::proto::Resource* memory_resource = ai->mutable_memory_resource();
    memory_resource->CopyFrom(*(rm_->GetMemoryResource()));
    memory_resource->set_used(memory_used + memory_volum_used);

    std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumResource> > vrs;
    rm_->GetVolumResource(vrs);
    for (size_t i = 0; i < vrs.size(); i++) {
        baidu::galaxy::proto::VolumResource* vr = ai->add_volum_resources();
        vr->set_device_path(vrs[i]->device_path());
        vr->set_medium(vrs[i]->medium());
        baidu::galaxy::proto::Resource* r = vr->mutable_volum();
        r->set_total(vrs[i]->volum().total());
        r->set_assigned(vrs[i]->volum().assigned());

        std::map<std::string, int64_t>::const_iterator iter = volum_used.find(vrs[i]->device_path());
        if (iter != volum_used.end()) {
            r->set_used(iter->second);
        } else {
            r->set_used(0L);
        }
    }

    baidu::galaxy::proto::ErrorCode* ec = response->mutable_code();
    ec->set_status(baidu::galaxy::proto::kOk);
    std::cerr << response->DebugString() << std::endl;
    VLOG(10) << "query:" << response->DebugString();
    //std::cout << "query:" << response->DebugString() << std::endl;
    done->Run();
}


}
}
