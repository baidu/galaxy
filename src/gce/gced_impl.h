// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GCED_IMPL_H_
#define GCED_IMPL_H_

#include <string>
#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include "proto/gced.pb.h"
#include "mutex.h"

namespace baidu {
namespace galaxy {

class RpcClient;
class ProcessInfo;

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
    struct Task;

    void CreateCgroup();

    void ConvertToInternalTask(const TaskDescriptor& req_pod, 
                               Task* task);

    void LaunchInitd();

    int RandRange(int min, int max);

    void TaskStatusCheck();

    void GetProcessStatus(const std::string& key, 
                          ProcessInfo* info);

private:
    struct Task {
        std::string binary;
        std::string start_command;
        std::string stop_command;
        int32_t millicores;
        int32_t memory;
        std::vector<int32_t> ports;
        std::vector<Volume> disks;
        std::vector<Volume> ssds;
        std::string key;
    };

    struct Pod {
        // std::string podid;
        std::vector<Task> task_group;
        int32_t port;
        PodState state;
    };

    void BuildDeployingCommand(const Task& task, std::string* deploying_command); 
private:
    RpcClient* rpc_client_; 
    Mutex mutex_;
    std::map<std::string, Pod> pods_;
    // common::ThreadPool* thread_pool_;
};

} // ending namespace galaxy
} // ending namespace baidu

#endif
