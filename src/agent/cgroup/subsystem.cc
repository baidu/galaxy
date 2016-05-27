// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "subsystem.h"
#include "protocol/galaxy.pb.h"
#include "gflags/gflags.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast/lexical_cast_old.hpp>
#include <sys/types.h>
#include <signal.h>

#include <stdio.h>

DECLARE_string(cgroup_root_path);

namespace baidu {
namespace galaxy {
namespace cgroup {

std::string Subsystem::RootPath(const std::string& name)
{
    boost::filesystem::path path(FLAGS_cgroup_root_path);
    path.append(name);
    path.append("galaxy");
    return path.string();
}

baidu::galaxy::util::ErrorCode Subsystem::Destroy()
{

    boost::filesystem::path path(this->Path());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec)) {
        return ERRORCODE_OK;
    }

    std::vector<int> pids;
    this->GetProcs(pids);
    for (size_t i = 0; i < pids.size(); i++) {
        ::kill(pids[i], SIGKILL);
    }

    boost::filesystem::remove_all(path, ec);

    if (ec.value() != 0) {
        return ERRORCODE(-1, "failed in removing %s: %s",
                path.string().c_str(),
                ec.message().c_str());
    }

    if (boost::filesystem::exists(path, ec)) {
        return ERRORCODE(-1, "%s exist still after removing");
    }
    return ERRORCODE_OK;
}

std::string Subsystem::Path()
{
    std::string id = container_id_ + "_" + cgroup_->id();
    boost::filesystem::path path(Subsystem::RootPath(this->Name()));
    path.append(id);
    return path.string();
}

baidu::galaxy::util::ErrorCode Subsystem::Attach(pid_t pid)
{
    boost::filesystem::path proc_path(Path());
    proc_path.append("tasks");
    boost::system::error_code ec;

    if (!boost::filesystem::exists(proc_path, ec)) {
        return ERRORCODE(-1, "no such file %s: ",
                    proc_path.string().c_str(),
                    ec.message().c_str());
    }

    return baidu::galaxy::cgroup::Attach(proc_path.c_str(), int64_t(pid), true);
}

baidu::galaxy::util::ErrorCode Subsystem::GetProcs(std::vector<int>& pids)
{
    boost::filesystem::path proc_path(Path());
    proc_path.append("cgroup.procs");
    boost::system::error_code ec;

    if (!boost::filesystem::exists(proc_path, ec)) {
        return ERRORCODE(-1, "no such file %s: %s",
                    proc_path.string().c_str(),
                    ec.message().c_str());
    }

    FILE* fin = ::fopen(proc_path.string().c_str(), "re");

    if (NULL == fin) {
        return ERRORCODE(-1, 
                    "failed in open file: %s",
                    proc_path.string().c_str());
    }

    std::string value;
    char temp_read_buffer[30];

    while (::feof(fin) == 0) {
        size_t readlen = ::fread((void*) temp_read_buffer,
                         sizeof(char),
                         sizeof temp_read_buffer, fin);

        if (readlen == 0) {
            break;
        }

        value.append(temp_read_buffer, readlen);
    }

    if (::ferror(fin) != 0) {
        ::fclose(fin);
        return ERRORCODE(-1, 
                    "close file faile: %s",
                    proc_path.string().c_str());
    }
    ::fclose(fin);
    boost::algorithm::trim(value);
    if (value.empty()) {
        return ERRORCODE_OK;
    }

    std::vector<std::string> str_pids;
    boost::split(str_pids, value, boost::is_any_of("\n"));

    for (size_t i = 0; i < str_pids.size(); i++) {
        pids.push_back(atoi(str_pids[i].c_str()));
    }

    return ERRORCODE_OK;
}


const static int64_t CFS_PERIOD = 100000L; // cpu.cfs_period_us
const static int64_t CFS_SHARE = 1000L; // unit
const static int64_t MILLI_CORE = 1000L; // unit

baidu::galaxy::util::ErrorCode Attach(const std::string& file, int64_t value, bool append)
{
    return Attach(file, boost::lexical_cast<std::string>(value), append);
}

baidu::galaxy::util::ErrorCode Attach(const std::string& file, const std::string& value, bool append)
{
    FILE* fd = NULL;

    if (append) {
        fd = ::fopen(file.c_str(), "a+");
    } else {
        fd = ::fopen(file.c_str(), "w");
    }

    if (NULL == fd) {
        return PERRORCODE(-1, errno, 
                    "open file(%s) failed",
                    file.c_str());
    }

    int ret = ::fprintf(fd, "%s\n", value.c_str());
    ::fclose(fd);

    if (ret <= 0) {
        return PERRORCODE(-1, errno, 
                    "close file(%s) failed",
                    file.c_str());
    }

    return ERRORCODE_OK;
}

int64_t CfsToMilliCore(int64_t cfs)
{
    if (cfs <= 0) {
        return -1L;
    }

    return cfs * MILLI_CORE / CFS_PERIOD;
}

int64_t ShareToMilliCore(int64_t share)
{
    if (share <= 0) {
        return 0;
    }

    return share * MILLI_CORE / CFS_SHARE;
}

int64_t MilliCoreToCfs(int64_t millicore)
{
    if (millicore <= 0) {
        return -1L;
    }

    return millicore * CFS_PERIOD / MILLI_CORE;
}

int64_t MilliCoreToShare(int64_t millicore)
{
    if (millicore <= 0) {
        return 0L;
    }

    return millicore * CFS_SHARE / MILLI_CORE;
}

} //namespace cgroup
} //namespace galaxy
} //namespace baidu
