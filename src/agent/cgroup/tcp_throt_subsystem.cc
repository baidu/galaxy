/***************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file src/agent/cgroup/tcp_throt_subsystem.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/04/25 15:13:03
 * @brief
 *
 **/

#include "tcp_throt_subsystem.h"
#include "protocol/galaxy.pb.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>

namespace baidu {
namespace galaxy {
namespace cgroup {

TcpThrotSubsystem::TcpThrotSubsystem() {}
TcpThrotSubsystem::~TcpThrotSubsystem() {}

std::string TcpThrotSubsystem::Name()
{
    return "tcp_throt";
}

baidu::galaxy::util::ErrorCode TcpThrotSubsystem::Construct()
{
    assert(NULL != cgroup_);
    assert(!container_id_.empty());
    boost::system::error_code ec;
    boost::filesystem::path path(this->Path());

    if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
        return ERRORCODE(-1,
                "file (%s) donot exist: %s",
                ec.message().c_str());
    }

    // tcp_throt.recv_bps_quota & tcp_throt.recv_bps_limit
    boost::filesystem::path recv_bps_quota = path;
    recv_bps_quota.append("tcp_throt.recv_bps_quota");
    baidu::galaxy::util::ErrorCode err = baidu::galaxy::cgroup::Attach(recv_bps_quota.string(),
                                         cgroup_->tcp_throt().recv_bps_quota(),
                                         false);

    if (0 != err.Code()) {
        return ERRORCODE(-1,
                "attach recv_bps_quota failed: %s",
                err.Message().c_str());
    }

    uint64_t limit = 0L;
    std::stringstream ss;

    if (cgroup_->tcp_throt().recv_bps_excess()) {
        limit = (uint64_t)(-1L);
    }

    ss << limit;
    boost::filesystem::path recv_bps_limit = path;
    recv_bps_limit.append("tcp_throt.recv_bps_limit");
    err = baidu::galaxy::cgroup::Attach(recv_bps_limit.string(), ss.str(), false);

    if (0 != err.Code()) {
        return ERRORCODE(-1, "attach recv_bps_limit failed: %s",
                err.Message().c_str());
    }

    // tcp_throt.send_bps_quota & tcp_throt.send_bps_limit
    boost::filesystem::path send_bps_quota = path;
    send_bps_quota.append("tcp_throt.send_bps_quota");
    err = baidu::galaxy::cgroup::Attach(send_bps_quota.string(), cgroup_->tcp_throt().send_bps_quota(), false);

    if (0 != err.Code()) {
        return ERRORCODE(-1, "attch send_bps_quota failed: %s",
                err.Message().c_str());
    }

    limit = 0L;

    if (cgroup_->tcp_throt().send_bps_excess()) {
        limit = (uint64_t)(-1L);
    }

    ss.str("");
    ss << limit;
    boost::filesystem::path send_bps_limit = path;
    send_bps_limit.append("tcp_throt.send_bps_limit");
    err = baidu::galaxy::cgroup::Attach(send_bps_limit.string(), ss.str(), false);

    if (0 != err.Code()) {
        return ERRORCODE(-1, "attach send_bps_limit filed: %s",
                err.Message().c_str()
                                                );
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode TcpThrotSubsystem::Collect(std::map<std::string, AutoValue>& stat)
{
    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> TcpThrotSubsystem::Clone()
{
    boost::shared_ptr<Subsystem> ret(new TcpThrotSubsystem());
    return ret;
}

}
}
}

