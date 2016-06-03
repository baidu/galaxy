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
    explicit CgroupCollector(Cgroup* cgroup);
    ~CgroupCollector();

    baidu::galaxy::util::ErrorCode Collect();

    void Enable(bool enable);
    bool Enabled();
    bool Equal(const Collector&);
    void SetCycle(int cycle);
    int Cycle(); // unit second
    std::string Name() const;
    void SetName(const std::string& name);
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> Statistics();

private:
    bool enabled_;
    int cycle_;
    Cgroup* cgroup_;
    std::string name_;
    boost::mutex mutex_;

    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix_;
    int64_t last_time_;

};
}
}
}
