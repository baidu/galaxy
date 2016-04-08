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

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/){
    s_quit = true;
}

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    baidu::galaxy::SetupLog("appworker");
    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    LOG(INFO) << "resman started.";
    while (!s_quit) {
        sleep(1);
    }
    return 0;
}

