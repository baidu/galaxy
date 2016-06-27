/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file prison_breaker.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/05/31 13:41:12
 * @brief 
 *  
 **/

#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <sys/mount.h>

#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/algorithm/string/split.hpp"
#include "util/input_stream_file.h"
#include <iostream>
#include <vector>
#include <string>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)

#ifndef setns
#define setns(fd) syscall(308, 0, fd)
#endif

bool GetProcessInfo(const std::string& root_path, int& pid);

int main(int argc, char *argv[])
{

    if (argc != 2 && argc != 3) {
        std::cerr << "usage: " << argv[0] << " container_id [work_dir]\n";
        return 0;
    }

    std::string container_id = argv[1];
    std::string work_dir = "/home/galaxy3/agent/work_dir";
    if (argc == 3) {
        work_dir = argv[2];
    }

    const std::string root_path = work_dir + "/" + container_id;
    int appworker_pid = -1;
    if (!GetProcessInfo(root_path, appworker_pid)) {
        std::cerr << "get container info failed\n";
        return -1;
    }

    int fd;
    char buf[1024] = {0};
    snprintf(buf, sizeof buf, "/proc/%d/ns/pid", appworker_pid);
    fd = open(buf, O_RDONLY);
    if (fd == -1) {
        std::cerr << "open failed " << buf << std::endl;
        return -1;
    }
    if (setns(fd) == -1)        /* Join that namespace */
      errExit("setns");

    snprintf(buf, sizeof buf, "/proc/%d/ns/mnt", appworker_pid);
    fd = open(buf, O_RDONLY);
    if (fd == -1) {
        std::cerr << "open failed " << buf << std::endl;
        return -1;
    }

    if (setns(fd) == -1)        /* Join that namespace */
      errExit("setns");


    const char* mounts[] = {"/proc", NULL};
    int i = 0;
    const char* x = mounts[i];
    while (x != NULL) {
        if (strcmp(mounts[i], "/proc") != 0) {
            std::string target = root_path + mounts[i];
            mount(mounts[i], target.c_str(), "", MS_BIND, NULL);
        } else {
            std::string target = root_path + mounts[i];
            mount("proc", target.c_str(), "proc", 0, NULL);
        }
        i++;
        x = mounts[i];
    }

    if (0 != chroot(root_path.c_str())) {
        errExit("chroot");
    }


    if (0 != chdir("/")) {
        errExit("chdir");
    }

    pid_t pid = fork();

    if (pid == 0) {
        const char* cmd[] = {"sh", "-c", "/bin/sh -l", NULL};
        execv("/bin/sh", (char**)cmd);
        perror("hh");
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("mb");
    }
    std::cerr << "what happend ..." << std::endl;
    return 0;
}


bool GetProcessInfo(const std::string& path, int& pid) {
    const std::string property = path + "/container.property";
    boost::system::error_code ec;
    if (!boost::filesystem::exists(property, ec)) {
        return false;
    }

    baidu::galaxy::file::InputStreamFile file(property);
    if (!file.IsOpen()) {
        return false;
    }

    std::string line;
    while (!file.Eof()) {
        file.ReadLine(line);
        if (line.size() < 5) {
            continue;
        }
        if (boost::starts_with(line, "appworker :")) {
            std::vector<std::string> v;
            boost::split(v, line, boost::is_any_of(":"));
            if (v.size() != 2) {
                return false;
            }
            pid = atoi(v[1].c_str());
            return true;
        }
    }
    return false;
}
