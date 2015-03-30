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

extern int FLAGS_task_retry_times;

namespace galaxy {

int AbstractTaskRunner::IsRunning(){
   if (m_child_pid == -1) {
        return -1;
    }
    // check process exist
    int ret = ::kill(m_child_pid, 0);
    if(ret == 0 ){
        //check process status
        pid_t pid = waitpid(m_child_pid,&ret,WNOHANG);
        if(pid == -1){
            LOG(WARNING,"check process %d state error",m_child_pid);
            return -1;
        }else if(pid == 0){
            LOG(INFO,"process %d is running",m_child_pid);
        }else{
            LOG(WARNING,"process %d has gone",m_child_pid);
            //restart
            return -1;
        }
    }
    LOG(INFO, "check task %d error[%d:%s] ",
            m_task_info.task_id(),
            errno,
            strerror(errno));
    return ret;
}

int AbstractTaskRunner::Stop(){
    if (IsRunning() != 0) {
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
    return ret;

}

void AbstractTaskRunner::PrepareStart(std::vector<int>& fd_vector,int* stdout_fd,int* stderr_fd){
    pid_t current_pid = getpid();
    common::util::GetProcessFdList(current_pid, fd_vector);
    std::string task_stdout = m_workspace->GetPath() + "/./stdout";
    std::string task_stderr = m_workspace->GetPath() + "/./stderr";
    *stdout_fd = open(task_stdout.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    *stderr_fd = open(task_stderr.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
}

void AbstractTaskRunner::StartTaskAfterFork(std::vector<int>& fd_vector,int stdout_fd,int stderr_fd){
    pid_t my_pid = getpid();
    int ret = setpgid(my_pid, my_pid);
    if (ret != 0) {
        return ;
    }
   // do in child process,
   // all interface called in child process should be async-safe.
   // NOTE if dup2 will return errno == EINTR?
   while (dup2(stdout_fd, STDOUT_FILENO) == -1 && errno == EINTR) {}
   while (dup2(stderr_fd, STDERR_FILENO) == -1 && errno == EINTR) {}
   for (size_t i = 0; i < fd_vector.size(); i++) {
        if (fd_vector[i] == STDOUT_FILENO
                    || fd_vector[i] == STDERR_FILENO
                    || fd_vector[i] == STDIN_FILENO) {
                // do not deal with std input/output
            continue;
         }
         close(fd_vector[i]);
    }
    chdir(m_workspace->GetPath().c_str());
    execl("/bin/sh", "sh", "-c", m_task_info.cmd_line().c_str(), NULL);
    assert(0);
    _exit(127);
}
int AbstractTaskRunner::ReStart(){
    int max_retry_times = FLAGS_task_retry_times;
    if (m_task_info.has_fail_retry_times()) {
        max_retry_times = m_task_info.fail_retry_times();
    }
    if (m_has_retry_times
            >= max_retry_times) {
        return -1;
    }

    m_has_retry_times ++;
    if (IsRunning() == 0) {
        if (!Stop()) {
            return -1;
        }
    }

    return Start();
}


//start process
//1. fork a subprocess A
//2. exec command in the subprocess A
//TODO add workspace
int CommandTaskRunner::Start() {
    LOG(INFO, "start a task with id %d", m_task_info.task_id());
    if (IsRunning() == 0) {
        LOG(WARNING, "task with id %d has been runing", m_task_info.task_id());
        return -1;
    }
    int stdout_fd,stderr_fd;
    std::vector<int> fds;
    PrepareStart(fds,&stdout_fd,&stderr_fd);
    m_child_pid = fork();
    //child
    if (m_child_pid == 0) {
        StartTaskAfterFork(fds,stdout_fd,stderr_fd);
    } else {
        close(stdout_fd);
        close(stderr_fd);
        m_group_pid = m_child_pid;
    }
    return 0;
}

}



