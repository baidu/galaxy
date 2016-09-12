// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "subsystem.h"


namespace baidu {
namespace galaxy {

namespace {
class CgroupMetrix;
}

namespace cgroup {

class CpuacctSubsystem : public Subsystem {
public:
    CpuacctSubsystem();
    ~CpuacctSubsystem();
    std::string Name();
    baidu::galaxy::util::ErrorCode Construct();
    boost::shared_ptr<Subsystem> Clone();
    baidu::galaxy::util::ErrorCode Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix);

private:
    baidu::galaxy::util::ErrorCode CgroupCpuTime(int64_t& t);
    baidu::galaxy::util::ErrorCode SystemCpuTime(int64_t& t);
};

}
}
}
