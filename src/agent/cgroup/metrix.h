/***************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file src/agent/cgroup/metrix.h
 * @author haolifei(com@baidu.com)
 * @date 2016/05/28 09:39:44
 * @brief
 *
 **/
#pragma once

namespace baidu {
namespace galaxy {
namespace cgroup {

class Metrix {
public:
    Metrix() :
        timestamp(-1),
        cpu_systime(0L),
        cpu_usertime(0L),
        cpu_used_in_millicore(0),
        memory_used_in_byte(0) {
    }
    int64_t timestamp;
    int64_t cpu_systime;
    int64_t cpu_usertime;
    int64_t cpu_used_in_millicore;
    int64_t memory_used_in_byte;
};

}
}
}
