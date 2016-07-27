// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "cpu_subsystem.h"
#include "protocol/galaxy.pb.h"
#include "agent/util/path_tree.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

#include <assert.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

CpuSubsystem::CpuSubsystem()
{
}

CpuSubsystem::~CpuSubsystem()
{
}

std::string CpuSubsystem::Name()
{
    return "cpu";
}

baidu::galaxy::util::ErrorCode CpuSubsystem::Construct()
{
    assert(NULL != cgroup_);
    assert(!container_id_.empty());
    const std::string cpu_path = this->Path();
    boost::filesystem::path path(cpu_path);
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path) 
                && !baidu::galaxy::file::create_directories(path, ec)) {
        return ERRORCODE(-1, "failed in creating path %s: %s",
                path.string().c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path cpu_share(cpu_path);
    cpu_share.append("cpu.shares");
    boost::filesystem::path cpu_cfs(cpu_path);
    cpu_cfs.append("cpu.cfs_quota_us");
    baidu::galaxy::util::ErrorCode err;

    if (cgroup_->cpu().excess()) {
        err = baidu::galaxy::cgroup::Attach(cpu_share.c_str(),
                MilliCoreToShare(cgroup_->cpu().milli_core()),
                false);

        if (err.Code() != 0) {
            return ERRORCODE(-1, "%s", err.Message().c_str());
        }
    } else {
        boost::filesystem::path cpu_cfs(cpu_path);
        cpu_cfs.append("cpu.cfs_quota_us");
        err = baidu::galaxy::cgroup::Attach(cpu_cfs.c_str(),
                MilliCoreToCfs(cgroup_->cpu().milli_core()),
                false);

        if (0 != err.Code()) {
            return ERRORCODE(-1, "%s", err.Message().c_str());
        }
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode CpuSubsystem::Collect(std::map<std::string, AutoValue>& stat)
{
    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> CpuSubsystem::Clone()
{
    boost::shared_ptr<Subsystem> ret(new CpuSubsystem());
    return ret;
}

} //namespace cgroup
} //namespace galaxy
} //namespace baidu
