// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com
// Date  : 2015-04-15
#include "master/scheduler.h"

#include <iostream>
#include <vector>

int main(){
    galaxy::SqliteScheduler scheduler;
    scheduler.Init();
    //load1
    galaxy::AgentLoad load1;
    load1.agent_id= 1;
    load1.mem_total= 137438953472;
    load1.mem_left= 68719476736;
    load1.cpu_total= 24.0;
    load1.cpu_left= 12.0;
    load1.task_count = 5;
    load1.addr="9527";
    std::cout << "load1 " << scheduler.CalcLoad(load1)<<std::endl;
    //load2
    galaxy::AgentLoad load2;
    load2.agent_id= 2;
    load2.mem_left= 85899345920;
    load2.mem_total= 137438953472;
    load2.cpu_total= 24.0;
    load2.cpu_left= 15.0;
    load2.task_count = 3;
    load2.addr="9527";
    galaxy::AgentLoad load3;
    load3.agent_id= 4;
    load3.mem_left= 85899345920;
    load3.mem_total= 137438953472;
    load3.cpu_total= 24.0;
    load3.cpu_left= 15.0;
    load3.task_count = 2;
    load3.addr="9527:asdasd";


    std::cout << "load2 " << scheduler.CalcLoad(load2)<<std::endl;
    int ret = scheduler.Save(load1);
    assert(ret==0);
    ret = scheduler.Save(load2);
    assert(ret==0);
    ret = scheduler.Save(load2);
    assert(ret==0);
    ret = scheduler.Save(load3);
    assert(ret==0);
    std::vector<int32_t> ports;
    int64_t mem_require = 1024*1024*1024*70;
    double cpu_require = 10.0;
    galaxy::AgentLoad s_load;
    ret =  scheduler.Schedule(mem_require, cpu_require ,ports,&s_load);
    assert(s_load.agent_id == 4);

    ret = scheduler.Delete(4);
    assert(ret == 0);
    ret =  scheduler.Schedule(mem_require, cpu_require ,ports,&s_load);
    assert(s_load.agent_id == 2);
    ret = scheduler.Delete(5);
    assert(ret == 0);
    return 0;
}



