// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/task_runner.h"

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include "common/logging.h"
#include "common/util.h"

namespace galaxy {


//check process with m_child_pid
int CommandTaskRunner::IsRunning() {
    if (m_child_pid == -1) {
        return -1;
    }
    // check process exist
    int ret = ::kill(m_child_pid, 0);
    if(ret == 0 ){
        //check process status
        pid_t pid = waitpid(m_child_pid,&ret,WNOHANG);
        if(pid == -1 ){
            LOG(WARNING,"check process %d state error",m_child_pid);
            return -1;
        }else if(pid==0){
            LOG(INFO,"process %d is running",m_child_pid);
        }else{
            LOG(WARNING,"process %d has gone",m_child_pid);
            //restart
            return -1;
        }
    }
    LOG(INFO, "check task %d ret %d", m_task_info.task_id(), ret);
    return ret;
}

//start process
//1. fork a subprocess A
//2. exec command in the subprocess A
//TODO add workspace
int CommandTaskRunner::Start() {
    LOG(INFO, "start a task with id %d", m_task_info.task_id());
    m_mutex->Lock("start task lock");
    if (IsRunning()) {
        m_mutex->Unlock();
        LOG(WARNING, "task with id %d has existed", m_task_info.task_id());
        return -1;
    }
    std::string task_stdout = m_workspace->GetPath() + "/./stdout";
    std::string task_stderr = m_workspace->GetPath() + "/./stderr";
    int stdout_fd = open(task_stdout.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    int stderr_fd = open(task_stderr.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    int cur_pid = getpid();
    std::vector<int> fds;
    common::util::GetProcessFdList(cur_pid, fds);
    m_child_pid = fork();
    //child
    if (m_child_pid == 0) {
        pid_t my_pid = getpid();
        int ret = setpgid(my_pid, my_pid);

        if (ret != 0) {
            return ret;
        }
        // do in child process,
        // all interface called in child process should be async-safe.
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

        RunInnerChildProcess(m_workspace->GetPath(), m_task_info.cmd_line());
    } else {
        close(stdout_fd);
        close(stderr_fd);
        m_mutex->Unlock();
        m_group_pid = m_child_pid;
        return 0;
    }
}

int CommandTaskRunner::ReStart(){


}


int CommandTaskRunner::Stop() {
    m_mutex->Lock();
    if (IsRunning() != 0) {
        m_mutex->Unlock();
        return 0;
    }
    LOG(INFO,"start to kill process group %d",m_group_pid);
    int ret = killpg(m_group_pid, 9);
    if(ret != 0){
        LOG(WARNING,"fail to kill process group %d",m_group_pid);
    }
    pid_t killed_pid = wait(&ret);
    if(killed_pid == -1){
        LOG(FATAL,"fail to kill process group %d",m_group_pid);
    }else{
        LOG(INFO,"kill child process %d successfully",killed_pid);
    }
    m_child_pid = -1;
    m_group_pid = -1;
    m_mutex->Unlock();
    return ret;
}

void CommandTaskRunner::RunInnerChildProcess(const std::string& root_path,
        const std::string& cmd_line) {
    chdir(root_path.c_str());
    execl("/bin/sh", "sh", "-c", cmd_line.c_str(), NULL);
    assert(0);
    _exit(127);
}

}



