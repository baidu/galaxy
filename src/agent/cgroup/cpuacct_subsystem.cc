// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpuacct_subsystem.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "util/input_stream_file.h"
#include "boost/algorithm/string/predicate.hpp"

#include "protocol/agent.pb.h"

namespace baidu {
namespace galaxy {
namespace cgroup {


CpuacctSubsystem::CpuacctSubsystem()
{
}

CpuacctSubsystem::~CpuacctSubsystem()
{
}

std::string CpuacctSubsystem::Name()
{
    return "cpuacct";
}

baidu::galaxy::util::ErrorCode CpuacctSubsystem::Construct()
{
    assert(NULL != cgroup_.get());
    assert(!container_id_.empty());
    boost::filesystem::path path(this->Path());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec) 
                && !baidu::galaxy::file::create_directories(path, ec)) {
        return ERRORCODE(-1, "create file %s failed: ",
                ec.message().c_str());
    }

    return ERRORCODE_OK;
}

boost::shared_ptr<Subsystem> CpuacctSubsystem::Clone()
{
    boost::shared_ptr<Subsystem> ret(new CpuacctSubsystem());
    return ret;
}

baidu::galaxy::util::ErrorCode CpuacctSubsystem::Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix.get());
    int64_t container_cpu_time = 0L;
    int64_t system_cpu_time = 0L;
    baidu::galaxy::util::ErrorCode ec = CgroupCpuTime(container_cpu_time);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, "collect cgroup time failed: %s", ec.Message().c_str());
    }
    
    ec = SystemCpuTime(system_cpu_time);
    if (ec.Code() != 0) {
        return ERRORCODE(-1, "collect system time failed: %s", ec.Message().c_str());
    }
    
    metrix->set_container_cpu_time(container_cpu_time);
    metrix->set_system_cpu_time(system_cpu_time);
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode CpuacctSubsystem::CgroupCpuTime(int64_t& cpu_time) {
    cpu_time = 0;
    boost::filesystem::path path(Path());
    path.append("cpuacct.stat");
    baidu::galaxy::file::InputStreamFile in(path.string());
    if (!in.IsOpen()) {
        baidu::galaxy::util::ErrorCode ec = in.GetLastError();
        return ERRORCODE(-1, "open %s failed: %s", path.string().c_str(), ec.Message().c_str());
    }
    
    std::string line;
    bool has_data = false;
    while (!in.Eof()) {
        baidu::galaxy::util::ErrorCode ec = in.ReadLine(line);
        if (ec.Code() != 0) {
            return ERRORCODE(-1, "read (%s) failed: %s", 
                        path.string().c_str(), 
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
        return ERRORCODE_OK;
    }
    
    return ERRORCODE(-1, "unkown error");
}

baidu::galaxy::util::ErrorCode CpuacctSubsystem::SystemCpuTime(int64_t& cpu_time) {
    cpu_time = 0;
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

     return ERRORCODE_OK;
}
    
}
}
}
