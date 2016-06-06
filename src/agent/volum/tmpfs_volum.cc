// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tmpfs_volum.h"
#include "protocol/galaxy.pb.h"
#include "mounter.h"
#include "util/error_code.h"
#include "collector/collector_engine.h"

#include "glog/logging.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <assert.h>
#include <sys/mount.h>

#include <sstream>

namespace baidu {
namespace galaxy {
namespace volum {
TmpfsVolum::TmpfsVolum()
{
}

TmpfsVolum::~TmpfsVolum() {}

baidu::galaxy::util::ErrorCode TmpfsVolum::Construct() {
    baidu::galaxy::util::ErrorCode err = Construct_();
    if (err.Code() == 0) {
        vc_.reset(new VolumCollector(this->TargetPath()));
        vc_->Enable(true);
        baidu::galaxy::collector::CollectorEngine::GetInstance()->Register(vc_);
    }
    return err;
}

baidu::galaxy::util::ErrorCode TmpfsVolum::Construct_()
{
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    assert(baidu::galaxy::proto::kTmpfs == vr->medium());
    // create target dir
    boost::system::error_code ec;
    boost::filesystem::path target_path(this->TargetPath());
    std::map<std::string, boost::shared_ptr<Mounter> > mounters;
    ListMounters(mounters);
    std::map<std::string, boost::shared_ptr<Mounter> >::iterator iter = mounters.find(target_path.string());

    if (iter != mounters.end() && iter->second->source == "tmpfs") {
        return ERRORCODE_OK;
    }

    if (iter != mounters.end()) {
        return ERRORCODE(-1, "mount another path");
    }

    if (!boost::filesystem::exists(target_path, ec) && !boost::filesystem::create_directories(target_path, ec)) {
        return ERRORCODE(-1, "failed in creating dirs(%s): %s",
                target_path.string().c_str(),
                ec.message().c_str());
    }

    baidu::galaxy::util::ErrorCode errc = MountTmpfs(target_path.string(), vr->size(), vr->readonly());

    if (0 != errc.Code()) {
        return ERRORCODE(-1, "mount failed: %s", errc.Message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode TmpfsVolum::Destroy()
{
    // do nothing
    if (vc_.get() != NULL) {
        vc_->Enable(false);
    }
    return Umount(this->TargetPath());
}

int64_t TmpfsVolum::Used()
{
    int64_t ret = 0L;
    if (NULL != vc_.get()) {
        ret = vc_->Size();
    }
    return ret;
}

std::string TmpfsVolum::ToString()
{
    return "";
}

}
}
}
