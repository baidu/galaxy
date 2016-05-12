// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include <algorithm>
#include <vector>

#include <sofa/pbrpc/pbrpc.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "setting_utils.h"
#include "appmaster_impl.h"

DECLARE_string(appmaster_endpoint);

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/){
    s_quit = true;
}

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    baidu::galaxy::SetupLog("appmaster");
    baidu::galaxy::AppMasterImpl * appmaster = new baidu::galaxy::AppMasterImpl();
    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);
    if (!rpc_server.RegisterService(static_cast<baidu::galaxy::proto::AppMaster*>(appmaster))) {
        LOG(WARNING) << "failed to register appmaster service";
        exit(-1);
    }

    if (!rpc_server.Start(FLAGS_appmaster_endpoint)) {
        LOG(WARNING)  << "failed to start server on %s", FLAGS_appmaster_endpoint.c_str();
        exit(-2);
    }
    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    LOG(INFO) << "appmaster started.";
    while (!s_quit) {
        sleep(1);
    }
    return 0;
}
