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
#include <errno.h>
#include <string.h>

#include "common/util.h"
#include "rpc/rpc_client.h"

extern std::string FLAGS_master_addr;
extern std::string FLAGS_agent_port;
extern std::string FLAGS_agent_work_dir;

namespace galaxy {

AgentImpl::AgentImpl() {
    rpc_client_ = new RpcClient();
    if (!rpc_client_->GetStub(FLAGS_master_addr, &master_)) {
        assert(0);
    }
    thread_pool_.Start();
    thread_pool_.AddTask(boost::bind(&AgentImpl::Report, this));
}

AgentImpl::~AgentImpl() {
}

void AgentImpl::Report() {
    HeartBeatRequest request;
    HeartBeatResponse response;
    std::string addr = common::util::GetLocalHostName() + ":" + FLAGS_agent_port;
    request.set_agent_addr(addr);
    LOG(INFO, "Reprot to master %s", addr.c_str());
    rpc_client_->SendRequest(master_, &Master_Stub::HeartBeat,
                                &request, &response, 5, 1);
    thread_pool_.DelayTask(5000, boost::bind(&AgentImpl::Report, this));
}

void AgentImpl::OpenProcess(const std::string& task_name,
                            const std::string& task_raw,
                            const std::string& cmd_line) {
    std::string task_path = FLAGS_agent_work_dir + task_name;
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

    LOG(INFO, "Fork to Run %s", task_name.c_str());
    std::string root_path = FLAGS_agent_work_dir;

    // do prepare, NOTE, root_path should be unqic
    std::string task_stdout = root_path + "./stdout";
    std::string task_stderr = root_path + "./stderr"; 
    int stdout_fd = open(task_stdout.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    int stderr_fd = open(task_stdout.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

    int cur_pid = getpid();
    std::vector<int> fds;
    common::util::GetProcessFdList(cur_pid, fds);
    // put last two fd for child process, dup

    pid_t pid = fork();
    if (pid != 0) {
        close(stdout_fd);
        close(stderr_fd);
        return;
    }

    // do in child process
    // NOTE if dup2 will return errno == EINTR?  
    while (dup2(stdout_fd, STDOUT_FILENO) == -1 && errno == EINTR) {}
    while (dup2(stderr_fd, STDERR_FILENO) == -1 && errno == EINTR) {}

    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i] == STDOUT_FILENO 
                || fds[i] == STDERR_FILENO 
                || fds[i] == STDIN_FILENO) {
            // do not deal with std input/output
            continue; 
        } 
        close(fds[i]);
    }
    
    chdir(root_path.c_str());

    int ret = execl("/bin/sh", "sh", "-c", cmd_line.c_str(), NULL);
    if (ret != 0) {
        LOG(INFO, "exec failed %d %s", errno, strerror(errno));
    }
    assert(0);
    _exit(127);
    return;
}

void AgentImpl::RunTask(::google::protobuf::RpcController* controller,
                        const ::galaxy::RunTaskRequest* request,
                        ::galaxy::RunTaskResponse* response,
                        ::google::protobuf::Closure* done) {
    LOG(INFO, "Run Task %s %s", request->task_name().c_str(), request->cmd_line().c_str());
    OpenProcess(request->task_name(), request->task_raw(), request->cmd_line());
    done->Run();
}

} // namespace galxay

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
