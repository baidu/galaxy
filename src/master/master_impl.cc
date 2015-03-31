// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "master_impl.h"

#include "proto/agent.pb.h"
#include "rpc/rpc_client.h"

extern int FLAGS_task_deloy_timeout;
extern int FLAGS_agent_keepalive_timeout;

namespace galaxy {

MasterImpl::MasterImpl()
    : next_agent_id_(0),
      next_task_id_(0),
      next_job_id_(0) {
    rpc_client_ = new RpcClient();
    thread_pool_.AddTask(boost::bind(&MasterImpl::Schedule, this));
    thread_pool_.AddTask(boost::bind(&MasterImpl::DeadCheck, this));
}

void MasterImpl::TerminateTask(::google::protobuf::RpcController* /*controller*/,
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
    it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        response->set_status(-1);
        done->Run();
        return;
    }

    std::string agent_addr = it->second.agent_addr();
    AgentInfo& agent = agents_[agent_addr];
    if (agent.stub == NULL) {
        bool ret = rpc_client_->GetStub(agent_addr, &agent.stub); 
        assert(ret);
    }
    KillTaskRequest kill_request; 
    kill_request.set_task_id(task_id);
    KillTaskResponse kill_response;
    bool ret = rpc_client_->SendRequest(agent.stub, &Agent_Stub::KillTask,
                                        &kill_request, &kill_response, 5, 1);
    if (!ret) {
        LOG(WARNING, "Kill failed agent= %s", agent_addr.c_str()); 
        response->set_status(-2);
    } else {
        response->set_status(0); 
    }
    done->Run();
}

void MasterImpl::ListTask(::google::protobuf::RpcController* /*controller*/,
                 const ::galaxy::ListTaskRequest* request,
                 ::galaxy::ListTaskResponse* response,
                 ::google::protobuf::Closure* done) {
    std::map<int64_t, TaskInstance> tmp_task_pair;
    std::map<int64_t, TaskInstance>::iterator it;
    {
        // @TODO memory copy 
        common::MutexLock lock(&agent_lock_); 
        tmp_task_pair = tasks_; 
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


void MasterImpl::ListJob(::google::protobuf::RpcController* controller,
                         const ::galaxy::ListJobRequest* request,
                         ::galaxy::ListJobResponse* response,
                         ::google::protobuf::Closure* done) {
    MutexLock lock(&agent_lock_);

    std::map<int64_t, JobInfo>::iterator it = jobs_.begin();
    for (; it !=jobs_.end(); ++it) {
        JobInfo& job = it->second;
        JobInstance* job_inst = response->add_jobs();
        job_inst->set_job_id(job.id);
        job_inst->set_job_name(job.job_name);
        job_inst->set_running_task_num(job.running_num);
        job_inst->set_replica_num(job.replica_num);
    }
    done->Run();
}

void MasterImpl::DeadCheck() {
    int32_t now_time = common::timer::now_time();

    MutexLock lock(&agent_lock_);
    std::map<int32_t, std::set<std::string> >::iterator it = alives_.begin();
    
    while (it != alives_.end() 
           && it->first + FLAGS_agent_keepalive_timeout < now_time) {
        std::set<std::string>::iterator node = it->second.begin();
        while (node != it->second.end()) {
            AgentInfo& agent = agents_[*node];
            LOG(INFO, "[DeadCheck] Agent %s dead, %lu task fail",
                agent.addr.c_str(), agent.running_tasks.size());
            std::set<int64_t> running_tasks;
            UpdateJobsOnAgent(&agent, running_tasks, true);
            agents_.erase(*node);
            it->second.erase(node);
            node = it->second.begin();
        }
        alives_.erase(it);
        it = alives_.begin();
    }
    int idle_time = 5;
    if (it != alives_.end()) {
        idle_time = it->first + FLAGS_agent_keepalive_timeout - now_time;
        if (idle_time > 5) {
            idle_time = 5;
        }
    }
    thread_pool_.DelayTask(idle_time * 1000,
                           boost::bind(&MasterImpl::DeadCheck, this));
}

void MasterImpl::UpdateJobsOnAgent(AgentInfo* agent,
                                   const std::set<int64_t>& running_tasks,
                                   bool clear_all) {
    const std::string& agent_addr = agent->addr;
    assert(!agent_addr.empty());

    int32_t now_time = common::timer::now_time();
    std::set<int64_t>::iterator it = agent->running_tasks.begin();
    std::vector<int64_t> del_tasks;
    for (; it != agent->running_tasks.end(); ++it) {
        int64_t task_id = *it;
        if (running_tasks.find(task_id) == running_tasks.end()) {
            TaskInstance& instance = tasks_[task_id];
            if (instance.status() == DEPLOYING &&
                instance.start_time() + FLAGS_task_deloy_timeout > now_time
                && !clear_all) {
                LOG(INFO, "Wait for deloy timeout %ld", task_id);
                continue;
            }
            int64_t job_id = instance.job_id();
            assert(jobs_.find(job_id) != jobs_.end());
            JobInfo& job = jobs_[job_id];
            job.running_agents[agent_addr] --;
            job.running_num --;
            del_tasks.push_back(task_id);
            tasks_.erase(task_id);
            LOG(INFO, "Job[%s] task %ld disappear from %s",
                job.job_name.c_str(), task_id, agent_addr.c_str());
        }
    }
    for (uint64_t i = 0UL; i < del_tasks.size(); ++i) {
        agent->running_tasks.erase(del_tasks[i]);
    }
}

void MasterImpl::HeartBeat(::google::protobuf::RpcController* /*controller*/,
                           const ::galaxy::HeartBeatRequest* request,
                           ::galaxy::HeartBeatResponse* response,
                           ::google::protobuf::Closure* done) {
    const std::string& agent_addr = request->agent_addr();
    LOG(INFO, "HeartBeat from %s", agent_addr.c_str());

    int now_time = common::timer::now_time();

    MutexLock lock(&agent_lock_);
    std::map<std::string, AgentInfo>::iterator it;
    it = agents_.find(agent_addr);
    AgentInfo* agent = NULL;
    if (it == agents_.end()) {
        LOG(INFO, "New Agent register %s", agent_addr.c_str());
        alives_[now_time].insert(agent_addr);
        agent = &agents_[agent_addr];
        agent->addr = agent_addr;
        agent->id = next_agent_id_ ++;
        agent->task_num = request->task_status_size();
        agent->stub = NULL;
    } else {
        agent = &(it->second);
        alives_[agent->alive_timestamp].erase(agent_addr);
        alives_[now_time].insert(agent_addr);
    }
    agent->alive_timestamp = now_time;
    response->set_agent_id(agent->id);
    
    //@TODO maybe copy out of lock
    int task_num = request->task_status_size();
    std::set<int64_t> running_tasks;
    for (int i = 0; i < task_num; i++) {
        int64_t task_id = request->task_status(i).task_id();
        running_tasks.insert(task_id);

        TaskInstance& instance = tasks_[task_id];

        int task_status = request->task_status(i).status();
        instance.set_status(task_status);
        LOG(INFO, "Task %d status: %s", 
            task_id, TaskState_Name((TaskState)task_status).c_str());
        instance.set_agent_addr(agent_addr);
        LOG(INFO, "%s run task %d %d", agent_addr.c_str(),
            task_id, request->task_status(i).status());
    }
    agent->task_num = request->task_status_size();

    UpdateJobsOnAgent(agent, running_tasks);
    done->Run();
}

void MasterImpl::NewTask(::google::protobuf::RpcController* /*controller*/,
                         const ::galaxy::NewTaskRequest* request,
                         ::galaxy::NewTaskResponse* response,
                         ::google::protobuf::Closure* done) {
    MutexLock lock(&agent_lock_);
    int64_t job_id = next_job_id_++;
    JobInfo& job = jobs_[job_id];
    job.id = job_id;
    job.job_name = request->task_name();
    job.job_raw = request->task_raw();
    job.cmd_line = request->cmd_line();
    job.replica_num = request->replica_num();
    job.running_num = 0;

    response->set_status(0); 
    done->Run();
}

std::string MasterImpl::AllocResource(/*ResourceRequirement*/) {
    agent_lock_.AssertHeld();
    std::string agent_addr;
    int low_load = 1 << 30;
    ///TODO: Use priority queue
    std::map<std::string, AgentInfo>::iterator it = agents_.begin();
    for (; it != agents_.end(); ++it) {
        AgentInfo& ai = it->second;
        if (ai.task_num < low_load) {
            low_load = ai.task_num;
            agent_addr = ai.addr;
        }
    }
    LOG(INFO, "Allocate resource %s", agent_addr.c_str());
    return agent_addr;
}

bool MasterImpl::ScheduleTask(JobInfo* job, const std::string& agent_addr) {
    agent_lock_.AssertHeld();
    AgentInfo& agent = agents_[agent_addr];

    if (agent.stub == NULL) {
        bool ret = rpc_client_->GetStub(agent_addr, &agent.stub);
        assert(ret);
    }

    int64_t task_id = next_task_id_++;
    RunTaskRequest rt_request;
    rt_request.set_task_id(task_id);
    rt_request.set_task_name(job->job_name);
    rt_request.set_task_raw(job->job_raw);
    rt_request.set_cmd_line(job->cmd_line);
    RunTaskResponse rt_response;
    LOG(INFO, "ScheduleTask on %s", agent_addr.c_str());
    bool ret = rpc_client_->SendRequest(agent.stub, &Agent_Stub::RunTask,
                                        &rt_request, &rt_response, 5, 1);
    if (!ret) {
        LOG(WARNING, "RunTask faild agent= %s", agent_addr.c_str());
    } else {
        agent.running_tasks.insert(task_id);
        agent.task_num ++;
        TaskInstance& instance = tasks_[task_id];
        instance.mutable_info()->set_task_name(job->job_name);
        instance.mutable_info()->set_task_id(task_id);
        instance.set_agent_addr(agent_addr);
        instance.set_job_id(job->id);
        instance.set_start_time(common::timer::now_time());
        instance.set_status(DEPLOYING);
        job->running_agents[agent_addr]++;
        job->running_num++;
    }
    return ret;
}

void MasterImpl::Schedule() {
    MutexLock lock(&agent_lock_);
    std::map<int64_t, JobInfo>::iterator job_it = jobs_.begin();
    for (; job_it != jobs_.end(); ++job_it) {
        JobInfo& job = job_it->second;
        for (int i = job.running_num; i < job.replica_num; i++) {
            LOG(INFO, "[Schedule] Job[%s] running %d tasks, replica_num %d",
                job.job_name.c_str(), job.running_num, job.replica_num);
            std::string agent_addr = AllocResource();
            if (agent_addr.empty()) {
                LOG(WARNING, "Allocate resource fail, delay schedule job %s",
                    job.job_name.c_str());
                continue;
            }
            ScheduleTask(&job, agent_addr);
        }
    }
    thread_pool_.DelayTask(1000, boost::bind(&MasterImpl::Schedule, this));
}

} // namespace galasy

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
