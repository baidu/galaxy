// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <signal.h>

#include <sofa/pbrpc/pbrpc.h>
#include <gflags/gflags.h>

#include <logging.h>

#include "agent_impl.h"

DECLARE_string(flagfile);
DECLARE_string(agent_port);

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/)
{
    s_quit = true;
}

int main(int argc, char* argv[])
{
    FLAGS_flagfile = "./galaxy.flag";
    ::google::ParseCommandLineFlags(&argc, &argv, false);

    sofa::pbrpc::RpcServerOptions options;
    options.work_thread_num = 8;
    sofa::pbrpc::RpcServer rpc_server(options);

    baidu::galaxy::AgentImpl* agent_service = new baidu::galaxy::AgentImpl();

    if (!rpc_server.RegisterService(agent_service)) {
            return EXIT_FAILURE;
    }

    std::string server_host = std::string("0.0.0.0:") + FLAGS_agent_port;
    if (!rpc_server.Start(server_host)) {
            return EXIT_FAILURE;
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);
    while (!s_quit) {
        sleep(1);
    }   

    return EXIT_SUCCESS;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
