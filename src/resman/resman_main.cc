#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <sofa/pbrpc/pbrpc.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "resman_impl.h"
#include "setting_utils.h"

DECLARE_string(resman_port);

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/){
    s_quit = true;
}

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    baidu::galaxy::SetupLog("resman");
    baidu::galaxy::ResManImpl * resman = new baidu::galaxy::ResManImpl();
    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);
    if (!rpc_server.RegisterService(static_cast<baidu::galaxy::ResMan*>(resman))) {
        LOG(WARNING) << "failed to register resman service";
        exit(-1);
    }

    std::string endpoint = "0.0.0.0:" + FLAGS_resman_port;
    if (!rpc_server.Start(endpoint)) {
        LOG(WARNING) << "failed to start server on %s", endpoint.c_str();
        exit(-2);
    }
    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    LOG(INFO) << "resman started.";
    while (!s_quit) {
        sleep(1);
    }
    return 0;
}


