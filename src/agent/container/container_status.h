// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include "protocol/galaxy.pb.h"
#include "boost/thread/mutex.hpp"

#include <string>
#include <set>

namespace baidu {
namespace galaxy {
namespace container {
class ContainerStatus {
public:
    ContainerStatus(const std::string& id);
    ~ContainerStatus();
    static void Setup();

    // for test
    void SetStatus(baidu::galaxy::proto::ContainerStatus cs) {
        boost::mutex::scoped_lock lock(mutex_);
        status_ = cs;
    }

    baidu::galaxy::util::ErrorCode EnterAllocating();
    baidu::galaxy::util::ErrorCode EnterReady();
    baidu::galaxy::util::ErrorCode EnterError();
    baidu::galaxy::util::ErrorCode EnterDestroying();
    baidu::galaxy::util::ErrorCode EnterTerminated();
    baidu::galaxy::proto::ContainerStatus Status();
    bool CmpRetOld(const baidu::galaxy::proto::ContainerStatus status,
            baidu::galaxy::proto::ContainerStatus* old);

    // 当前模式为prestatus时才能进入status
    baidu::galaxy::util::ErrorCode  EnterErrorFrom(baidu::galaxy::proto::ContainerStatus prestatus);

private:
    baidu::galaxy::util::ErrorCode Enter(const std::set<baidu::galaxy::proto::ContainerStatus>& allow,
            baidu::galaxy::proto::ContainerStatus target_status);

    boost::mutex mutex_;
    baidu::galaxy::proto::ContainerStatus status_;
    baidu::galaxy::proto::ContainerStatus old_status_;
    int64_t change_time_;
    const std::string& container_id_;

    static std::set<baidu::galaxy::proto::ContainerStatus> kallocating_pre_status_;
    static std::set<baidu::galaxy::proto::ContainerStatus> kready_pre_status_;
    static std::set<baidu::galaxy::proto::ContainerStatus> kerror_pre_status_;
    static std::set<baidu::galaxy::proto::ContainerStatus> kdestroying_pre_status_;
    static std::set<baidu::galaxy::proto::ContainerStatus> kterminated_pre_status_;
    static bool setup_ok_;

};
}
}
}
