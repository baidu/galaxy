// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "master_impl.h"

#include <sofa/pbrpc/pbrpc.h>

#include <stdio.h>
#include <signal.h>
#include <gflags/gflags.h>

DECLARE_string(master_port);
DECLARE_int32(task_deploy_timeout);
DECLARE_string(master_checkpoint_path);
DECLARE_int32(master_safe_mode_last);

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/)
{
    s_quit = true;
}

int main(int argc, char* argv[])
{
    ::google::ParseCommandLineFlags(&argc, &argv, true);

    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    galaxy::MasterImpl* master_service = new galaxy::MasterImpl();

    if (!master_service->Recover()) {
        fprintf(stderr, "master recover from checkpoint failed\n"); 
        return EXIT_FAILURE;
    }

    if (!rpc_server.RegisterService(master_service)) {
            return EXIT_FAILURE;
    }

    std::string server_host = std::string("0.0.0.0:") + FLAGS_master_port;
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
