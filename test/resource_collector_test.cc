// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/resource_collector_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common/logging.h"

int FLAGS_resource_collector_engine_interval = 4;
std::string FLAGS_cgroup_root = "/cgroups/";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "./resource_collector_test group_path\n"); 
        return EXIT_FAILURE;
    }

    //galaxy::CGroupResourceCollector* collector = new galaxy::CGroupResourceCollector(argv[1]);
    galaxy::ProcResourceCollector* collector = new galaxy::ProcResourceCollector(atoi(argv[1]));
    galaxy::ResourceCollectorEngine engine(1000);
    engine.Start();
    engine.AddCollector(collector);

    while (1) {
        sleep(2);
        printf("cpu usage %f\n", collector->GetCpuUsage());
        printf("memory usage %ld\n", collector->GetMemoryUsage());
    }
    return EXIT_SUCCESS;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
