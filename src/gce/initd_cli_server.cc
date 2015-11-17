// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "gflags/gflags.h"
#include "proto/agent.pb.h"
#include "proto/initd.pb.h"
#include "rpc/rpc_client.h"
#include "tprinter.h"
#include "string_util.h"
#include "logging.h"

DEFINE_string(initd_endpoint, "", "initd endpoint");
DEFINE_string(user, "", "use user");
DEFINE_string(chroot, "", "chroot path");
DEFINE_string(LINES, "39", "env values");
DEFINE_string(COLUMNS, "139", "env values");
DEFINE_string(TERM, "xterm-256color", "env values");
DECLARE_string(agent_port);
DECLARE_string(agent_default_user);
DECLARE_string(flagfile);
DECLARE_int32(cli_server_port);
DEFINE_string(pod_id, "", "pod id");

using baidu::common::INFO;
using baidu::common::WARNING;


bool PreparePty(int* fdm, std::string* pty_file) {
    if (pty_file == NULL || fdm == NULL) {
        return false; 
    }
    *fdm = -1;
    *fdm = ::posix_openpt(O_RDWR);
    if (*fdm < 0) {
        LOG(WARNING, "posix_openpt err[%d: %s]",
                errno, strerror(errno)); 
        return false;
    }

    int ret = ::grantpt(*fdm);
    if (ret != 0) {
        LOG(WARNING, "grantpt err[%d: %s]", 
                errno, strerror(errno)); 
        ::close(*fdm);
        return false;
    }

    ret = ::unlockpt(*fdm);
    if (ret != 0) {
        LOG(WARNING, "unlockpt err[%d: %s]", 
                errno, strerror(errno));
        ::close(*fdm);
        return false;
    }
    pty_file->clear();
    pty_file->append(::ptsname(*fdm));
    return true;
}


int GetPodFd(std::string pod_id) {
    ::baidu::galaxy::Agent_Stub* agent;    
    ::baidu::galaxy::RpcClient* rpc_client = 
        new ::baidu::galaxy::RpcClient();
    std::string endpoint("127.0.0.1:");
    endpoint.append(FLAGS_agent_port);
    rpc_client->GetStub(endpoint, &agent);

    ::baidu::galaxy::ShowPodsRequest request;
    ::baidu::galaxy::ShowPodsResponse response;
    request.set_podid(pod_id);
    bool ret = rpc_client->SendRequest(agent,
            &::baidu::galaxy::Agent_Stub::ShowPods,
            &request,
            &response, 5, 1);
    if (!ret) {
        LOG(WARNING, "rpc failed\n");
        return -1;
    } else if (response.has_status()
            && response.status() != ::baidu::galaxy::kOk) {
        LOG(WARNING, "response status %s\n",
                ::baidu::galaxy::Status_Name(response.status()).c_str()); 
        return -1;
    }
    if (response.pods_size() != 1) {
       LOG(WARNING, "pod size not 1[%d]\n", 
                        response.pods_size()); 
        return -1;
    }
    const ::baidu::galaxy::PodPropertiy& pod = response.pods(0);

    FLAGS_initd_endpoint = pod.initd_endpoint();
    FLAGS_chroot = pod.pod_path();
    ::baidu::galaxy::Initd_Stub* initd;
    std::string initd_endpoint(FLAGS_initd_endpoint);
    rpc_client->GetStub(initd_endpoint, &initd);

    std::string pty_file;
    int pty_fdm = -1;
    if (!PreparePty(&pty_fdm, &pty_file)) {
        LOG(WARNING, "prepare pty failed\n"); 
        return -1;
    }

    baidu::galaxy::ExecuteRequest exec_request;
    // TODO unqic key 
    exec_request.set_key("client");
    exec_request.set_commands("/bin/bash");
    exec_request.set_path(".");
    exec_request.set_pty_file(pty_file);
    if (FLAGS_user != "") {
        exec_request.set_user(FLAGS_user);
    }
    if (FLAGS_chroot != "") {
        LOG(WARNING, "%s\n", FLAGS_chroot.c_str());
        exec_request.set_chroot_path(FLAGS_chroot); 
    }
    std::string* lines_env = exec_request.add_envs();
    lines_env->append("LINES=");
    lines_env->append(FLAGS_LINES);
    std::string* columns_env = exec_request.add_envs();
    columns_env->append("COLUMNS=");
    columns_env->append(FLAGS_COLUMNS);
    std::string* xterm_env = exec_request.add_envs();
    xterm_env->append("TERM=");
    xterm_env->append(FLAGS_TERM);
    baidu::galaxy::ExecuteResponse exec_response;
    ret = rpc_client->SendRequest(initd,
                            &baidu::galaxy::Initd_Stub::Execute,
                            &exec_request,
                            &exec_response, 5, 1);
    if (ret && exec_response.status() == baidu::galaxy::kOk) {
        LOG(INFO, "terminate starting...\n");
        return pty_fdm;
    } 
    LOG(WARNING, "exec in initd failed %s\n", 
            baidu::galaxy::Status_Name(exec_response.status()).c_str());
    return -1;
}

int main(int argc, char* argv[]) {
    FLAGS_flagfile="galaxy.flag";
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(FLAGS_cli_server_port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 100);

    const int BUFFER_LEN = 10 * 1024;
    char pod_id[40];
    char buffer[BUFFER_LEN];
    int ret = -1;
    int max_fd = listenfd;
    fd_set fd_in;
    
    boost::unordered_map<int, int> sock2pty;
    while (1) {
        FD_ZERO(&fd_in);
        FD_SET(listenfd, &fd_in);
        for (boost::unordered_map<int, int>::iterator it = sock2pty.begin();
                it != sock2pty.end(); ++it) {
            FD_SET(it->first, &fd_in);
            FD_SET(it->second, &fd_in);
        }
        ret = ::select(max_fd + 1, &fd_in, NULL, NULL, NULL);
        if (ret < 0) {
            LOG(WARNING, "select err[%d: %s]\n", errno, strerror(errno));
            continue;
        }
        if (FD_ISSET(listenfd, &fd_in)) {
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
            ret = read(connfd, pod_id, 36);
            if (ret != 36) {
                LOG(WARNING, "read pod id error");
                close(connfd);
            }
            pod_id[36] = '\0';
            LOG(INFO, "read pod: %s\n", pod_id);
            int ptyfd = GetPodFd(pod_id);
            if (ptyfd < 0) {
                LOG(WARNING, "create pty fd error\n");
                continue;
            }
            sock2pty[connfd] = ptyfd;
            if (ptyfd > max_fd) {
                max_fd = ptyfd;
            }
            if (connfd > max_fd) {
                max_fd = connfd;
            }
        }
        boost::unordered_set<int> broken_sock;
        for (boost::unordered_map<int, int>::iterator it = sock2pty.begin();
                it != sock2pty.end(); ++it) {
            if (FD_ISSET(it->first, &fd_in)) {
                ret = ::read(it->first, buffer, BUFFER_LEN);
                if (ret > 0) {
                    ::write(it->second, buffer, ret);
                } else if (ret < 0) {
                    LOG(WARNING, "read from sock err[%d: %s]", 
                            errno, strerror(errno));
                    broken_sock.insert(it->first);
                    continue;
                }
            }
            if (FD_ISSET(it->second, &fd_in)) {
                ret = ::read(it->second, buffer, BUFFER_LEN);
                if (ret > 0) {
                    ::write(it->first, buffer, ret);
                } else if (ret < 0) {
                    LOG(WARNING, "read from pty err[%d, %s]",
                            errno, strerror(errno));
                    broken_sock.insert(it->first);
                }
            }
        } 
        for (boost::unordered_set<int>::iterator it = broken_sock.begin();
                it != broken_sock.end(); ++it) {
            LOG(INFO, "connection from sock[%d] to pty[%d] ends up", 
                    *it, sock2pty[*it]);
            close(*it);
            close(sock2pty[*it]);
            sock2pty.erase(*it);
        }
    }
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */

