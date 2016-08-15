// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "container_stage.h"
#include "util/error_code.h"
#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {
namespace container {

ContainerStage::ContainerStage() {
}

ContainerStage::~ContainerStage() {
}

baidu::galaxy::util::ErrorCode ContainerStage::EnterDestroyingStage(const std::string& id) {
    boost::mutex::scoped_lock lock(mutex_);
    boost::unordered_map<std::string, baidu::galaxy::proto::ContainerStatus>::const_iterator iter = stage_.find(id);

    if (stage_.end() == iter) {
        stage_[id] = baidu::galaxy::proto::kContainerDestroying;
        return ERRORCODE(baidu::galaxy::util::kErrorOk, "");
    }

    if (baidu::galaxy::proto::kContainerDestroying == iter->second) {
        return ERRORCODE(baidu::galaxy::util::kErrorRepeated,
                "container %s is in kContainerDestroying stage",
                id.c_str());
    }

    return ERRORCODE(baidu::galaxy::util::kErrorNotAllowed,
            "container %s CANNOT transfer from stage %s to stage %s",
            id.c_str(),
            baidu::galaxy::proto::ContainerStatus_Name(iter->second).c_str(),
            baidu::galaxy::proto::ContainerStatus_Name(baidu::galaxy::proto::kContainerDestroying).c_str());
}


baidu::galaxy::util::ErrorCode ContainerStage::EnterCreatingStage(const std::string& id) {
    boost::mutex::scoped_lock lock(mutex_);
    boost::unordered_map<std::string, baidu::galaxy::proto::ContainerStatus>::const_iterator iter = stage_.find(id);

    if (stage_.end() == iter) {
        stage_[id] = baidu::galaxy::proto::kContainerAllocating;
        return ERRORCODE(baidu::galaxy::util::kErrorOk, "");
    }

    if (baidu::galaxy::proto::kContainerAllocating == iter->second) {
        return ERRORCODE(baidu::galaxy::util::kErrorRepeated,
                "container %s is in %s stage",
                id.c_str(),
                baidu::galaxy::proto::ContainerStatus_Name(baidu::galaxy::proto::kContainerAllocating).c_str());
    }

    return ERRORCODE(baidu::galaxy::util::kErrorNotAllowed,
            "container %s CANNOT transfer from stage %s to stage %s",
            id.c_str(),
            baidu::galaxy::proto::ContainerStatus_Name(iter->second).c_str(),
            baidu::galaxy::proto::ContainerStatus_Name(baidu::galaxy::proto::kContainerAllocating).c_str());
}

baidu::galaxy::util::ErrorCode ContainerStage::LeaveDestroyingStage(const std::string& id) {
    boost::mutex::scoped_lock lock(mutex_);
    boost::unordered_map<std::string, baidu::galaxy::proto::ContainerStatus>::const_iterator iter = stage_.find(id);
    assert(stage_.end() != iter);
    assert(baidu::galaxy::proto::kContainerDestroying == iter->second);
    stage_.erase(iter);
    return ERRORCODE(baidu::galaxy::util::kErrorOk, "");
}

baidu::galaxy::util::ErrorCode ContainerStage::LeaveCreatingStage(const std::string& id) {
    boost::mutex::scoped_lock lock(mutex_);
    boost::unordered_map<std::string, baidu::galaxy::proto::ContainerStatus>::const_iterator iter = stage_.find(id);
    assert(stage_.end() != iter);
    assert(baidu::galaxy::proto::kContainerAllocating == iter->second);
    stage_.erase(iter);
    return ERRORCODE(baidu::galaxy::util::kErrorOk, "");
}

}
}
}
