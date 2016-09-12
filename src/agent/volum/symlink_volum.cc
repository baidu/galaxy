// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "symlink_volum.h"
#include "protocol/galaxy.pb.h"

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
baidu::galaxy::util::ErrorCode SymlinkVolum::Construct() {
    //const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    boost::system::error_code ec;
    boost::filesystem::path source_path(this->SourcePath());

    if (!boost::filesystem::exists(source_path, ec)
            && !baidu::galaxy::file::create_directories(source_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                source_path.string().c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path.parent_path(), ec)
            && !baidu::galaxy::file::create_directories(target_path.parent_path(), ec)) {
        return ERRORCODE(-1,
                "failed in creating dir(%s): %s",
                target_path.parent_path().string().c_str(),
                ec.message().c_str());
    }

    if (!boost::filesystem::exists(target_path, ec)) {
        boost::filesystem::create_symlink(source_path, target_path, ec);

        if (ec.value() != 0) {
            return ERRORCODE(-1, "failed in creating dir(%s): %s",
                    target_path.string().c_str(),
                    ec.message().c_str());
        }
    } else {
        if (boost::filesystem::is_symlink(target_path, ec)) {
            boost::filesystem::path t = boost::filesystem::read_symlink(target_path, ec);

            if (0 == ec.value() && t == source_path) {
                return ERRORCODE_OK;
            } else {
                return ERRORCODE(-1, "target path(%s) is symlink, which links with another path(%s), expect %s",
                        target_path.string().c_str(),
                        t.string().c_str(),
                        source_path.string().c_str());
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
            return ERRORCODE(-1, "failed in create dir %s already exists and is not a symlink",
                    target_path.string().c_str());
        }
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode SymlinkVolum::Destroy() {
    boost::filesystem::path source_path(SourcePath());
    boost::filesystem::path target_path(TargetPath());
    boost::filesystem::path gc_source_path(SourceGcPath());
    boost::filesystem::path gc_target_path(TargetGcPath());
    boost::system::error_code ec;

    if (boost::filesystem::exists(source_path, ec)) {
        boost::filesystem::rename(source_path, gc_source_path, ec);

        if (0 != ec.value()) {
            return ERRORCODE(-1, "failed in renaming file(%s->%s): %s",
                    source_path.string().c_str(),
                    gc_source_path.string().c_str(),
                    ec.message().c_str());
        }
    }

    boost::filesystem::remove_all(target_path, ec);

    if (ec.value() != 0) {
        return ERRORCODE(-1, "failed in removing file(%s): %s",
                target_path.c_str(),
                ec.message().c_str());
    }

    boost::filesystem::create_symlink(gc_source_path, target_path, ec);

    if (ec.value() != 0) {
        return ERRORCODE(-1, "create_symlink(%s->%s) failed: %s",
                gc_target_path.string().c_str(),
                gc_source_path.string().c_str(),
                ec.message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode SymlinkVolum::Gc() {
    return ERRORCODE_OK;
}

int64_t SymlinkVolum::Used() {
    return 0;
}

std::string SymlinkVolum::ToString() {
    std::stringstream ss;
    ss << "Symlink: " << this->TargetPath() << " -> " << this->SourcePath() << "\t"
       << "Size: " << this->Description()->size();
    return ss.str();
}

}
}
}
