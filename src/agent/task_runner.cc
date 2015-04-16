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
#include <sstream>
#include <sys/types.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "common/logging.h"
#include "common/util.h"
#include "downloader_manager.h"
#include "agent/resource_collector_engine.h"
#include "agent/utils.h"

extern int FLAGS_task_retry_times;

namespace galaxy {

static const std::string RUNNER_META_PREFIX = "task_runner_";

int AbstractTaskRunner::IsRunning(){
    if (m_child_pid == -1) {
        return -1;
    }
    // check process exist
    int ret = ::kill(m_child_pid, 0);
    if(ret == 0 ){
        //check process status
        pid_t pid = waitpid(m_child_pid,&ret,WNOHANG);
        if(pid == 0 ){
            LOG(INFO,"process %d is running",m_child_pid);
            return 0;
        }else if(pid == -1){
            LOG(WARNING,"fail to check process %d state",m_child_pid);
            return -1;
        }
        else{
            if(WIFEXITED(ret)){
                int exit_code = WEXITSTATUS(ret);
                if(exit_code == 0 ){
                    //normal exit
                    LOG(INFO,"process %d exits successfully",m_child_pid);
                    return 1;
                }
                LOG(FATAL,"process %d exits with err code %d", exit_code);
            }
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
    m_child_pid = -1;
    m_group_pid = -1;

    if (killed_pid == -1){
        LOG(FATAL,"fail to kill process group %d",m_group_pid);
        return -1;
    }else{
        StopPost();
        LOG(INFO,"kill child process %d successfully",killed_pid);
        return 0;
    }
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
    char *argv[] = {"sh","-c",const_cast<char*>(m_task_info.cmd_line().c_str()),NULL};
    std::stringstream task_id_env;
    task_id_env <<"TASK_ID="<<m_task_info.task_offset();
    std::stringstream task_num_env;
    task_num_env <<"TASK_NUM="<<m_task_info.job_replicate_num();
    char *env[] = {const_cast<char*>(task_id_env.str().c_str()),
                   const_cast<char*>(task_num_env.str().c_str()),
                   NULL};
    execve("/bin/sh", argv, env);
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

void CommandTaskRunner::StopPost() {
    if (collector_ != NULL) {
        collector_->Clear();
    }
}

int CommandTaskRunner::Prepare() {
    std::string uri = m_task_info.task_raw();
    std::string path = m_workspace->GetPath();
    path.append("/");
    path.append("tmp.tar.gz");
    m_task_state = DEPLOYING;
    // set deploying state
    DownloaderManager* downloader_handler = DownloaderManager::GetInstance();
    downloader_handler->DownloadInThread(
            uri,
            path,
            boost::bind(&CommandTaskRunner::StartAfterDownload, this, _1));
    return 0;
}

CommandTaskRunner::~CommandTaskRunner() {
    ResourceCollectorEngine* engine 
        = GetResourceCollectorEngine();
    engine->DelCollector(collector_id_);
    if (collector_ != NULL) {
        delete collector_; 
        collector_ = NULL;
    }
}

void CommandTaskRunner::StartAfterDownload(int ret) {
    if (ret == 0) {
        std::string tar_cmd = "cd " + m_workspace->GetPath() + " && tar -xzf tmp.tar.gz";
        int status = system(tar_cmd.c_str());
        if (status != 0) {
            LOG(WARNING, "tar -xf failed");
            return;
        }
        Start();
    }
    // set deploy failed state
}

void CommandTaskRunner::Status(TaskStatus* status) {
    if (collector_ != NULL) {
        status->set_cpu_usage(collector_->GetCpuUsage()); 
        status->set_memory_usage(collector_->GetMemoryUsage());
        LOG(WARNING, "cpu usage %f memory usage %ld", 
                status->cpu_usage(), status->memory_usage());
    }

    int ret = IsRunning();
    if (ret == 0) {
        m_task_state = RUNNING;
        status->set_status(RUNNING);
    }
    else if (ret == 1) {
        m_task_state = COMPLETE; 
        status->set_status(COMPLETE);
    }
    else if (m_task_state == DEPLOYING) {
        status->set_status(DEPLOYING); 
    }
    else {
        if (ReStart() == 0) {
            m_task_state = RESTART;
            status->set_status(RESTART); 
        }
        else {
            m_task_state = ERROR;
            status->set_status(ERROR); 
        }
    }
    return;
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
    //sequence_id_ ++;
    m_child_pid = fork();
    //child
    if (m_child_pid == 0) {
        pid_t my_pid = getpid();
        int ret = setpgid(my_pid, my_pid);
        if (ret != 0) {
            assert(0);
        }
        std::string meta_file = persistence_path_dir_ 
            + "/" + RUNNER_META_PREFIX 
            + boost::lexical_cast<std::string>(sequence_id_);
        int meta_fd = open(meta_file.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
        if (meta_fd == -1) {
            assert(0);
        }
        int64_t value = my_pid;
        LOG(WARNING, "value = %d", my_pid);
        int len = write(meta_fd, (void*)&value, sizeof(value));
        if (len == -1) {
            close(meta_fd);
            assert(0);
        }
        if (0 != fsync(meta_fd)) {
            close(meta_fd); 
            assert(0);
        }
        close(meta_fd);
        StartTaskAfterFork(fds,stdout_fd,stderr_fd);
    } else {
        close(stdout_fd);
        close(stderr_fd);
        m_group_pid = m_child_pid;
        // NOTE not multi thread safe 
        if (collector_ == NULL) {
            collector_ = new ProcResourceCollector(m_child_pid); 
            ResourceCollectorEngine* engine 
                = GetResourceCollectorEngine(); 
            collector_id_ = engine->AddCollector(collector_);
        }
        else {
            collector_->ResetPid(m_child_pid); 
        }
    }
    return 0;
}

bool CommandTaskRunner::RecoverRunner(const std::string& persistence_path) {
    std::vector<std::string> files;    
    if (!GetDirFilesByPrefix(
                persistence_path, 
                RUNNER_META_PREFIX, 
                &files)) {
        LOG(WARNING, "get meta files failed"); 
        return false;
    }
    LOG(DEBUG, "get meta files size %lu", files.size());
    if (files.size() == 0) {
        return true; 
    }
    int max_seq_id = -1;
    std::string last_meta_file;
    for (size_t i = 0; i < files.size(); i++) {
        std::string file = files[i];
        size_t pos = file.find(RUNNER_META_PREFIX);
        if (pos == std::string::npos) {
            continue;  
        }

        if (pos + RUNNER_META_PREFIX.size() >= file.size()) {
            LOG(WARNING, "meta file format err %s", file.c_str()); 
            continue;
        }

        int cur_id = atoi(file.substr(pos + RUNNER_META_PREFIX.size()).c_str());
        if (max_seq_id < cur_id) {
            max_seq_id = cur_id; 
            last_meta_file = file;
        }
    }
    if (max_seq_id < 0) {
        return false; 
    }
    
    std::string meta_file = persistence_path + "/" + last_meta_file;
    LOG(DEBUG, "start to recover %s", meta_file.c_str());
    int fin = open(meta_file.c_str(), O_RDONLY);
    if (fin == -1) {
        LOG(WARNING, "open meta file failed %s err[%d: %s]", 
                meta_file.c_str(),
                errno,
                strerror(errno)); 
        return false;
    }

    size_t value;
    int len = read(fin, (void*)&value, sizeof(value));
    if (len == -1) {
        LOG(WARNING, "read meta file failed err[%d: %s]",
                errno,
                strerror(errno)); 
        close(fin);
        return false;
    }
    close(fin);

    LOG(DEBUG, "recove gpid %lu", value);
    int ret = killpg((pid_t)value, 9);
    if (ret != 0 && errno != ESRCH) {
        LOG(WARNING, "fail to kill process group %lu", value); 
        return false;
    }
    return true;
}

}
