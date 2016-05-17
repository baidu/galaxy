// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tmpfs_volum.h"
#include "protocol/galaxy.pb.h"
#include "mounter.h"
#include "util/error_code.h"
#include "glog/logging.h"

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
    std::map<std::string, boost::shared_ptr<Mounter> > mounters;
    ListMounters(mounters);
    std::map<std::string, boost::shared_ptr<Mounter> >::iterator iter = mounters.find(target_path.string());

    if (iter != mounters.end() && iter->second->source == "tmpfs") {
        return 0;
    }

    if (iter != mounters.end()) {
        LOG(WARNING) << ContainerId() << " aleady mount another path on " << target_path.string() << " : " << iter->second->ToString();
        return -1;
    }

    if (!boost::filesystem::exists(target_path, ec) && !boost::filesystem::create_directories(target_path, ec)) {
        LOG(WARNING) << ContainerId() << " target_path " << target_path << " donot exist and is created failed: "
                     << ec.message();
        return  -1;
    }

    baidu::galaxy::util::ErrorCode errc = MountTmpfs(target_path.string(), vr->size(), vr->readonly());

    if (0 != errc.Code()) {
        LOG(WARNING) << ContainerId() << " mount tmpfs failed: " << errc.Message();
        return -1;
    }

    return 0;
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
