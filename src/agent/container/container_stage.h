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

class ScopedDestroyingStage {
    public:
        ScopedDestroyingStage(ContainerStage& stage, const std::string& id) :
            stage_(stage),
            id_(id) {
            ec_ = stage_.EnterDestroyingStage(id_);
        }

        ~ScopedDestroyingStage() {
            if (0 == ec_.Code()) {
                ec_ = stage_.LeaveDestroyingStage(id_);
            }
        }

        baidu::galaxy::util::ErrorCode GetLastError() {
            return ec_;
        }

    private:
        ContainerStage& stage_;
        const std::string id_;
        baidu::galaxy::util::ErrorCode ec_;

};


class ScopedCreatingStage {
    public:
        ScopedCreatingStage(ContainerStage& stage, const std::string& id) :
            stage_(stage),
            id_(id) {
            ec_ = stage_.EnterCreatingStage(id_);
        }

        ~ScopedCreatingStage() {
            if (0 == ec_.Code()) {
                ec_ = stage_.LeaveCreatingStage(id_);
            }
        }

        baidu::galaxy::util::ErrorCode GetLastError() {
            return ec_;
        }

    private:
        ContainerStage& stage_;
        const std::string id_;
        baidu::galaxy::util::ErrorCode ec_;

};



}
}
}
