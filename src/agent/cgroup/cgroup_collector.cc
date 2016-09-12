// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup_collector.h"
#include "protocol/agent.pb.h"
#include "util/input_stream_file.h"
#include "cgroup.h"
#include "timer.h"
#include "boost/algorithm/string/predicate.hpp"
#include <assert.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

const static long CPU_CORES = sysconf(_SC_NPROCESSORS_CONF);
CgroupCollector::CgroupCollector() :
    enabled_(false),
    cycle_(-1),
    metrix_(new baidu::galaxy::proto::CgroupMetrix()),
    last_time_(0L) {
}

CgroupCollector::~CgroupCollector() {
    enabled_ = false;
}

baidu::galaxy::util::ErrorCode CgroupCollector::Collect() {
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix1(new baidu::galaxy::proto::CgroupMetrix);
    int64_t t1 = 0L;
    baidu::galaxy::util::ErrorCode ec = Collect(metrix1);

    if (ec.Code() == 0) {
        t1 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    ::usleep(1000000);
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix2(new baidu::galaxy::proto::CgroupMetrix);
    int64_t t2 = 0L;
    ec = Collect(metrix2);

    if (ec.Code() == 0) {
        t2 = baidu::common::timer::get_micros();
    } else {
        return ERRORCODE(-1, "%s", ec.Message().c_str());
    }

    // cal cpu
    boost::mutex::scoped_lock lock(mutex_);
    metrix_.reset(new baidu::galaxy::proto::CgroupMetrix());
    last_time_ = baidu::common::timer::get_micros();
    metrix_->set_memory_used_in_byte(metrix2->memory_used_in_byte());

    if (metrix1->has_container_cpu_time() && metrix1->has_system_cpu_time()
            && metrix2->has_container_cpu_time() && metrix2->has_system_cpu_time()) {
        double delta1 = (double)(metrix2->container_cpu_time() - metrix1->container_cpu_time());
        double delta2 = (double)(metrix2->system_cpu_time() - metrix1->system_cpu_time());

        if (delta2 > 0.01 && delta1 > 0.01) {
            int64_t mcore = (int64_t)(1000.0 * delta1 / delta2 * CPU_CORES);
            metrix_->set_cpu_used_in_millicore(mcore);
        }
    }

    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode CgroupCollector::Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix);
    baidu::galaxy::util::ErrorCode ec = ContainerCpuStat(metrix);

    if (ec.Code() != 0) {
        return ERRORCODE(-1, ec.Message().c_str());
    }

    ec = SystemCpuStat(metrix);

    if (ec.Code() != 0) {
        return ERRORCODE(-1, ec.Message().c_str());
    }

    ec = MemoryStat(metrix);

    if (ec.Code() != 0) {
        return ERRORCODE(-1, ec.Message().c_str());
    }

    return ERRORCODE_OK;
}

void CgroupCollector::Enable(bool enabled) {
    boost::mutex::scoped_lock lock(mutex_);
    enabled_ = enabled;
}

bool CgroupCollector::Enabled() {
    boost::mutex::scoped_lock lock(mutex_);
    return enabled_;
}

bool CgroupCollector::Equal(const Collector* c) {
    assert(NULL != c);
    int64_t addr1 = (int64_t)this;
    int64_t addr2 = (int64_t)c;
    return addr1 == addr2;
}

void CgroupCollector::SetCycle(int cycle) {
    boost::mutex::scoped_lock lock(mutex_);
    cycle_ = cycle;
}

int CgroupCollector::Cycle() {
    boost::mutex::scoped_lock lock(mutex_);
    return cycle_;
}

std::string CgroupCollector::Name() const {
    //boost::mutex::scoped_lock lock(mutex_);
    return name_;
}

void CgroupCollector::SetName(const std::string& name) {
    boost::mutex::scoped_lock lock(mutex_);
    name_ = name;
}

boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> CgroupCollector::Statistics() {
    boost::mutex::scoped_lock lock(mutex_);
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> ret(new baidu::galaxy::proto::CgroupMetrix);
    ret->CopyFrom(*metrix_);
    return ret;
}

baidu::galaxy::util::ErrorCode CgroupCollector::ContainerCpuStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix.get());

    if (cpuacct_path_.empty()) {
        return ERRORCODE(-1, "empty path");
    }

    baidu::galaxy::file::InputStreamFile in(cpuacct_path_);

    if (!in.IsOpen()) {
        baidu::galaxy::util::ErrorCode ec = in.GetLastError();
        return ERRORCODE(-1, "open %s failed: %s", cpuacct_path_.c_str(), ec.Message().c_str());
    }

    std::string line;
    bool has_data = false;
    int64_t cpu_time = 0;

    while (!in.Eof()) {
        baidu::galaxy::util::ErrorCode ec = in.ReadLine(line);

        if (ec.Code() != 0) {
            return ERRORCODE(-1, "read (%s) failed: %s",
                    cpuacct_path_.c_str(),
                    ec.Message().c_str());
        }

        char type[32];
        long long int t = 0;

        if (2 != sscanf(line.c_str(), "%s %lld", type, &t)) {
            return ERRORCODE(-1, "format error");
        }

        cpu_time += t;
        has_data = true;
    }

    if (has_data) {
        metrix->set_container_cpu_time(cpu_time);
        return ERRORCODE_OK;
    }

    return ERRORCODE(-1, "unkown error");
}


baidu::galaxy::util::ErrorCode CgroupCollector::SystemCpuStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix.get());
    int64_t cpu_time = 0;
    const static std::string  path("/proc/stat");
    baidu::galaxy::file::InputStreamFile in(path);

    if (!in.IsOpen()) {
        baidu::galaxy::util::ErrorCode ec = in.GetLastError();
        return ERRORCODE(-1, "open %s failed: %s", path.c_str(), ec.Message().c_str());
    }

    std::string line;
    bool has_data = false;

    while (!in.Eof()) {
        baidu::galaxy::util::ErrorCode ec = in.ReadLine(line);

        if (ec.Code() != 0) {
            return ERRORCODE(-1, "read (%s) failed: %s", path.c_str(), ec.Message().c_str());
        }

        //cpu  19782368743 69952042 1588879335 90754227704 229233079 0 136086465 0 0
        if (boost::starts_with(line, "cpu ")) {
            char c[16];
            long long int cpu_user_time = 0L;
            long long int cpu_nice_time = 0L;
            long long int cpu_system_time = 0L;
            long long int cpu_idle_time = 0L;
            long long int cpu_iowait_time = 0L;
            long long int cpu_irq_time = 0L;
            long long int cpu_softirq_time = 0L;
            long long int cpu_stealstolen = 0L;
            long long int cpu_guest = 0L;

            if (10 == sscanf(line.c_str(), "%s %lld %lld %lld %lld %lld %lld %lld %lld %lld",
                    c,
                    &cpu_user_time,
                    &cpu_nice_time,
                    &cpu_system_time,
                    &cpu_idle_time,
                    &cpu_iowait_time,
                    &cpu_irq_time,
                    &cpu_softirq_time,
                    &cpu_stealstolen,
                    &cpu_guest)) {
                cpu_time = cpu_user_time
                           + cpu_nice_time
                           + cpu_system_time
                           + cpu_idle_time
                           + cpu_iowait_time
                           + cpu_irq_time
                           + cpu_softirq_time
                           + cpu_stealstolen
                           + cpu_guest;
                has_data = true;
            }

            break;
        }
    }

    if (!has_data) {
        return ERRORCODE(-1, "unkown error");
    }

    metrix->set_system_cpu_time(cpu_time);
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode CgroupCollector::MemoryStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix.get());
    const std::string& path = memory_path_;
    baidu::galaxy::file::InputStreamFile in(path);

    if (!in.IsOpen()) {
        baidu::galaxy::util::ErrorCode ec = in.GetLastError();
        return ERRORCODE(-1, "open file(%s) failed: %s",
                path.c_str(),
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

}
}
}
