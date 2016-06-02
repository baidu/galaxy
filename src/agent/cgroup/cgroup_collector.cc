// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup_collector.h"
#include "protocol/agent.pb.h"
#include "cgroup.h"
#include "timer.h"
#include <assert.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

CgroupCollector::CgroupCollector(Cgroup* cgroup) :
    enabled_(false),
    cycle_(-1),
    cgroup_(cgroup),
    metrix_(new baidu::galaxy::proto::CgroupMetrix()),
    last_time_(0L)
{
    assert(NULL != cgroup);
}

CgroupCollector::~CgroupCollector()
{
    enabled_ = false;
}

baidu::galaxy::util::ErrorCode CgroupCollector::Collect()
{
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix1;
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix2;
    int64_t t1 = 0L;
    int64_t t2 = 0L;

    baidu::galaxy::util::ErrorCode ec = cgroup_->Collect(metrix1);

    if (ec.Code() == 0) {
        t1 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    usleep(1000);
    ec = cgroup_->Collect(metrix2);

    if (ec.Code() == 0) {
        t2 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    // cal cpu
    boost::mutex::scoped_lock lock(mutex_);
    metrix_.reset(new baidu::galaxy::proto::CgroupMetrix());
    last_time_ = baidu::common::timer::get_micros();

    return ERRORCODE_OK;
}

void CgroupCollector::Enable(bool enabled)
{
    boost::mutex::scoped_lock lock(mutex_);
    enabled_ = enabled;
}

bool CgroupCollector::Enabled()
{
    boost::mutex::scoped_lock lock(mutex_);
    return enabled_;
}

bool CgroupCollector::Equal(const Collector& c)
{
    int64_t addr1 = (int64_t)this;
    int64_t addr2 = (int64_t)&c;
    return addr1 == addr2;
}

void CgroupCollector::SetCycle(int cycle)
{
    boost::mutex::scoped_lock lock(mutex_);
    cycle_ = cycle;
}

int CgroupCollector::Cycle()
{
    boost::mutex::scoped_lock lock(mutex_);
    return cycle_;
}

std::string CgroupCollector::Name() const
{
    //boost::mutex::scoped_lock lock(mutex_);
    return name_;
}

void CgroupCollector::SetName(const std::string& name)
{
    boost::mutex::scoped_lock lock(mutex_);
    name_ = name;
}

boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> CgroupCollector::Statistics() {
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> ret;
    return ret;
}

}
}
}
