// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <signal.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

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
    google::SetStderrLogging(google::GLOG_INFO);

    signal(SIGTERM, SignalIntHandler);

    baidu::galaxy::AppWorkerImpl* appworker_impl = new baidu::galaxy::AppWorkerImpl();
    appworker_impl->Init();

    while (true) {
        if (s_quit) {
            appworker_impl->Quit();
        }
        sleep(1);
    }

    return 0;
}
