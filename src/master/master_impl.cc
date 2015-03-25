// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "master_impl.h"

#include "proto/agent.pb.h"
#include "rpc/rpc_client.h"

namespace galaxy {

MasterImpl::MasterImpl()
    : next_agent_id_(0),
      next_task_id_(0) {
    rpc_client_ = new RpcClient();
}

void MasterImpl::TerminateTask(::google::protobuf::RpcController* controller,
                 const ::galaxy::TerminateTaskRequest* request,
                 ::galaxy::TerminateTaskResponse* response,
                 ::google::protobuf::Closure* done) {
    if (!request->has_task_id()) {
        response->set_status(-1); 
        done->Run();
        return;
    }

    int64_t task_id = request->task_id();
    common::MutexLock lock(&agent_lock_); 
    std::map<int64_t, TaskInstance>::iterator it; 
    it = agent_task_pair_.find(task_id);
    if (it == agent_task_pair_.end()) {
        response->set_status(-1);
        done->Run();
        return;
    }

    std::string agent_addr = it->second.agent_addr();
    AgentInfo& agent = agents_[agent_addr];
    if (agent.stub == NULL) {
        bool ret = 
            rpc_client_->GetStub(agent_addr, &agent.stub); 
        assert(ret);
    }
    KillTaskRequest kill_request; 
    kill_request.set_task_id(task_id);
    KillTaskResponse kill_response;
    bool ret = 
        rpc_client_->SendRequest(agent.stub, &Agent_Stub::KillTask,
                                &kill_request, &kill_response, 5, 1);
    if (!ret) {
        LOG(WARNING, "Kill failed agent= %s", agent_addr.c_str()); 
        response->set_status(-2);
    } else {
        agent_task_pair_.erase(it);
        response->set_status(0); 
    }
    done->Run();
}

void MasterImpl::ListTask(::google::protobuf::RpcController* controller,
                 const ::galaxy::ListTaskRequest* request,
                 ::galaxy::ListTaskResponse* response,
                 ::google::protobuf::Closure* done) {
    std::map<int64_t, TaskInstance> tmp_task_pair;
    std::map<int64_t, TaskInstance>::iterator it;
    {
        // @TODO memory copy 
        common::MutexLock lock(&agent_lock_); 
        tmp_task_pair = agent_task_pair_; 
    }
    if (request->has_task_id()) {
        // just list one
        it = tmp_task_pair.find(request->task_id());     
        if (it != tmp_task_pair.end()) {
            response->add_tasks()->CopyFrom(it->second);
        }
    }
    else {
        it = tmp_task_pair.begin(); 
        for (; it != tmp_task_pair.end(); ++it) {
            response->add_tasks()->CopyFrom(it->second);
        }
    }
    done->Run();
}

void MasterImpl::HeartBeat(::google::protobuf::RpcController* controller,
                           const ::galaxy::HeartBeatRequest* request,
                           ::galaxy::HeartBeatResponse* response,
                           ::google::protobuf::Closure* done) {
    const std::string& agent_addr = request->agent_addr();
    LOG(INFO, "HeartBeat from %s", agent_addr.c_str());
    MutexLock lock(&agent_lock_);
    std::map<std::string, AgentInfo>::iterator it;
    it = agents_.find(agent_addr);
    if (it == agents_.end()) {
        AgentInfo& agent = agents_[agent_addr];
        agent.addr = agent_addr;
        agent.id = next_agent_id_ ++;
        agent.task_num = request->task_status_size();
        agent.stub = NULL;
    } else if (request->task_status_size() != 0){
        //@TODO maybe copy out of lock
        for (int ind = 0; 
                ind < request->task_status_size(); 
                ind++) {
            int64_t task_id = request->task_status(ind).task_id();
            TaskInstance& instance = agent_task_pair_[task_id];
            // @NOTE only update status in heartbeat
            instance.mutable_status()->CopyFrom(request->task_status(ind));
            instance.set_agent_addr(agent_addr);
            LOG(INFO, "%s run task %d %d", agent_addr.c_str(), task_id, request->task_status(ind).status());
        }
    }

    it->second.task_num = request->task_status_size();
    done->Run();
}

void MasterImpl::NewTask(::google::protobuf::RpcController* controller,
                         const ::galaxy::NewTaskRequest* request,
                         ::galaxy::NewTaskResponse* response,
                         ::google::protobuf::Closure* done) {
    MutexLock lock(&agent_lock_);
    std::string agent_addr;

    int low_load = 1;
    ///TODO: Use priority queue
    std::map<std::string, AgentInfo>::iterator it = agents_.begin();
    for (; it != agents_.end(); ++it) {
        AgentInfo& ai = it->second;
        if (ai.task_num < low_load) {
            low_load = ai.task_num;
            agent_addr = ai.addr;
        }
    }
    if (agent_addr.empty()) {
        response->set_status(-1);
        done->Run();
        return;
    }

    AgentInfo& agent = agents_[agent_addr];
    if (agent.stub == NULL) {
        bool ret = rpc_client_->GetStub(agent_addr, &agent.stub);
        assert(ret);
    }
    RunTaskRequest rt_request;
    rt_request.set_task_id(next_task_id_++);
    rt_request.set_task_name(request->task_name());
    rt_request.set_task_raw(request->task_raw());
    rt_request.set_cmd_line(request->cmd_line());
    RunTaskResponse rt_response;
    LOG(INFO, "RunTask on %s", agent_addr.c_str());
    bool ret = rpc_client_->SendRequest(agent.stub, &Agent_Stub::RunTask,
                                        &rt_request, &rt_response, 5, 1);
    if (!ret) {
        LOG(WARNING, "RunTask faild agent= %s", agent_addr.c_str());
        response->set_status(-2);
    } else {
        TaskInstance& instance = agent_task_pair_[rt_request.task_id()];
        instance.mutable_info()->set_task_name(request->task_name());
        instance.set_agent_addr(agent_addr);
        response->set_status(0);
    }
    done->Run();
}
} // namespace galasy

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
