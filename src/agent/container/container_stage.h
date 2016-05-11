// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"

#include "protocol/galaxy.pb.h"
#include "boost/unordered_map.hpp"
#include "boost/thread/mutex.hpp"

#include <string>

namespace baidu {
namespace galaxy {
namespace container {

class ContainerStage {
public:
    ContainerStage();
    ~ContainerStage();

    baidu::galaxy::util::ErrorCode EnterDestroyingStage(const std::string& id);
    baidu::galaxy::util::ErrorCode EnterCreatingStage(const std::string& id);

    baidu::galaxy::util::ErrorCode LeaveDestroyingStage(const std::string& id);
    baidu::galaxy::util::ErrorCode LeaveCreatingStage(const std::string& id);

private:
    // use kContainerAlloctation & kContainerDestroying only
    boost::unordered_map<std::string, baidu::galaxy::proto::ContainerStatus> stage_;
    boost::mutex mutex_;

};
}
}
}
