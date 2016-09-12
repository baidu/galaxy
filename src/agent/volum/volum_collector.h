// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "collector/collector.h"
#include "util/error_code.h"
#include "boost/filesystem/path.hpp"
#include "boost/thread/mutex.hpp"
#include <string>
namespace baidu {
namespace galaxy {
namespace volum {

class VolumCollector : public baidu::galaxy::collector::Collector {
public:
    explicit VolumCollector(const std::string& phy_path);
    ~VolumCollector();
    baidu::galaxy::util::ErrorCode Collect();
    void Enable(bool enable);
    bool Enabled();
    bool Equal(const Collector*);
    int Cycle(); // unit second
    std::string Name() const;
    std::string Path() const;

    void SetCycle(const int cycle);

    int64_t Size();
private:
    void Du(const boost::filesystem::path& p, int64_t& size, int64_t& counter);
    bool enable_;
    int cycle_;
    std::string name_;
    std::string phy_path_;

    boost::mutex mutex_;
    int64_t size_;
};
}
}
}
