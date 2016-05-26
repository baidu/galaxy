// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <assert.h>
#include <stdint.h>
#include <gflags/gflags.h>
#include <iostream>

DECLARE_int64(cpu_resource);
namespace baidu {
namespace galaxy {
namespace resource {

class CpuResource {
public:
    CpuResource();
    ~CpuResource();

    int Load() {
        total_ = FLAGS_cpu_resource;
        assert(FLAGS_cpu_resource >= 0);
        return 0;
    }

    int Allocate(uint64_t milli_cores) {
        if (assigned_ + milli_cores > total_) {
            return -1;
        }

        assigned_ += milli_cores;
        return 0;
    }

    int Release(uint64_t milli_cores) {
        assigned_ -= milli_cores;
        assert(assigned_ >= 0);
        return 0;
    }

    void Resource(uint64_t& total, uint64_t& assigned) {
        total = total_;
        assigned = assigned_;
    }

private:
    uint64_t total_;
    uint64_t assigned_;
};
}
}
}
