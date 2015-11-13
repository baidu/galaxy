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

bool TerminateContact(int fdm, int connfd) {
    if (fdm < 0 || connfd < 0) {
        return false; 
    }
    const int INPUT_BUFFER_LEN = 1024 * 10;
    char input[INPUT_BUFFER_LEN];
    fd_set fd_in; 
    int ret = 0;
    int max_fd = connfd; 
    if (max_fd < fdm) {
        max_fd = fdm;
    }
    while (1) {
        FD_ZERO(&fd_in); 
        FD_SET(fdm, &fd_in);
        FD_SET(connfd, &fd_in);
        ret = ::select(max_fd + 1, &fd_in, NULL, NULL, NULL);
        switch (ret) {
            case -1 :  
                LOG(WARNING, "select err[%d: %s]\n", errno, strerror(errno));
                break;
            default : {
                if (FD_ISSET(connfd, &fd_in))  {
                    ret = ::read(connfd, input, sizeof(input)); 
                    if (ret > 0) {
                        ::write(fdm, input, ret); 
                    } else {
                        if (ret < 0) {
                            LOG(0, "read err[%d: %s]\n",
                                    errno,
                                    strerror(errno)); 
                            break;
                        }
                    }
                }         
                if (FD_ISSET(fdm, &fd_in)) {
                    ret = ::read(fdm, input, sizeof(input)); 
                    if (ret > 0) {
                        ::write(connfd, input, ret);
                    } else {
                        if (ret < 0) {
                            LOG(WARNING, "read err[%d: %s]\n",
                                    errno,
                                    strerror(errno)); 
                            break;
                        } 
                    }
                }
            }
        }
        if (ret < 0) {
            break; 
        }
    }
    if (ret < 0) {
        return false; 
    }
    return true;
} 

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


void AttachPod(std::string pod_id, int connfd) {
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
        return;
    } else if (response.has_status()
            && response.status() != ::baidu::galaxy::kOk) {
        LOG(WARNING, "response status %s\n",
                ::baidu::galaxy::Status_Name(response.status()).c_str()); 
        return;
    }

    if (response.pods_size() != 1) {
        LOG(WARNING, "pod size not 1[%d]\n", 
                        response.pods_size()); 
        return;
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
        return;
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
        LOG(WARNING, "terminate starting...\n");
        ret = TerminateContact(pty_fdm, connfd); 
        if (ret) {
            LOG(INFO, "terminate contact over\n"); 
        } else {
            fprintf(stderr, "terminate contact interrupt\n"); 
        }
        return;
    } 
    LOG(WARNING, "exec in initd failed %s\n", 
            baidu::galaxy::Status_Name(exec_response.status()).c_str());
    return;
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

    char pod_id[40];
    int ret = -1;
    while (1) {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        ret = read(connfd, pod_id, 36);
        if (ret != 36) {
            LOG(WARNING, "read WARNING\n");
            close(connfd);
            continue;
        }
        pod_id[36] = '\0';
        LOG(INFO, "read pod: %s\n", pod_id);
        AttachPod(std::string(pod_id), connfd);
        close(connfd);
    }
}


/* vim: set ts=4 sw=4 sts=4 tw=100 */
