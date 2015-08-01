// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gce/utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "logging.h"

namespace baidu {
namespace galaxy {

void GetStrFTime(std::string* time_str) {
    const int TIME_BUFFER_LEN = 100;        
    char time_buffer[TIME_BUFFER_LEN];
    time_t seconds = time(NULL);
    struct tm t;
    localtime_r(&seconds, &t);
    size_t len = strftime(time_buffer,
            TIME_BUFFER_LEN - 1,
            "%Y%m%d%H%M%S",
            &t);
    time_buffer[len] = '\0';
    time_str->clear();
    time_str->append(time_buffer, len);
    return ;
}

namespace process { 

bool PrepareStdFds(const std::string& pwd, 
                   int* stdout_fd, 
                   int* stderr_fd) {
    if (stdout_fd == NULL || stderr_fd == NULL) {
        LOG(WARNING, "prepare stdout_fd or stderr_fd NULL"); 
        return false;
    }
    std::string now_str_time;
    GetStrFTime(&now_str_time);
    std::string stdout_file = pwd + "/stdout_" + now_str_time;
    std::string stderr_file = pwd + "/stderr_" + now_str_time;

    const int STD_FILE_OPEN_FLAG = O_CREAT | O_APPEND | O_WRONLY;
    const int STD_FILE_OPEN_MODE = S_IRWXU | S_IRWXG | S_IROTH;
    *stdout_fd = ::open(stdout_file.c_str(), 
            STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);
    *stderr_fd = ::open(stderr_file.c_str(),
            STD_FILE_OPEN_FLAG, STD_FILE_OPEN_MODE);
    if (*stdout_fd == -1 || *stderr_fd == -1) {
        LOG(WARNING, "open file failed err[%d: %s]",
                errno, strerror(errno));
        return false; 
    }
    return true;
}

void GetProcessOpenFds(const pid_t pid, 
                       std::vector<int>* fd_vector) {
    if (fd_vector == NULL) {
        return; 
    }

    // pid_t current_pid = ::getpid();
    std::string pid_path = "/proc/";
    pid_path.append(boost::lexical_cast<std::string>(pid));
    pid_path.append("/fd/");

    std::vector<std::string> fd_files;
    if (!file::ListFiles(pid_path, &fd_files)) {
        LOG(WARNING, "list %s failed", pid_path.c_str());    
        return;
    }

    for (size_t i = 0; i < fd_files.size(); i++) {
        if (fd_files[i] == "." || fd_files[i] == "..") {
            continue; 
        }
        fd_vector->push_back(::atoi(fd_files[i].c_str()));    
    }
    return;
}

void PrepareChildProcessEnvStep1(pid_t pid, const char* work_dir) {
    // pid_t my_pid = ::getpid();
    int ret = ::setpgid(pid, pid);
    if (ret != 0) {
        assert(0); 
    }

    ret = ::chdir(work_dir);
    if (ret != 0) {
        assert(0);
    }
}

void PrepareChildProcessEnvStep2(const int stdout_fd, 
                                 const int stderr_fd, 
                                 const std::vector<int>& fd_vector) {
    while (::dup2(stdout_fd, STDOUT_FILENO) == -1 
            && errno == EINTR) {}
    while (::dup2(stderr_fd, STDERR_FILENO) == -1
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

} // ending namespace process

namespace file {

bool ListFiles(const std::string& dir_path,
        std::vector<std::string>* files) {
    if (files == NULL) {
        return false;
    }

    DIR* dir = ::opendir(dir_path.c_str());

    if (dir == NULL) {
        LOG(WARNING, "opendir %s failed err[%d: %s]",
                dir_path.c_str(),
                errno,
                strerror(errno));
        return false;
    }

    struct dirent* entry;

    while ((entry = ::readdir(dir)) != NULL) {
        files->push_back(entry->d_name);
    }

    closedir(dir);
    return true;
}

bool IsExists(const std::string& path) {
    struct stat stat_buf;
    int ret = ::lstat(path.c_str(), &stat_buf);
    if (ret < 0) {
        return false; 
    }
    return true;
}

bool Mkdir(const std::string& dir_path) {
    const int dir_mode = 0777;
    int ret = ::mkdir(dir_path.c_str(), dir_mode); 
    return ret == 0;
    // TODO 
    //if (ret != 0 && errno == ) {
    //     
    //} 
}

}   // ending namespace file
}   // ending namespace galaxy
}   // ending namespace baidu
