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
VolumResource::VolumResource() {
}

VolumResource::~VolumResource() {
}

// cofig format: size_in_GB:mediu(DISK:SSD):mount_point
baidu::galaxy::util::ErrorCode VolumResource::LoadVolum(const std::string& config, Volum& volum) {
    std::vector<std::string> v;
    boost::split(v, config, boost::is_any_of(":"));

    if (v.size() != 4) {
        return ERRORCODE(-1, "spilt size is %d, expect 4", v.size());
    }

    boost::filesystem::path fs(v[0]);
    boost::filesystem::path mt(v[3]);
    boost::system::error_code ec;

    if (!boost::filesystem::exists(fs, ec)
            || !boost::filesystem::exists(mt, ec)) {
        return ERRORCODE(-1, "%s or %s donot exist",
                fs.string().c_str(),
                mt.string().c_str());
    }

    volum.filesystem_ = v[0];
    volum.mount_point_ = v[3];
    //FIXME: check size_in_byte is num or not
    volum.total_ = boost::lexical_cast<int64_t>(v[1]);

    if ("DISK" == v[2]) {
        volum.medium_ = baidu::galaxy::proto::kDisk;
    } else if ("SSD" == v[2]) {
        volum.medium_ = baidu::galaxy::proto::kSsd;
    } else {
        return ERRORCODE(-1, "volum medium is %s, expect SSD or DISK", v[2].c_str());
    }

    return ERRORCODE_OK;
}

int VolumResource::Load() {
    assert(!FLAGS_volum_resource.empty());
    std::vector<std::string> vs;
    boost::split(vs, FLAGS_volum_resource, boost::is_any_of(","));

    for (size_t i = 0; i < vs.size(); i++) {
        VolumResource::Volum volum;
        baidu::galaxy::util::ErrorCode ec = LoadVolum(vs[i], volum);

        if (0 != ec.Code()) {
            //LOG
            LOG(WARNING) << "load volum failed: " << vs[i] << ":" << ec.Message();
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

baidu::galaxy::util::ErrorCode VolumResource::Allocat(const baidu::galaxy::proto::VolumRequired& require) {
    if (require.medium() == baidu::galaxy::proto::kTmpfs) {
        return ERRORCODE(-1, "medium is kTmpfs");
    }

    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.find(require.source_path());

    if (iter == resource_.end()) {
        return ERRORCODE(-1, "source path %s donot exist");
    }

    if (require.medium() != iter->second.medium_) {
        return ERRORCODE(-1, "medium donot match");
    }

    if (iter->second.assigned_ + require.size() > iter->second.total_) {
        return ERRORCODE(-1, "assigned(%lld) + requie(%lld) > %lld",
                (long long int)iter->second.assigned_,
                (long long int)require.size(),
                (long long int)iter->second.total_);
    }

    iter->second.assigned_ += require.size();
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode VolumResource::Release(const baidu::galaxy::proto::VolumRequired& require) {
    if (require.medium() == baidu::galaxy::proto::kTmpfs) {
        return ERRORCODE(-1, "volum must notbe KTmpfs");
    }

    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.find(require.source_path());

    if (iter == resource_.end()) {
        return ERRORCODE(-1,
                "required source path %s donot exist",
                require.source_path().c_str());
    }

    if (iter->second.assigned_ < require.size()) {
        return ERRORCODE(-1,
                "resource overflow: %lld < %lld",
                (long long int)iter->second.assigned_,
                (long long int)require.size());
    }

    iter->second.assigned_ -= require.size();
    return ERRORCODE_OK;
}

void VolumResource::Resource(std::map<std::string, VolumResource::Volum>& r) {
    std::map<std::string, VolumResource::Volum>::iterator iter = resource_.begin();

    while (iter != resource_.end()) {
        r[iter->first] = iter->second;
        iter++;
    }
}
}
}
}
