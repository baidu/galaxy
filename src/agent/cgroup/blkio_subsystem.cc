// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "blkio_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/kdev_t.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

BlkioSubsystem::BlkioSubsystem()
{
}

BlkioSubsystem::~BlkioSubsystem()
{
}

std::string BlkioSubsystem::Name()
{
    return "blkio";
}

baidu::galaxy::util::ErrorCode BlkioSubsystem::Construct()
{
    assert(NULL != cgroup_);
    assert(!container_id_.empty());
    boost::system::error_code ec;
    boost::filesystem::path path(this->Path());

    if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
        return ERRORCODE(-1, "crate file %s failed:",
                path.string().c_str(),
                ec.message().c_str());
    }

    // tcp_throt.recv_bps_quota & tcp_throt.recv_bps_limit
    boost::filesystem::path blkio_weight = path;
    blkio_weight.append("blkio.weight");
    baidu::galaxy::util::ErrorCode err = baidu::galaxy::cgroup::Attach(blkio_weight.string(),
                                         (int64_t)cgroup_->blkio().weight());

    if (0 != err.Code()) {
        return ERRORCODE(-1, "attch weight failed: %s",
                err.Message().c_str());
    }

    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> BlkioSubsystem::Clone()
{
    boost::shared_ptr<Subsystem> ret(new BlkioSubsystem());
    return ret;
}

baidu::galaxy::util::ErrorCode BlkioSubsystem::Collect(std::map<std::string, AutoValue>& stat)
{
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode BlkioSubsystem::GetDeviceNum(const std::string& path, int& major, int& minor)
{
    struct stat st;

    if (0 != ::stat(path.c_str(), &st)) {
        return PERRORCODE(-1, errno, "stat faile failed");
    }

    major = MAJOR(st.st_dev);
    minor = MINOR(st.st_dev);
    return ERRORCODE_OK;
}

}
}
}
