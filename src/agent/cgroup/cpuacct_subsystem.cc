// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpuacct_subsystem.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

namespace baidu {
namespace galaxy {
namespace cgroup {
CpuacctSubsystem::CpuacctSubsystem() {
}

CpuacctSubsystem::~CpuacctSubsystem() {
}

std::string CpuacctSubsystem::Name() {
    return "cpuacct";
}

int CpuacctSubsystem::Construct() {
    assert(NULL != cgroup_.get());
    assert(!container_id_.empty());
    boost::filesystem::path path(this->Path());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
        return -1;
    }

    return 0;
}

boost::shared_ptr<Subsystem> CpuacctSubsystem::Clone() {
    boost::shared_ptr<Subsystem> ret(new CpuacctSubsystem());
    return ret;
}

baidu::galaxy::util::ErrorCode CpuacctSubsystem::Collect(std::map<std::string, AutoValue>& stat) {
    return ERRORCODE_OK;
}

}
}
}
