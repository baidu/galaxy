// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum_resource.h"
#include "protocol/galaxy.pb.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast/lexical_cast_old.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <assert.h>

#include <vector>
#include <iostream>

DECLARE_string(volum_resource);

namespace baidu {
namespace galaxy {
namespace resource {
VolumResource::VolumResource()
{
}

VolumResource::~VolumResource()
{
}

// cofig format: size_in_GB:mediu(DISK:SSD):mount_point
int VolumResource::LoadVolum(const std::string& config, Volum& volum)
{
    std::vector<std::string> v;
    boost::split(v, config, boost::is_any_of(","));
    if (v.size() == 4) {
        return -1;
    }

    boost::filesystem::path fs(v[0]);
    boost::filesystem::path mt(v[3]);

    boost::system::error_code ec;
    if (!boost::filesystem::exists(fs, ec)
            || !boost::filesystem::exists(mt, ec)) {
        return -1;
    }
    volum.filesystem_ = v[0];
    volum.mount_point_ = v[3];

    //FIXME: check size_in_byte is num or not

    volum.total_ = boost::lexical_cast<int64_t>(v[1]);

    if ("DISK" == v[3]) {
        volum.medium_ = baidu::galaxy::proto::kDisk;
    } else if ("SSD" == v[3]) {
        volum.medium_ = baidu::galaxy::proto::kSsd;
    } else {
        return -1;
    }
    return 0;
}

int VolumResource::Load()
{
    assert(!FLAGS_volum_resource.empty());
    std::vector<std::string> vs;
    boost::split(vs, FLAGS_volum_resource, boost::is_any_of(","));
    for (size_t i = 0; i < vs.size(); i++) {
        VolumResource::Volum volum;
        if (0 != LoadVolum(vs[i], volum)) {
            //LOG
            return -1;
        } else {
            resource_[volum.mount_point_] = volum;
        }
    }

    if (resource_.size() == 0) {
        return -1;
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
