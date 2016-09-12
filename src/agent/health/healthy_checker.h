// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "util/error_code.h"
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "thread.h"

#include <string>
#include <vector>
#include <map>

namespace baidu {
namespace galaxy {

namespace volum {
class Mounter;
}

namespace cgroup {
class SubsystemFactory;
}

namespace resource {
class ResourceManager;
}


namespace health {

class HealthChecker {

public:
    HealthChecker();
    ~HealthChecker();

    bool Healthy();

    void LoadVolum(const boost::shared_ptr<baidu::galaxy::resource::ResourceManager> res);
    void LoadCgroup(const boost::shared_ptr<baidu::galaxy::cgroup::SubsystemFactory> fac);

    void Setup();

private:
    void CheckRoutine();
    typedef std::map<std::string, boost::shared_ptr<baidu::galaxy::volum::Mounter> > MMounter;
    baidu::galaxy::util::ErrorCode CheckVolumMounter(const MMounter& mounters);
    baidu::galaxy::util::ErrorCode CheckCgroupMounter(const MMounter& mounters);
    baidu::galaxy::util::ErrorCode CheckVolumReadable();

    std::vector<std::string> volums_;
    std::vector<std::string> cgroups_;
    boost::mutex mutex_;
    bool running_;
    bool healthy_;
    baidu::galaxy::util::ErrorCode last_error_;
    baidu::common::Thread thread_;

};
}
}
}
