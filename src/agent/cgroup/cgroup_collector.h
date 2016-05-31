// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "collector/collector.h"

#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"
#include "cgroup/metrix.h"

namespace baidu {
namespace galaxy {
namespace cgroup {

class Cgroup;

class CgroupCollector : public baidu::galaxy::collector::Collector {
public:
    explicit CgroupCollector(boost::shared_ptr<Cgroup> cgroup);
    ~CgroupCollector();

    baidu::galaxy::util::ErrorCode Collect();

    void Enable(bool enable);
    bool Enabled();
    bool Equal(const Collector&);
    void SetCycle(int cycle);
    int Cycle(); // unit second
    std::string Name() const;
    void SetName(const std::string& name);
    int GetMetrix(Metrix* metrix);

private:
    bool enabled_;
    int cycle_;
    boost::shared_ptr<Cgroup> cgroup_;
    std::string name_;
    boost::mutex mutex_;

    boost::shared_ptr<Metrix> metrix_;

};
}
}
}
