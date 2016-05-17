// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "symlink_volum.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <glog/logging.h>

#include <iostream>

namespace baidu {
namespace galaxy {
namespace volum {

SymlinkVolum::SymlinkVolum() {
}

SymlinkVolum::~SymlinkVolum() {
}

// FIX me: check link exist
int SymlinkVolum::Construct() {
    //const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    boost::system::error_code ec;
    boost::filesystem::path source_path(this->SourcePath());

    if (!boost::filesystem::exists(source_path, ec)
            && !boost::filesystem::create_directories(source_path, ec)) {
        LOG(WARNING) << "create_directories failed: " << source_path << " :" << ec.message();
        return -1;
    }

    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path.parent_path(), ec)
            && !boost::filesystem::create_directories(target_path.parent_path(), ec)) {
        LOG(WARNING) << "target's parent path (" << target_path.parent_path().string()
                     << " )doesnot exist, an is created failed: " << ec.message();
        return -1;
    }

    if (!boost::filesystem::exists(target_path, ec)) {
        boost::filesystem::create_symlink(source_path, target_path, ec);

        if (ec.value() != 0) {
            LOG(WARNING) << "target path (" << target_path.string()
                         << ") doesnot exist, an is created failed: " << ec.message();
            return -1;
        }
    } else {
        if (boost::filesystem::is_symlink(target_path, ec)) {
            boost::filesystem::path t = boost::filesystem::read_symlink(target_path, ec);

            if (0 == ec.value() && t == source_path) {
                return 0;
            } else {
                LOG(WARNING) << "target path is symlink, which links with another path" << target_path << "->" << t;
                return -1;
            }

            /*if (!boost::filesystem::remove(target_path, ec)) {
                 LOG(WARNING) << target_path.string() << "is symlink, and is removed failed: "
                     << ec.message();
                return -1;
            }

            boost::filesystem::create_symlink(source_path, target_path, ec);

            if (ec.value() != 0) {
                 LOG(WARNING) << "create symlink failed: " << ec.message();
                return -1;
            }*/
        } else {
            LOG(WARNING) << target_path.string() << " already exists and is not a symlink";
            return -1;
        }
    }

    return 0;
}

int SymlinkVolum::Destroy() {
    return -1;
}

int64_t SymlinkVolum::Used() {
    return 0;
}

std::string SymlinkVolum::ToString() {
    return "";
}

}
}
}
