// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "freezer_subsystem.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <stdio.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

FreezerSubsystem::FreezerSubsystem()
{
}

FreezerSubsystem::~FreezerSubsystem()
{
}

int FreezerSubsystem::Freeze()
{
    boost::filesystem::path path(this->Path());
    path.append("freezer.state");
    return baidu::galaxy::cgroup::Attach(path.string(), "FROZEN", false);
}

int FreezerSubsystem::Thaw()
{
    boost::filesystem::path path(this->Path());
    path.append("freezer.state");
    return baidu::galaxy::cgroup::Attach(path.string(), "THAWED", false);
}

baidu::galaxy::util::ErrorCode FreezerSubsystem::Collect(std::map<std::string, AutoValue>& stat)
{
    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> FreezerSubsystem::Clone()
{
    boost::shared_ptr<FreezerSubsystem> ret(new FreezerSubsystem());
    return ret;
}

std::string FreezerSubsystem::Name()
{
    return "freezer";
}

int FreezerSubsystem::Construct()
{
    boost::filesystem::path path(this->Path());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
        return -1;
    }

    return 0;
}


bool FreezerSubsystem::Empty()
{
    std::vector<int> pids;
    GetProcs(pids);
    return pids.empty();
}

}
}
}
