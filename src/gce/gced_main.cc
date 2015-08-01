// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <signal.h>
#include "gced_impl.h"
#include "sofa/pbrpc/pbrpc.h"
#include "logging.h"
#include "gflags/gflags.h"

DECLARE_string(gce_gced_port);

volatile static bool s_is_stop = false;

void StopSigHandler(int /*sig*/) {
    s_is_stop = true;        
}

int main(int argc, char* argv[]) {
    // GcedImpl* impl = new GcedImpl();
    ::google::ParseCommandLineFlags(&argc, &argv, true);

    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    baidu::galaxy::GcedImpl* gced_service =
        new baidu::galaxy::GcedImpl();
    // if (!gced_service->Init()) {
    //     LOG(WARNING, "Initd Service Init failed"); 
    //     return EXIT_FAILURE;
    // }

    if (!rpc_server.RegisterService(gced_service)) {
        LOG(WARNING, "Rpc Server Regist Service failed");
        return EXIT_FAILURE;
    }

    std::string server_host = std::string("0.0.0.0:") 
        + FLAGS_gce_gced_port;

    if (!rpc_server.Start(server_host)) {
        LOG(WARNING, "Rpc Server Start failed");
        return EXIT_FAILURE;
    } 
    signal(SIGTERM, StopSigHandler);
    signal(SIGINT, StopSigHandler);
    while (!s_is_stop) {
        sleep(5); 
    }
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
