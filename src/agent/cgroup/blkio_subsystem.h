// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "subsystem.h"

namespace baidu {
namespace galaxy {
namespace cgroup {

class BlkioSubsystem : public Subsystem {
public:
    BlkioSubsystem();
    ~BlkioSubsystem();
    std::string Name();
    int Construct();
    boost::shared_ptr<Subsystem> Clone();
    baidu::galaxy::util::ErrorCode Collect(std::map<std::string, AutoValue>& stat);

private:
    int GetDeviceNum(const std::string& path, int& major, int& minor);
};
}
}
}
