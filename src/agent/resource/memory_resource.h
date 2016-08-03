// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <assert.h>
#include <stdint.h>
#include <gflags/gflags.h>

DECLARE_int64(memory_resource);

namespace baidu {
namespace galaxy {
namespace resource {

class MemoryResource {
public:
    MemoryResource() :
        total_(0),
        assigned_(0) {
    }

    int Load() {
        total_ = FLAGS_memory_resource;
        assert(FLAGS_memory_resource >= 0);
        return 0;
    }

    int Allocate(uint64_t size) {
        if (assigned_ + size > total_) {
            return -1;
        }

        assigned_ += size;
        return 0;
    }

    int Release(uint64_t size) {
        assert(assigned_ >= size);
        assigned_ -= size;
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
