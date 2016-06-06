// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"

namespace baidu {
namespace galaxy {
namespace collector {

class Collector {
public:
    Collector() {}
    virtual ~Collector() {}

    virtual baidu::galaxy::util::ErrorCode Collect() = 0;
    virtual void Enable(bool enable) = 0;
    virtual bool Enabled() = 0;
    virtual bool Equal(const Collector*) = 0;
    virtual int Cycle() = 0;  // unit second
    virtual std::string Name() const = 0;
};
}
}
}
