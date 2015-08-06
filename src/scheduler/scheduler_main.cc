// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <signal.h>
#include <unistd.h>
#include <gflags/gflags.h>
//#include "logging.h"
#include "scheduler/scheduler_io.h"
DECLARE_string(master_host);
DECLARE_string(master_port);

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/) {
    s_quit = true;
}

int main(int argc, char* argv[]) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    //LOG(INFO, "start scheduler....")
    ::baidu::galaxy::SchedulerIO io;
    io.Init();
    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);
    while (!s_quit) {
        //LOG(INFO, "scheduler loop");
        io.Loop();
        sleep(5);
    }
    //LOG(INFO, "scheduler stopped");
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
