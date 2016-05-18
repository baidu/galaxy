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

bool PrepareStdFds(const std::string& pwd,
                   const std::string& process_id,
                   int* stdout_fd,
                   int* stderr_fd) {
    if (stdout_fd == NULL || stderr_fd == NULL) {
        LOG(WARNING) << "prepare stdout_fd or stderr_fd NULL";
        return false;
    }
    file::Mkdir(pwd);
    std::string now_str_time;
    GetStrFTime(&now_str_time);
    pid_t pid = ::getpid();
    std::string str_pid = boost::lexical_cast<std::string>(pid);
    std::string stdout_file = pwd + "/" + process_id + "_stdout_" + str_pid + "_" + now_str_time;
    std::string stderr_file = pwd + "/" + process_id + "_stderr_" + str_pid + "_" + now_str_time;

    const int STD_FILE_OPEN_FLAG = O_CREAT | O_APPEND | O_WRONLY;
    const int STD_FILE_OPEN_MODE = S_IRWXU | S_IRWXG | S_IROTH;
    *stdout_fd = ::open(stdout_file.c_str(),
            STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);
    *stderr_fd = ::open(stderr_file.c_str(),
            STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);
    if (*stdout_fd == -1 || *stderr_fd == -1) {
        LOG(WARNING) << "open file " << stdout_file.c_str() << " failed, "\
                     << "err[" << errno << ":" << strerror(errno) << "]";
        return false;
    }
    return true;
}

// setpgid and chdir
void PrepareChildProcessEnvStep1(pid_t pid, const char* work_dir) {
    int ret = ::setpgid(pid, pid);
    if (ret != 0) {
        assert(0);
    }

    ret = ::chdir(work_dir);
    if (ret != 0) {
        assert(0);
    }
}

// dup fd
void PrepareChildProcessEnvStep2(const int stdin_fd,
                                 const int stdout_fd,
                                 const int stderr_fd,
                                 const std::vector<int>& fd_vector) {
    while (::dup2(stdout_fd, STDOUT_FILENO) == -1
            && errno == EINTR) {}
    while (::dup2(stderr_fd, STDERR_FILENO) == -1
            && errno == EINTR) {}
    while (stdin_fd >= 0
            && ::dup2(stdin_fd, STDIN_FILENO) == -1
            && errno == EINTR) {}
    for (size_t i = 0; i < fd_vector.size(); i++) {
        if (fd_vector[i] == STDOUT_FILENO
            || fd_vector[i] == STDERR_FILENO
            || fd_vector[i] == STDIN_FILENO) {
            // not close std fds
            continue;
        }
        ::close(fd_vector[i]);
    }
}

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

int ProcessManager::CreateProcess(const std::string& process_id,
                                  const std::string& cmd,
                                  const std::string& path) {
    {
        MutexLock scope_lock(&lock_);
        std::map<std::string, Process*>::iterator it = processes_.find(process_id);
        if (it != processes_.end() && proto::kProcessRunning == it->second->status) {
            LOG(INFO) << cmd.c_str() << " is already running";
            return -1;
        }
    }
    LOG(INFO) << "create process of command: " << cmd.c_str();

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
    if (!PrepareStdFds(path, process_id, &stdout_fd, &stderr_fd)) {
        if (stdout_fd != -1) {
            ::close(stdout_fd);
        }
        if (stderr_fd != -1) {
            ::close(stderr_fd);
        }
        LOG(WARNING) << "prepare for " << process_id.c_str() << " std file failed";
        return -1;
    }

//    int pty_fds = -1;
//    if (request->has_pty_file()) {
//        pty_fds = ::open(request->pty_file().c_str(), O_RDWR);
//        stdout_fd = pty_fds;
//        stderr_fd = pty_fds;
//        stdin_fd = pty_fds;
//    }

    // 3. Fork
    pid_t child_pid = ::fork();
    if (child_pid == -1) {
        LOG(WARNING) <<  "fork " << process_id << " failed, "\
            <<"err[" << errno << ":" << strerror(errno) << "]";
//        if (pty_fds != -1) {
//            ::close(pty_fds);
//        }
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
        PrepareChildProcessEnvStep1(my_pid, path.c_str());
//        // attach cgroup
//        for (int i = 0; i < request->cgroups_size(); i++) {
//            bool ok = AttachCgroup(request->cgroups(i).cgroup_path(), my_pid);
//            if (!ok) {
//                assert(0);
//            }
//        }
        PrepareChildProcessEnvStep2(stdin_fd, stdout_fd, stderr_fd, fd_vector);
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
            const_cast<char*>(cmd.c_str()),
            NULL};

        // prepare envs
        char* env[1];
//        for (int i = 0; i < request->envs_size(); i++) {
//            env[i] = const_cast<char*>(request->envs(i).c_str());
//        }
        env[0] = NULL;
        // exec
        ::execve("/bin/sh", argv, env);
        fprintf(stdout, "execve %s err[%d: %s]", cmd.c_str(), errno, strerror(errno));
        assert(0);
    }

//    // close child's std fds
//    if (pty_fds != -1) {
//        ::close(pty_fds);
//    }
    if (stdout_fd != -1) {
        ::close(stdout_fd);
    }
    if (stderr_fd != -1) {
        ::close(stderr_fd);
    }

    Process* process = new Process();
    process->process_id = process_id;
    process->pid = child_pid;
    process->status = proto::kProcessRunning;
    {
        MutexLock scope_lock(&lock_);
        processes_.insert(std::make_pair(process_id, process));
    }
    return 0;
}

int ProcessManager::QueryProcess(const std::string& process_id, Process& process) {
    MutexLock scope_lock(&lock_);
    std::map<std::string, Process*>::iterator it = processes_.find(process_id);
    if (it == processes_.end()) {
        LOG(WARNING) << "process: " << process_id.c_str() << " not exist";
        return -1;
    }
    process.process_id = process_id;
    process.pid = it->second->pid;
    process.status = it->second->status;
    process.exit_code = it->second->exit_code;
    LOG(INFO) << "process: " << process_id.c_str()\
        << ", status: " << proto::ProcessStatus_Name(it->second->status);
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
                LOG(WARNING) <<  "process: " << it->second->process_id.c_str()\
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
