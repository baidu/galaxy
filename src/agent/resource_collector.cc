// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/resource_collector.h"

#include <errno.h>
#include <string.h>
#include <time.h>

#include <string>
#include <boost/lexical_cast.hpp>
#include "common/logging.h"

namespace galaxy {

const std::string CGROUP_MOUNT_PATH = "/cgroups/";
const std::string CPUACT_SUBSYSTEM_PATH = "cpuact";

static long SYS_TIMER_HZ = sysconf(_SC_CLK_TCK);

// unit jiffies
struct ResourceStatistics {
    // cpu
    long cpu_user_time; 
    long cpu_nice_time;
    long cpu_system_time;
    long cpu_idle_time;
    long cpu_iowait_time;
    long cpu_irq_time;
    long cpu_softirq_time;
    long cpu_stealstolen;
    long cpu_guest;

    long cpu_cores;
};

// unit ns
struct CgroupResourceStatistics {
    long cpu_user_time;
    long cpu_system_time;
};

// get cpu usage from /cgroups/cpuacct/xxxxx/cpuacct.stat
bool GetCgroupCpuUsage(
        const std::string& group_path, 
        CgroupResourceStatistics* statistics);

// get total cpu usage from /proc/stat
bool GetGlobalCpuUsage(ResourceStatistics* statistics);

// get limit cores num from /cgroups/cpu/xxxxx/cpu.cfs_quota_us
bool GetCgroupCpuCores(
        const std::string& group_path,
        double* cpu_cores);

class CGroupResourceCollectorImpl {
public:
    CGroupResourceCollectorImpl() 
        : statistics_prev_(), 
          statistics_cur_(),
          timestamp_prev_(0.0),
          timestamp_cur_(0.0),
          mutex_() {
    }

    virtual ~CGroupResourceCollectorImpl() {}

    virtual bool CollectStatistics();

    double GetCpuUsage();

    long GetMemoryUsage();
private:
    // global resource statistics
    ResourceStatistics global_statistics_prev_;
    ResourceStatistics global_statistics_cur_;
    // cgroup resource statistics
    CgroupResourceStatistics cgroup_statistics_prev_;
    CgroupResourceStatistics cgroup_statistics_cur_;

    double timestamp_prev_;
    double timestamp_cur_;
    long collector_times_;
    common::Mutex mutex_;
};

double CGroupResourceCollectorImpl::GetCpuUsage() {
    const int MIN_COLLOECT_TIMES_NEED = 2;
    common::MutexLock lock(&mutex_);
    if (collector_times_ < MIN_COLLOECT_TIMES_NEED) {
        LOG(WARNING, "need try agin"); 
        return 0.0;
    }

    long global_cpu_before = 
        global_statistics_prev_.cpu_user_time
        + global_statistics_prev_.cpu_nice_time
        + global_statistics_prev_.cpu_system_time
        + global_statistics_prev_.cpu_idle_time
        + global_statistics_prev_.cpu_iowait_time
        + global_statistics_prev_.cpu_irq_time
        + global_statistics_prev_.cpu_softirq_time
        + global_statistics_prev_.cpu_stealstolen
        + global_statistics_prev_.cpu_guest;
        
    long global_cpu_after =
        global_statistics_cur_.cpu_user_time
        + global_statistics_cur_.cpu_nice_time
        + global_statistics_cur_.cpu_system_time
        + global_statistics_cur_.cpu_idle_time
        + global_statistics_cur_.cpu_iowait_time
        + global_statistics_cur_.cpu_irq_time
        + global_statistics_cur_.cpu_softirq_time
        + global_statistics_cur_.cpu_stealstolen
        + global_statistics_cur_.cpu_guest;

}

bool CGroupResourceCollectorImpl::CollectStatistics() {
    common::MutexLock lock(&mutex_);
    collector_times_ ++;
    global_statistics_prev_ = global_statistics_cur_;
    cgroup_statistics_prev_ = cgroup_statistics_cur_;
    timestamp_prev_ = timestamp_cur_;

    timestamp_cur_ = time(NULL);

    if (!GetCgroupCpuUsage(&cgroup_statistics_cur_)) {
        return false; 
    }

    if (!GetGlobalCpuUsage(&global_statistics_cur_)) {
        return false; 
    }

    return true;
}

// ----------- CGroupResourceCollector implement -----

bool CGroupResourceCollector::CollectStatistics() {
    return impl_.CollectStatistics();
}

double CGroupResourceCollector::GetCgroupCpuUsage() {
    return impl_.GetCpuUsage();
}

long CGroupResourceCollector::GetMemoryUsage() {
    return impl_.GetMemoryUsage();
}

// -----------  resource collector utils -------------

bool GetCgroupCpuUsage(
        const std::string& group_path, 
        CgroupResourceStatistics* statistics) {
    if (statistics == NULL) {
        return false; 
    }
    std::string path = group_path + "/cpuacct.stat";
    FILE* fin = fopen(path.c_str(), "r");
    if (fin == NULL) {
        LOG(WARNING, "open %s failed", path.c_str());
        return false; 
    }

    ssize_t read;
    size_t len = 0;
    char* line = NULL;

    // read user time
    if ((read = getline(&line, &len, fin)) == -1) {
        LOG(WARNING, "read line failed err[%d: %s]", 
                errno, strerror(errno));
        fclose(fin);
        return false;
    }

    char cpu_tmp_char[20];
    int item_size = sscanf(line, "%s %ld", 
            cpu_tmp_char, &(statistics->cpu_user_time));
    free(line);

    if (item_size != 2) {
        LOG(WARNING, "read from %s format err", path.c_str()); 
        fclose(fin);
        return false;
    }

    // read sys time
    if ((read = getline(&line, &len, fin)) == -1) {
        LOG(WARNING, "read line failed err[%d: %s]", 
                errno, strerror(errno));  
        fclose(fin);
        return false;
    }

    item_size = sscan(line, "%s %ld",
            cpu_tmp_char, &(statistics->cpu_system_time));
    free(line);

    if (item_size != 2) {
        LOG(WARNING, "read from %s format err", path.c_str()); 
        fclose(fin);
        return false;
    }

    fclose(fin);
    return true;
}

bool GetGlobalCpuUsage(ResourceStatistics* statistics) {
    if (statistics == NULL) {
        return false; 
    }
    std::string path = "/proc/stat";
    FILE* fin = fopen(path.c_str(), "r");
    if (fin == NULL) {
        LOG(WARNING, "open %s failed", path.c_str());
        return false; 
    }

    ssize_t read;
    size_t len = 0;
    char* line = NULL;
    if ((read = getline(&line, &len, fin)) == -1) {
        LOG(WARNING, "read line failed err[%d: %s]", 
                errno, strerror(errno)); 
        fclose(fin);
        return false;
    }
    fclose(fin);

    char cpu[5];
    int item_size = sscanf(line, 
                           "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld", 
                           cpu,
                           &(statistics->cpu_user_time),
                           &(statistics->cpu_nice_time),
                           &(statistics->cpu_system_time),
                           &(statistics->cpu_idle_time),
                           &(statistics->cpu_iowait_time),
                           &(statistics->cpu_irq_time),
                           &(statistics->cpu_softirq_time),
                           &(statistics->cpu_stealstolen),
                           &(statistics->cpu_guest)); 

    free(line);
    if (item_size != 10) {
        LOG(WARNING, "read from /proc/stat format err"); 
        return false;
    }
    return true;
}


}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
