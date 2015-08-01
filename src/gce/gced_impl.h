// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GCED_IMPL_H_
#define GCED_IMPL_H_

#include <string>
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "proto/gced.pb.h"
#include "mutex.h"

namespace baidu {
namespace galaxy {

class RpcClient;

class GcedImpl : public Gced, 
                 boost::noncopyable {
public:
    GcedImpl();

    ~GcedImpl();

    void LaunchPod(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::LaunchPodRequest* request,
                   ::baidu::galaxy::LaunchPodResponse* response,
                   ::google::protobuf::Closure* done);

    void TerminatePod(::google::protobuf::RpcController* controller,
                      const ::baidu::galaxy::TerminatePodRequest* request, 
                      ::baidu::galaxy::TerminatePodResponse* response,
                      ::google::protobuf::Closure* done);

    void QueryPods(::google::protobuf::RpcController* controller,
                   const ::baidu::galaxy::QueryPodsRequest* request,
                   ::baidu::galaxy::QueryPodsResponse* response, 
                   ::google::protobuf::Closure* done);

private:

    struct Pod;

    void CreateCgroup();


    void ConvertToInternalPod(const PodDescriptor& req_pod, 
                              Pod* pod);

    void LaunchInitd();

    // TODO  

private:
    struct Task {
        std::string start_command;
        std::string stop_command;
        int32_t millicores;
        int32_t memory;
        std::vector<int32_t> ports;
        std::vector<Volume> disks;
        std::vector<Volume> ssds;
    };

    struct Pod {
        // std::string podid;
        std::vector<Task> task_group;
    };

private:
    boost::scoped_ptr<RpcClient> rpc_client_; 
    
    Mutex mutex_;
    std::map<std::string, Pod> pods_;

};

} // ending namespace galaxy
} // ending namespace baidu

#endif
