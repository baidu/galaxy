// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "agent_impl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/bind.hpp>
#include "common/util.h"
#include "rpc/rpc_client.h"

extern std::string FLAGS_master_addr;
extern std::string FLAGS_agent_port;

namespace galaxy {

AgentImpl::AgentImpl() {
    rpc_client_ = new RpcClient();
    ws_mgr_ = new ::galaxy::agent::WorkspaceManager("/tmp");
    task_mgr_ = new ::galaxy::agent::TaskManager();
    if (!rpc_client_->GetStub(FLAGS_master_addr, &master_)) {
        assert(0);
    }
    thread_pool_.Start();
    thread_pool_.AddTask(boost::bind(&AgentImpl::Report, this));
}

AgentImpl::~AgentImpl() {
    if(ws_mgr_ != NULL){
        delete ws_mgr_;
    }

}

void AgentImpl::Report() {
    HeartBeatRequest request;
    HeartBeatResponse response;
    std::string addr = common::util::GetLocalHostName() + ":" + FLAGS_agent_port;
    std::vector< ::galaxy::TaskStatus > status_vector;
    task_mgr_->Status(status_vector);
    std::vector< ::galaxy::TaskStatus>::iterator it = status_vector.begin();
    for(;it != status_vector.end();++it){
        ::galaxy::TaskStatus * req_status = request.add_task_status();
        req_status->set_task_id(it->task_id());
        req_status->set_status(it->status());
    }
    request.set_agent_addr(addr);

    LOG(INFO, "Reprot to master %s", addr.c_str());
    rpc_client_->SendRequest(master_, &Master_Stub::HeartBeat,
                                &request, &response, 5, 1);
    thread_pool_.DelayTask(5000, boost::bind(&AgentImpl::Report, this));
}

void AgentImpl::OpenProcess(const std::string& task_name,
                            const std::string& task_raw,
                            const std::string& cmd_line,
                            const std::string& task_root_path) {

    std::string task_path = task_root_path + task_name;
    int fd = open(task_path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    if (fd < 0) {
        LOG(WARNING, "Open %s sor write fail", task_path.c_str());
        return ;
    }
    int len = write(fd, task_raw.data(), task_raw.size());
    if (len < 0) {
        LOG(WARNING, "Write fail : %s", strerror(errno));
    }
    LOG(INFO, "Write %d bytes to %s", len, task_path.c_str());
    close(fd);
    /* FILE* fp = fopen(task_name.c_str(), "w");
    fwrite(task_raw.data(), task_raw.size(), 1, fp);
    fclose(fp);
    */
    LOG(INFO,"Fork to run %s", task_name.c_str());
    pid_t pid = fork();
    if (pid != 0) {
        return;
    }
    execl("/bin/sh", "sh", "-c", task_path.c_str(), NULL);
    /* Exit the child process if execl fails */
    assert(0);
    _exit(127);
}
void AgentImpl::RunTask(::google::protobuf::RpcController* controller,
                        const ::galaxy::RunTaskRequest* request,
                        ::galaxy::RunTaskResponse* response,
                        ::google::protobuf::Closure* done) {
    LOG(INFO, "Run Task %s %s", request->task_name().c_str(), request->cmd_line().c_str());
    ::galaxy::TaskInfo task_info;
    task_info.set_task_name(9527);
    task_info.set_cmd_line("python -m SimpleHTTPServer 8189");
    LOG(INFO,"start to prepare workspace for %s",request->task_name().c_str());
    int ret = ws_mgr_->Add(task_info);
    if(ret != 0 ){
        LOG(FATAL,"fail to prepare workspace ");
        done->Run();
    }else{
        LOG(INFO,"start  task for %s",request->task_name().c_str());
        ::galaxy::agent::DefaultWorkspace * workspace ;
        workspace = ws_mgr_->GetWorkspace(task_info);
        ret = task_mgr_->Add(task_info,*workspace);
        if (ret != 0){
           LOG(FATAL,"fail to start task");
        }
        done->Run();
    }
    //OpenProcess(request->task_name(), request->task_raw(), request->cmd_line(),"/tmp");
    //done->Run();
}

} // namespace galxay

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
