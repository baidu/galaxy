// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "netcls_subsystem.h"
#include "protocol/galaxy.pb.h"

#include "boost/filesystem/path.hpp"
#include "boost/thread/thread_time.hpp"
#include "boost/filesystem/operations.hpp"

namespace baidu {
namespace galaxy {
namespace cgroup {

NetclsSubsystem::NetclsSubsystem()
{
}

NetclsSubsystem::~NetclsSubsystem() {}

std::string NetclsSubsystem::Name()
{
    return "net_cls";
}

baidu::galaxy::util::ErrorCode NetclsSubsystem::Construct()
{
    assert(NULL != cgroup_.get());
    assert(!container_id_.empty());
    boost::filesystem::path path(this->Path());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
        return ERRORCODE(-1, "create file %s failed: ",
                ec.message().c_str());
    }

    path.append("net_cls.bind_port_range");
    int min_port = 65535;
    int max_port = 65535;
    PortRange(&min_port, &max_port);
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "%d-%d", min_port, max_port);
    return baidu::galaxy::cgroup::Attach(path.string(), buf);
}

void NetclsSubsystem::PortRange(int* minp, int* maxp)
{
    int min_port = 65535;
    int max_port = 0;

    if (cgroup_->ports_size() == 0) {
        min_port = 65535;
        max_port = 65535;
    }

    for (int i = 0; i < cgroup_->ports_size(); i++) {
        int port = atoi(cgroup_->ports(i).real_port().c_str());

        if (min_port > port) {
            min_port = port;
        }

        if (max_port < port) {
            max_port = port;
        }
    }

    if (NULL != minp) {
        *minp = min_port;
    }

    if (NULL != maxp) {
        *maxp = max_port;
    }
}

boost::shared_ptr<Subsystem> NetclsSubsystem::Clone()
{
    boost::shared_ptr<Subsystem> ret(new NetclsSubsystem());
    return ret;
}

baidu::galaxy::util::ErrorCode NetclsSubsystem::Collect(std::map<std::string, AutoValue>& stat)
{
    return ERRORCODE_OK;
}

}
}
}
