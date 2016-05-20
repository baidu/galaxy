// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "process_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

#include "utils/utils.h"
#include "protocol/galaxy.pb.h"

DECLARE_int32(process_manager_loop_wait_interval);

namespace baidu {
namespace galaxy {

ProcessManager::ProcessManager() :
    lock_(),
    background_pool_(1) {

    background_pool_.DelayTask(
        FLAGS_process_manager_loop_wait_interval,
        boost::bind(&ProcessManager::LoopWaitProcesses, this)
    );
}

ProcessManager::~ProcessManager() {
}

int ProcessManager::CreateProcess(const ProcessEnv& env,
                                  const ProcessContext* context) {
    {
        MutexLock scope_lock(&lock_);
        std::map<std::string, Process*>::iterator it =\
            processes_.find(context->process_id);
        if (it != processes_.end()
            && proto::kProcessRunning == it->second->status) {
            LOG(INFO) << context->cmd << " is already running";
            return -1;
        }
    }
    LOG(INFO) << "create process of command: " << context->cmd;

    // 1. collect initd fds
    std::vector<int> fd_vector;
//    std::string proc_path;
//    if (!GetProcPath(&proc_path)) {
//        response->set_status(kUnknown);
//        done->Run();
//        return;
//    }
//    std::vector<std::string> files;
//    if (!file::ListFiles(proc_path, &files)) {
//        LOG(WARNING, "list new proc failed");
//    }
//    proc_path.append(boost::lexical_cast<std::string>(::getpid()));
//    proc_path.append("/fd/");
//    if (!file::ListFiles(proc_path, &files)) {
//        LOG(WARNING, "list new proc failed");
//        response->set_status(kInputError);
//        done->Run();
//        return;
//    }
//    for (size_t i = 0; i < files.size(); i++) {
//        if (files[i] == "." || files[i] == "..") {
//            continue;
//        }
//        fd_vector.push_back(::atoi(files[i].c_str()));
//    }
//    // check if need chroot
//    bool is_chroot = request->has_chroot_path();
//    if (is_chroot) {
//        LOG(WARNING, "chroot %s", request->chroot_path().c_str());
//    }
//
    // 2. prepare std fds for child
    int stdin_fd = -1;
    int stdout_fd = -1;
    int stderr_fd = -1;
    if (!process::PrepareStdFds(context->work_dir, context->process_id,
                                &stdout_fd, &stderr_fd)) {
        if (stdout_fd != -1) {
            ::close(stdout_fd);
        }
        if (stderr_fd != -1) {
            ::close(stderr_fd);
        }
        LOG(WARNING) << "prepare " << context->process_id << " std file failed";
        return -1;
    }

    // 3. Fork
    pid_t child_pid = ::fork();
    if (child_pid == -1) {
        LOG(WARNING) <<  "fork " << context->process_id << " failed, "\
            <<"err[" << errno << ":" << strerror(errno) << "]";
        if (stdout_fd != -1) {
            ::close(stdout_fd);
        }
        if (stderr_fd != -1) {
            ::close(stderr_fd);
        }
        return -1;
    } else if (child_pid == 0) {
        // setpgid  & chdir
        pid_t my_pid = ::getpid();
        process::PrepareChildProcessEnvStep1(my_pid,
                                             context->work_dir.c_str());
        // attach cgroup
        for (unsigned i = 0; i < env.cgroup_paths.size(); i++) {
            LOG(WARNING) << "process: " << my_pid\
                << " attach cgroup: " << env.cgroup_paths[i];
//            bool ok = cgroup::AttachCgroup(process_env.cgroup_paths[i], my_pid);
//            if (!ok) {
//                assert(0);
//            }
        }

        process::PrepareChildProcessEnvStep2(stdin_fd, stdout_fd,
                                             stderr_fd, fd_vector);
//        if (is_chroot) {
//            if (::chroot(request->chroot_path().c_str()) != 0) {
//                assert(0);
//            }
//        }
//        // set user
//        if (request->has_user()
//                && !user::Su(request->user())) {
//            assert(0);
//        }

        // prepare argv
        char* argv[] = {
            const_cast<char*>("sh"),
            const_cast<char*>("-c"),
            const_cast<char*>(context->cmd.c_str()),
            NULL};

        // prepare envs
        char* envs[env.envs.size() + 1];
        envs[env.envs.size()]= NULL;
        for (unsigned i = 0; i < env.envs.size(); i++) {
            envs[i] = const_cast<char*>(env.envs[i].c_str());
        }

        // different with deploy and main process
        const DownloadProcessContext* download_context =\
             dynamic_cast<const DownloadProcessContext*>(context);
        if (NULL != download_context) {
            // do deploy, if file no change then exit
            fprintf(stdout, "execve deploy process\n");
            std::string md5;
            if (file::IsExists(download_context->dst_path)
                && file::GetFileMd5(download_context->dst_path, md5)
                && md5 == download_context->version) {
                    // data not change
                    fprintf(stdout, "data not change, md5: %s", md5.c_str());
                    exit(0);
            }
        }
        // do exec
        ::execve("/bin/sh", argv, envs);
        fprintf(stdout, "execve %s err[%d: %s]\n",
                context->cmd.c_str(), errno, strerror(errno));
        assert(0);
    }

    if (stdout_fd != -1) {
        ::close(stdout_fd);
    }
    if (stderr_fd != -1) {
        ::close(stderr_fd);
    }

    Process* process = new Process();
    process->process_id = context->process_id;
    process->pid = child_pid;
    process->status = proto::kProcessRunning;
    {
        MutexLock scope_lock(&lock_);
        processes_.insert(std::make_pair(context->process_id, process));
    }
    return 0;
}

int ProcessManager::DeleteProcess(const std::string& process_id) {
    MutexLock scope_lock(&lock_);
    std::map<std::string, Process*>::iterator it = processes_.find(process_id);
    if (it == processes_.end()) {
        LOG(INFO) << "process: " << process_id << " not exist";
        return 0;
    }
    processes_.erase(it);
    return 0;
}

int ProcessManager::QueryProcess(const std::string& process_id,
                                 Process& process) {
    MutexLock scope_lock(&lock_);
    std::map<std::string, Process*>::iterator it = processes_.find(process_id);
    if (it == processes_.end()) {
        LOG(WARNING) << "process: " << process_id << " not exist";
        return -1;
    }
    process.process_id = process_id;
    process.pid = it->second->pid;
    process.status = it->second->status;
    process.exit_code = it->second->exit_code;
    return 0;
}

void ProcessManager::LoopWaitProcesses() {
    MutexLock scope_lock(&lock_);
    int status = 0;
    pid_t pid = ::waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        std::map<std::string, Process*>::iterator it = processes_.begin();
        for (; it != processes_.end(); ++it) {
            if (it->second->pid == pid) {
                it->second->status = proto::kProcessFinished;
                if (WIFEXITED(status)) {
                    it->second->exit_code = WEXITSTATUS(status);
                    if (0 != it->second->exit_code) {
                        it->second->status = proto::kProcessFailed;
                    }
                } else if (WIFSIGNALED(status)) {
                    it->second->exit_code = 128 + WTERMSIG(status);
                    if (WCOREDUMP(status)) {
                        it->second->status = proto::kProcessCoreDump;
                    } else {
                        it->second->status = proto::kProcessKilled;
                    }
                }
                LOG(WARNING) <<  "process: " << it->second->process_id\
                     << ", pid: " << pid\
                     << ", exit code: " << it->second->exit_code;
                break;
            }
        }
    }

    background_pool_.DelayTask(
        FLAGS_process_manager_loop_wait_interval,
        boost::bind(&ProcessManager::LoopWaitProcesses, this)
    );
    return;
}

} // ending namespace galaxy
} // ending namespace baidu
