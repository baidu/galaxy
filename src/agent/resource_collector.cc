// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "agent/resource_collector.h"

#include <timer.h>
#include <errno.h>
#include <string.h>
#include <string>
#include "gflags/gflags.h"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "logging.h"

namespace baidu {
namespace galaxy {

static long CPU_CORES = sysconf(_SC_NPROCESSORS_CONF);
static long MEMORY_PAGE_SIZE = sysconf(_SC_PAGESIZE);

static const std::string PROC_MOUNT_PATH = "/proc";

static const size_t PROC_STAT_FILE_SPLIT_SIZE = 44;
static const size_t PROC_STATUS_FILE_SPLIT_SIZE = 3;
static const int MIN_COLLECT_TIME = 2;

// get total cpu usage from /proc/stat
static bool GetGlobalCpuUsage(ResourceStatistics* statistics);

static bool GetProcPidUsage(int pid, ResourceStatistics* statistics);

ProcResourceCollector::ProcResourceCollector(int pid) 
    : pid_(pid),
     global_statistics_prev_(),
     global_statistics_cur_(),
     process_statistics_prev_(),
     process_statistics_cur_(),
     timestamp_prev_(0.0),
     timestamp_cur_(0.0),
     collector_times_(0) {
}

ProcResourceCollector::~ProcResourceCollector() {
}

void ProcResourceCollector::ResetPid(int pid) {
    pid_ = pid;
}

void ProcResourceCollector::Clear() {
    collector_times_ = 0;
}

bool ProcResourceCollector::CollectStatistics() {
    global_statistics_prev_ = global_statistics_cur_;
    process_statistics_prev_ = process_statistics_cur_;
    timestamp_prev_ = timestamp_cur_;
    timestamp_cur_ = ::time(NULL);
    if (!GetGlobalCpuUsage(&global_statistics_cur_)) {
        return false; 
    }

    if (!GetProcPidUsage(pid_, &process_statistics_cur_)) {
        return false;
    }

    collector_times_++;
    return true;
}

double ProcResourceCollector::GetCpuCoresUsage() {
    return GetCpuUsage() * CPU_CORES;
}

long ProcResourceCollector::GetMemoryUsage() {
    return process_statistics_cur_.memory_rss_in_bytes;
}

double ProcResourceCollector::GetCpuUsage() {
    if (collector_times_ < MIN_COLLECT_TIME) {
        LOG(WARNING, "pid %d need try again", pid_); 
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

    long proc_cpu_before = process_statistics_prev_.cpu_user_time 
        + process_statistics_prev_.cpu_system_time;
    long proc_cpu_after = process_statistics_cur_.cpu_user_time
        + process_statistics_cur_.cpu_system_time;

    if (global_cpu_after - global_cpu_before <= 0) {
        return 0.0; 
    }

    double rs = (proc_cpu_after - proc_cpu_before) 
                / static_cast<double>(global_cpu_after 
                                        - global_cpu_before);
    LOG(DEBUG, "process prev:%ld post:%ld global pre:%ld post:%ld rs: %lf",
            proc_cpu_before, 
            proc_cpu_after,
            global_cpu_before,
            global_cpu_after,
            rs);
    return rs;
}

bool GetGlobalCpuUsage(ResourceStatistics* statistics) {
    if (statistics == NULL) {
        return false; 
    }

    statistics->cpu_cores = CPU_CORES;
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
    line = NULL;
    if (item_size != 10) {
        LOG(WARNING, "read from /proc/stat format err"); 
        return false;
    }
    return true;
}

bool GetProcPidUsage(int pid, ResourceStatistics* statistics) {
    std::string path = PROC_MOUNT_PATH + "/" + boost::lexical_cast<std::string>(pid) + "/stat";    
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

    std::vector<std::string> values;
    std::string str_line(line);
    boost::split(values, str_line, boost::is_any_of(" "));
    LOG(DEBUG, "split items %lu", values.size());
    free(line);
    line = NULL;
    if (values.size() < PROC_STAT_FILE_SPLIT_SIZE) {
        LOG(WARNING, "proc[%ld] stat file format err", pid);    
        return false;
    }
    statistics->cpu_user_time = atol(values[13].c_str());
    statistics->cpu_system_time = atol(values[14].c_str());
    statistics->memory_rss_in_bytes = atol(values[23].c_str()) * MEMORY_PAGE_SIZE;
    LOG(DEBUG, "get proc[%ld] user %ld sys %ld mem %ld", 
            pid, statistics->cpu_user_time, 
            statistics->cpu_system_time, 
            statistics->memory_rss_in_bytes);
    return true;
}


}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
