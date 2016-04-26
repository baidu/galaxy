// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <algorithm>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sofa/pbrpc/pbrpc.h>

#include "appworker_impl.h"
#include "src/utils/setting_utils.h"

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

    baidu::galaxy::AppWorkerImpl* appworker_impl = new baidu::galaxy::AppWorkerImpl();
    int ret = appworker_impl->Init();

    LOG(INFO) << "appworker started. " << ret;
    while (!s_quit) {
        sleep(1);
    }

    return 0;
}
