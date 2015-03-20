// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  COMMON_THIS_THREAD_H_
#define  COMMON_THIS_THREAD_H_

#include <pthread.h>
#include <stdint.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>

__thread int THREAD_ID = 0;

namespace common {
    
class ThisThread {
public:
    /// Sleep in ms
    static void Sleep(int64_t time_ms) {
        if (time_ms > 0) {
            timespec ts = {time_ms / 1000, (time_ms % 1000) * 1000000 };
            nanosleep(&ts, &ts);
        }
    }
    /// Get thread id
    static int GetId() {
        if (THREAD_ID == 0) {
            THREAD_ID = syscall(__NR_gettid);
        }
        return THREAD_ID;
    }
    /// Yield cpu
    static void Yield() {
        sched_yield();
    }
    
};

} // namespace common

using common::ThisThread;

#endif  // COMMON_THIS_THREAD_H_
