// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "memory_subsystem.h"
#include "agent/util/path_tree.h"
#include "protocol/galaxy.pb.h"
#include "protocol/agent.pb.h"
#include "util/input_stream_file.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

namespace baidu {
namespace galaxy {
namespace cgroup {

MemorySubsystem::MemorySubsystem() {
}

MemorySubsystem::~MemorySubsystem() {
}

std::string MemorySubsystem::Name() {
    return "memory";
}


baidu::galaxy::util::ErrorCode MemorySubsystem::Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix.get());
    boost::filesystem::path path(Path());
    path.append("memory.usage_in_bytes");
    baidu::galaxy::file::InputStreamFile in(path.string());

    if (!in.IsOpen()) {
        baidu::galaxy::util::ErrorCode ec = in.GetLastError();
        return ERRORCODE(-1, "open file(%s) failed: %s",
                path.string().c_str(),
                ec.Message().c_str());
    }

    std::string data;
    baidu::galaxy::util::ErrorCode ec = in.ReadLine(data);

    if (ec.Code() != 0) {
        return ERRORCODE(-1, "read failed: %s", ec.Message().c_str());
    }

    metrix->set_memory_used_in_byte(::atol(data.c_str()));
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode MemorySubsystem::Construct() {
    assert(!this->container_id_.empty());
    assert(NULL != this->cgroup_.get());
    std::string path = this->Path();
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec)
            && !baidu::galaxy::file::create_directories(path, ec)) {
        return ERRORCODE(-1, "failed in creating file %s: %s",
                path.c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path memory_limit_path = path;
    memory_limit_path.append("memory.limit_in_bytes");
    baidu::galaxy::util::ErrorCode err;
    err = baidu::galaxy::cgroup::Attach(memory_limit_path.c_str(),
            this->cgroup_->memory().size(), false);

    if (0 != err.Code()) {
        return ERRORCODE(-1,
                "attch memory.limit_in_bytes failed: %s",
                err.Message().c_str());
    }

    int64_t excess_mode = this->cgroup_->memory().excess() ? 1L : 0L;
    boost::filesystem::path excess_mode_path = path;
    excess_mode_path.append("memory.excess_mode");
    err = baidu::galaxy::cgroup::Attach(excess_mode_path.c_str(),
            excess_mode,
            false);

    if (0 != err.Code()) {
        return ERRORCODE(-1,
                "attch memory.excess_mode failed: %s",
                err.Message().c_str());
    }

    boost::filesystem::path kill_mode_path = path;
    kill_mode_path.append("memory.kill_mode");
    err = baidu::galaxy::cgroup::Attach(kill_mode_path.c_str(),
            0L,
            false);

    if (0 != err.Code()) {
        return ERRORCODE(-1,
                "attch memory.kill_mode failed: %s",
                err.Message().c_str());
    }

    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> MemorySubsystem::Clone() {
    boost::shared_ptr<Subsystem> ret(new MemorySubsystem());
    return ret;
}



} //namespace cgroup
} //namespace galaxy
} //namespace baidu
