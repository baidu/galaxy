// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gced_impl.h"
#include "rpc/rpc_client.h"
#include "logging.h"
#include "proto/initd.pb.h"
// #include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

GcedImpl::GcedImpl() {
    rpc_client_.reset(new RpcClient());
}

GcedImpl::~GcedImpl() {
}

void GcedImpl::CreateCgroup() {
    return;
}

void GcedImpl::LaunchInitd() {
      
}

void GcedImpl::ConvertToInternalPod(const PodDescriptor& req_pod, 
                                    Pod* pod) {
    if (NULL == pod) {
        return;
    }

    for (int i = 0; i < req_pod.tasks_size(); ++i) {
        const TaskDescriptor& task_desc = req_pod.tasks(i);
        const Resource& resource = task_desc.requirement();
        Task task; 
        task.millicores = resource.millicores();
        task.memory = resource.memory();
        task.start_command = task_desc.start_command();
        task.stop_command = task_desc.stop_command();

        for (int i = 0; i < resource.ports_size(); ++i) {
            task.ports.push_back(resource.ports(i));
        }

        for (int i = 0; i < resource.disks_size(); ++i) {
            task.disks.push_back(resource.disks(i));
        }

        for (int i = 0; i < resource.ssds_size(); ++i) {
            task.ssds.push_back(resource.ssds(i));
        }
        pod->task_group.push_back(task);
    }
}

void GcedImpl::LaunchPod(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::LaunchPodRequest* request,
                         ::baidu::galaxy::LaunchPodResponse* response,
                         ::google::protobuf::Closure* done) {
    if (!request->has_podid() 
        || !request->has_pod()) {
        LOG(WARNING, "not enough input params"); 
        response->set_status(kInputError);
        done->Run();
        return;
    }

    Pod pod;
    ConvertToInternalPod(request->pod(), &pod);
    const std::string& podid = request->podid();
    LOG(DEBUG, "launch pod id:%s", podid.c_str());
    {
        MutexLock scope_lock(&mutex_);
        pods_[podid] = pod;
    }
    
    // 1. TODO create cgroup
    CreateCgroup();

    // 2. TODO clone namespace
     
    // 3. TODO launch
     


    // Initd_Stub* initd;
    // rpc_client_->GetS
}

void GcedImpl::TerminatePod(::google::protobuf::RpcController* controller,
                            const ::baidu::galaxy::TerminatePodRequest* request, 
                            ::baidu::galaxy::TerminatePodResponse* response,
                            ::google::protobuf::Closure* done) {
}

void GcedImpl::QueryPods(::google::protobuf::RpcController* controller,
                         const ::baidu::galaxy::QueryPodsRequest* request,
                         ::baidu::galaxy::QueryPodsResponse* response, 
                         ::google::protobuf::Closure* done) {

}

} // ending namespace galaxy
} // ending namespace baidu
