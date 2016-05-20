// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum_resource.h"
#include "protocol/galaxy.pb.h"
#include <iostream>

namespace baidu {
namespace galaxy {
namespace resource {
VolumResource::VolumResource()
{

}

VolumResource::~VolumResource()
{

}

int VolumResource::Load()
{
    {
        std::string key = "/home/disk1";
        Volum v;
        v.total_ = (int64_t)(2.7 * 1024 * 1024 * 1024 * 1024);
        v.assigned_ = 0;
        v.medium_ = baidu::galaxy::proto::kDisk;
        resource_[key] = v;
    }

    {
        std::string key = "/home/ssd0";
        Volum v;
        v.total_ = 400L * 1024L * 1024L * 1024L;
        v.assigned_ = 0;
        v.medium_ = baidu::galaxy::proto::kSsd;
        resource_[key] = v;
    }

    return 0;
}

int VolumResource::Allocat(const baidu::galaxy::proto::VolumRequired& require)
{
    if (require.medium() == baidu::galaxy::proto::kTmpfs) {
        return -1;
    }

    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.find(require.source_path());
    if (iter == resource_.end()) {
        return -1;
    }

    if (require.medium() != iter->second.medium_) {
        return -1;
    }

    if (iter->second.assigned_ + require.size() > iter->second.total_) {
        return -1;
    }

    iter->second.assigned_ += require.size();
    return 0;
}

int VolumResource::Release(const baidu::galaxy::proto::VolumRequired& require)
{
    if (require.medium() == baidu::galaxy::proto::kTmpfs) {
        return -1;
    }

    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.find(require.source_path());
    if (iter == resource_.end()) {
        return -1;
    }

    if (iter->second.assigned_ + require.size() > iter->second.total_) {
        return -1;
    }

    iter->second.assigned_ -= require.size();
    return 0;
}

void VolumResource::Resource(std::map<std::string, VolumResource::Volum>& r)
{
    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.begin();
    while (iter != resource_.end()) {
        r[iter->first] = iter->second;
        iter++;
    }
}
}
}
}
