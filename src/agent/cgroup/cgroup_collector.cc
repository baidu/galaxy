// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup_collector.h"
#include "cgroup.h"
#include "timer.h"
#include <assert.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

CgroupCollector::CgroupCollector(boost::shared_ptr<Cgroup> cgroup) :
    enabled_(false),
    cycle_(-1),
    cgroup_(cgroup),
    metrix_(new Metrix())
{
    assert(NULL != cgroup);
}

CgroupCollector::~CgroupCollector()
{
    enabled_ = false;
}

baidu::galaxy::util::ErrorCode CgroupCollector::Collect()
{
    Metrix metrix1;
    Metrix metrix2;
    int64_t t1 = 0L;
    int64_t t2 = 0L;
    baidu::galaxy::util::ErrorCode ec = cgroup_->Statistics(metrix1);

    if (ec.Code() != 0) {
        t1 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    usleep(1000000);
    ec = cgroup_->Statistics(metrix2);

    if (ec.Code() != 0) {
        t2 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    // cal cpu
    boost::mutex::scoped_lock lock(mutex_);
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
}
}
}
