// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "container_status.h"
#include <assert.h>

namespace baidu {
namespace galaxy {
namespace container {

std::set<baidu::galaxy::proto::ContainerStatus> ContainerStatus::kallocating_pre_status_;
std::set<baidu::galaxy::proto::ContainerStatus> ContainerStatus::kready_pre_status_;
std::set<baidu::galaxy::proto::ContainerStatus> ContainerStatus::kerror_pre_status_;
std::set<baidu::galaxy::proto::ContainerStatus> ContainerStatus::kdestroying_pre_status_;
std::set<baidu::galaxy::proto::ContainerStatus> ContainerStatus::kterminated_pre_status_;
bool ContainerStatus::setup_ok_ = false;


void ContainerStatus::Setup()
{
    // kAllocating
    kallocating_pre_status_.insert(baidu::galaxy::proto::kContainerPending);
    // kReady
    kready_pre_status_.insert(baidu::galaxy::proto::kContainerPending);
    kready_pre_status_.insert(baidu::galaxy::proto::kContainerAllocating);
    kready_pre_status_.insert(baidu::galaxy::proto::kContainerReady);
    // kError
    kerror_pre_status_.insert(baidu::galaxy::proto::kContainerAllocating);
    kerror_pre_status_.insert(baidu::galaxy::proto::kContainerReady);
    kerror_pre_status_.insert(baidu::galaxy::proto::kContainerDestroying);
    kerror_pre_status_.insert(baidu::galaxy::proto::kContainerError);
    // kDestroying
    kdestroying_pre_status_.insert(baidu::galaxy::proto::kContainerPending);
    kdestroying_pre_status_.insert(baidu::galaxy::proto::kContainerReady);
    kdestroying_pre_status_.insert(baidu::galaxy::proto::kContainerError);
    // kTerminated
    kterminated_pre_status_.insert(baidu::galaxy::proto::kContainerDestroying);
    setup_ok_ = true;
}



ContainerStatus::ContainerStatus(const std::string& container_id) :
    status_(baidu::galaxy::proto::kContainerPending),
    old_status_(baidu::galaxy::proto::kContainerAllocating),
    change_time_(0),
    container_id_(container_id)
{
}

ContainerStatus::~ContainerStatus()
{
}

baidu::galaxy::util::ErrorCode ContainerStatus::EnterAllocating()
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);

    if (baidu::galaxy::proto::kContainerAllocating == status_) {
        return ERRORCODE(baidu::galaxy::util::kErrorRepeated, "already in kContainerAllocating status");
    }

    return Enter(ContainerStatus::kallocating_pre_status_, baidu::galaxy::proto::kContainerAllocating);
}

baidu::galaxy::util::ErrorCode ContainerStatus::EnterReady()
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);

    if (baidu::galaxy::proto::kContainerAllocating != status_
            && baidu::galaxy::proto::kContainerReady != status_) {
        return ERRORCODE(baidu::galaxy::util::kErrorNotAllowed, "container CANNOT change from %s to %s",
                baidu::galaxy::proto::ContainerStatus_Name(status_).c_str(),
                "kContainerReady"
                                                );
    }

    return Enter(kready_pre_status_, baidu::galaxy::proto::kContainerReady);
}

baidu::galaxy::util::ErrorCode ContainerStatus::EnterError()
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);
    return Enter(kerror_pre_status_, baidu::galaxy::proto::kContainerError);
}

baidu::galaxy::util::ErrorCode ContainerStatus::EnterDestroying()
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);

    if (baidu::galaxy::proto::kContainerDestroying == status_) {
        return ERRORCODE(baidu::galaxy::util::kErrorRepeated,
                "already in %s status",
                baidu::galaxy::proto::ContainerStatus_Name(baidu::galaxy::proto::kContainerDestroying).c_str());
    }

    return Enter(kdestroying_pre_status_, baidu::galaxy::proto::kContainerDestroying);
}

baidu::galaxy::util::ErrorCode ContainerStatus::EnterTerminated()
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);
    return Enter(kterminated_pre_status_, baidu::galaxy::proto::kContainerTerminated);
}
baidu::galaxy::util::ErrorCode  ContainerStatus::EnterErrorFrom(baidu::galaxy::proto::ContainerStatus prestatus)
{
    assert(ContainerStatus::setup_ok_);
    boost::mutex::scoped_lock lock(mutex_);
    if (status_ != prestatus) {
        return ERRORCODE(-1, "current status is %s, expect %s",
                baidu::galaxy::proto::ContainerStatus_Name(status_).c_str(),
                baidu::galaxy::proto::ContainerStatus_Name(prestatus).c_str()
                                                );
    }
    return Enter(kerror_pre_status_, baidu::galaxy::proto::kContainerError);
}

baidu::galaxy::util::ErrorCode ContainerStatus::Enter(const std::set<baidu::galaxy::proto::ContainerStatus>& allow_set,
        baidu::galaxy::proto::ContainerStatus target_status)
{
    std::set<baidu::galaxy::proto::ContainerStatus>::const_iterator iter = allow_set.find(status_);

    if (allow_set.end() == iter) {
        return ERRORCODE(baidu::galaxy::util::kErrorNotAllowed,
                "container %s CANNOT change from %s to %s",
                container_id_.c_str(),
                baidu::galaxy::proto::ContainerStatus_Name(status_).c_str(),
                baidu::galaxy::proto::ContainerStatus_Name(target_status).c_str());
    }

    old_status_ = status_;
    status_ = target_status;
    return ERRORCODE(baidu::galaxy::util::kErrorOk,
            "enter %s status successfully",
            baidu::galaxy::proto::ContainerStatus_Name(target_status).c_str());
}

baidu::galaxy::proto::ContainerStatus ContainerStatus::Status()
{
    boost::mutex::scoped_lock lock(mutex_);
    return status_;
}

bool ContainerStatus::CmpRetOld(const baidu::galaxy::proto::ContainerStatus status,
        baidu::galaxy::proto::ContainerStatus* old)
{
    boost::mutex::scoped_lock lock(mutex_);

    if (NULL != old) {
        *old = status_;
    }

    return (status == status_);
}

}
}
}
