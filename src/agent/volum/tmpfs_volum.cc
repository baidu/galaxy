// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tmpfs_volum.h"
#include "protocol/galaxy.pb.h"
#include "mounter.h"
#include "util/error_code.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <assert.h>

#include <sstream>

namespace baidu {
namespace galaxy {
namespace volum {
TmpfsVolum::TmpfsVolum() {
}

TmpfsVolum::~TmpfsVolum() {}

int TmpfsVolum::Construct() {
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    assert(baidu::galaxy::proto::kTmpfs == vr->medium());
    // create target dir
    boost::system::error_code ec;
    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path, ec) && !boost::filesystem::create_directories(target_path, ec)) {
        return  -1;
    }

    baidu::galaxy::util::ErrorCode errc = MountTmpfs(target_path.string(), vr->size(), vr->readonly());
    return errc.Code();
}

int TmpfsVolum::Destroy() {
    // do nothing
    return 0;
}

int64_t TmpfsVolum::Used() {
    return 0;
}

std::string TmpfsVolum::ToString() {
    return "";
}
}
}
}
