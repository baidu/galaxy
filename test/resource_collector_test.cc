// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/resource_collector_engine.h"
#include "agent/resource_collector.h"
#include <stdlib.h>
#include <unistd.h>
#include "common/logging.h"
#include "common/timer.h"

class TestResourceCollector : public galaxy::ResourceCollector {
public:
    TestResourceCollector() {
        cur_collect_time_ = common::timer::get_micros() / 1000;
        prev_collect_time_ = cur_collect_time_;
    }
    int GetCollectTime() {
        printf("cur[%d] prev[%d]\n", cur_collect_time_, prev_collect_time_);
        return cur_collect_time_ - prev_collect_time_;
    }
private:
    virtual bool CollectStatistics() {
        prev_collect_time_ = cur_collect_time_;
        cur_collect_time_ = common::timer::get_micros() / 1000; 
        return true;
    }
    int cur_collect_time_;
    int prev_collect_time_;
};

int FLAGS_resource_collector_engine_interval = 100;
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
    TestResourceCollector* default_interval_collector = new TestResourceCollector();
    engine.AddCollector(default_interval_collector);

    TestResourceCollector* different_interval_collector = new TestResourceCollector();
    engine.AddCollector(different_interval_collector, 5000);

    while (1) {
        sleep(1);
        //printf("cpu usage %f\n", collector->GetCpuUsage());
        //printf("memory usage %ld\n", collector->GetMemoryUsage());
        printf("default collect interval %d\n", default_interval_collector->GetCollectTime());
        printf("different collect interval %d\n", different_interval_collector->GetCollectTime());
    }
    return EXIT_SUCCESS;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
