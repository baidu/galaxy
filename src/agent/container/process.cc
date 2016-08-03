// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "process.h"

#include "util/util.h"

#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>
#include <glog/logging.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <sched.h>

#include <iostream>
#include <sstream>


namespace baidu {
namespace galaxy {
namespace container {

Process::Process() :
    pid_(-1)
{
}

Process::~Process()
{
}

pid_t Process::SelfPid()
{
    return getpid();
}


baidu::galaxy::util::ErrorCode Process::Kill(pid_t pid)
{
    int ret = ::kill(pid, SIGKILL);
    int err = errno;
    if (0 == ret) {
        return ERRORCODE_OK;
    }

    if (err == ESRCH) {
        return PERRORCODE(0, err, "pid %d not exist", (int)pid);
    }

    return PERRORCODE(-1, err, "failed in killing pid %d: ", (int)pid);
}

int Process::RedirectStderr(const std::string& path)
{
    stderr_path_ = path;
    return 0;
}

int Process::RedirectStdout(const std::string& path)
{
    stdout_path_ = path;
    return 0;
}

int Process::Clone(boost::function<int (void*) > routine, void* param, int32_t flag)
{
    assert(!stderr_path_.empty());
    assert(!stdout_path_.empty());
    Context* context = new Context();
    std::vector<int> fds;
    int pid = SelfPid();

    if (0 != ListFds(pid, fds)) {
        LOG(WARNING) << "failed to list opened fd of process " << pid;
        return -1;
    }

    context->fds.swap(fds);
    const int STD_FILE_OPEN_FLAG = O_CREAT | O_APPEND | O_WRONLY;
    const int STD_FILE_OPEN_MODE = S_IRWXU | S_IRWXG | S_IROTH;
    int stdout_fd = ::open(stdout_path_.c_str(), STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);

    if (-1 == stdout_fd) {
        LOG(WARNING) << "open file failed: " << stdout_path_;
        delete context;
        return -1;
    }

    int stderr_fd = ::open(stderr_path_.c_str(), STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);

    if (-1 == stderr_fd) {
        LOG(WARNING) << "open file failed: " << stderr_path_;
        delete context;
        return -1;
    }

    context->stderr_fd = stderr_fd;
    context->stdout_fd = stdout_fd;
    context->self = this;
    context->routine = routine;
    context->parameter = param;

    const static int CLONE_FLAG = CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD;
    const static int CLONE_STACK_SIZE = 1024 * 1024;
    static char CLONE_STACK[CLONE_STACK_SIZE];
    pid = ::clone(&Process::CloneRoutine,
            CLONE_STACK + CLONE_STACK_SIZE,
            CLONE_FLAG,
            context);

    int en = errno;
    if (pid != 0) {
        ::close(context->stdout_fd);
        ::close(context->stderr_fd);
        pid_ = pid;
    }

    if (-1 == pid_) {
        LOG(WARNING) << "clone failed: " << strerror(en);
        return -1;
    }

    return pid_;
}

int Process::CloneRoutine(void* param)
{
    assert(NULL != param);
    Context* context = (Context*) param;
    assert(context->stdout_fd > 0);
    assert(context->stderr_fd > 0);
    assert(NULL != context->self);
    assert(NULL != context->routine);

    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    close(STDIN_FILENO);
    while (::dup2(context->stdout_fd, STDOUT_FILENO) == -1
            && errno == EINTR) {
    }

    while (::dup2(context->stderr_fd, STDERR_FILENO) == -1
            && errno == EINTR) {
    }


    for (size_t i = 0; i < context->fds.size(); i++) {
        if (STDOUT_FILENO == context->fds[i]
                || STDERR_FILENO == context->fds[i]
                || STDIN_FILENO == context->fds[i]) {
            // not close std fds
            continue;
        }

        ::close(context->fds[i]);
    }

    pid_t pid = SelfPid();

    if (0 != ::setpgid(pid, pid)) {
        return -1;
    }

    return context->routine(context->parameter);
}



int Process::Fork(boost::function<int (void*) > routine, void* param)
{
    assert(0);
    return -1;
}

pid_t Process::Pid()
{
    return pid_;
}

int Process::Wait(int& status)
{
    if (pid_ <= 0) {
        return -1;
    }

    if (pid_ != ::waitpid(pid_, &status, 0)) {
        return -1;
    }

    return 0;
}

int Process::ListFds(pid_t pid, std::vector<int>& fd)
{
    std::stringstream ss;
    ss << "/proc/" << (int)pid << "/fd";
    boost::filesystem::path path(ss.str());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec)) {
        return -1;
    }

    boost::filesystem::directory_iterator begin(path);
    boost::filesystem::directory_iterator end;

    // exclude .. and .
    for (boost::filesystem::directory_iterator iter = begin; iter != end; iter++) {
        //std::string file_name = iter->filename();
        std::string file_name = iter->path().filename().string();

        if (file_name != "." && file_name != "..") {
            fd.push_back(atoi(file_name.c_str()));
        }
    }

    return 0;
}


void Process::Reload(pid_t pid) {
    pid_ = pid;
}

} //namespace container
} //namespace galaxy
} //namespace baidu

