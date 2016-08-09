// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "collector/collector.h"

#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"

namespace baidu {
namespace galaxy {
namespace proto {
class CgroupMetrix;
}

namespace cgroup {
class Cgroup;

class CgroupCollector : public baidu::galaxy::collector::Collector {
public:
    explicit CgroupCollector();
    ~CgroupCollector();

    baidu::galaxy::util::ErrorCode Collect();

    void Enable(bool enable);
    bool Enabled();
    bool Equal(const Collector*);
    void SetCycle(int cycle);
    int Cycle(); // unit second
    std::string Name() const;
    void SetName(const std::string& name);
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> Statistics();

    void SetCpuacctPath(const std::string& path) {
        cpuacct_path_ = path;
    }

    void SetMemoryPath(const std::string& path) {
        memory_path_ = path;
    }

private:
    baidu::galaxy::util::ErrorCode Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix);
    baidu::galaxy::util::ErrorCode ContainerCpuStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix);
    baidu::galaxy::util::ErrorCode SystemCpuStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix);
    baidu::galaxy::util::ErrorCode MemoryStat(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix);

    bool enabled_;
    int cycle_;
    std::string name_;
    boost::mutex mutex_;

    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix_;
    int64_t last_time_;
    std::string cpuacct_path_;
    std::string memory_path_;
};
}
}
}
