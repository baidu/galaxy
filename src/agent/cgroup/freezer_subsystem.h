// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "subsystem.h"

namespace baidu {
namespace galaxy {
namespace cgroup {

class FreezerSubsystem : public Subsystem {
public:
    FreezerSubsystem();
    ~FreezerSubsystem();

    baidu::galaxy::util::ErrorCode Freeze();
    baidu::galaxy::util::ErrorCode Thaw();
    bool Empty();

    baidu::galaxy::util::ErrorCode Collect(std::map<std::string, AutoValue>& stat);
    boost::shared_ptr<Subsystem> Clone();
    std::string Name();
    baidu::galaxy::util::ErrorCode Construct();

};
}
}
}
