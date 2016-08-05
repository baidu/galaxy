// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "bind_volum.h"
#include "protocol/galaxy.pb.h"
#include "mounter.h"
#include "util/user.h"
#include "util/util.h"
#include "glog/logging.h"
#include "collector/collector_engine.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sys/mount.h>

namespace baidu {
namespace galaxy {
namespace volum {

BindVolum::BindVolum()
{
}

BindVolum::~BindVolum()
{
}


baidu::galaxy::util::ErrorCode BindVolum::Construct() {
    baidu::galaxy::util::ErrorCode err = Construct_();

    if (err.Code() == 0) {
        vc_.reset(new VolumCollector(this->SourcePath()));
        vc_->Enable(true);
        baidu::galaxy::collector::CollectorEngine::GetInstance()->Register(vc_);
    }

    return err;
}

baidu::galaxy::util::ErrorCode BindVolum::Construct_()
{
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    assert(vr->medium() == baidu::galaxy::proto::kDisk
            || vr->medium() == baidu::galaxy::proto::kSsd);
    assert(!vr->use_symlink());
    boost::system::error_code ec;
    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path, ec) 
                && !baidu::galaxy::file::create_directories(target_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                target_path.string().c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path source_path(this->SourcePath());
    if (!boost::filesystem::exists(source_path, ec) 
                && !baidu::galaxy::file::create_directories(source_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                source_path.string().c_str(),
                ec.message().c_str());
    }

    std::string user = Owner();
    if (!user.empty()) {
        baidu::galaxy::util::ErrorCode err = baidu::galaxy::user::Chown(source_path.string(), user);
        if (0 != err.Code()) {
            return ERRORCODE(-1, "chown %s to user %s failed: %s",
                        source_path.string().c_str(),
                        user.c_str(),
                        err.Message().c_str());
        }
        
    }

    std::map<std::string, boost::shared_ptr<Mounter> > mounters;
    ListMounters(mounters);
    std::map<std::string, boost::shared_ptr<Mounter> >::iterator iter = mounters.find(target_path.string());
    if (iter != mounters.end()) {
        return ERRORCODE(0, "already bind");
    }

    return MountDir(source_path.string(), target_path.string());
}


baidu::galaxy::util::ErrorCode BindVolum::Destroy()
{
    if (NULL != vc_.get()) {
        vc_->Enable(false);
    }

    baidu::galaxy::util::ErrorCode err = Umount(this->TargetPath());
    if (err.Code() != 0) {
        return err;
    }

    boost::system::error_code ec;
    boost::filesystem::path source_path(this->SourcePath());
    if (!boost::filesystem::exists(source_path, ec)) {
        return ERRORCODE_OK;
    }

    boost::filesystem::path gc_source_path(SourceGcPath());
    if (boost::filesystem::exists(gc_source_path, ec)) {
        boost::filesystem::remove_all(source_path, ec);
        LOG(INFO) << "gc source path exist, remove source path directly: " << source_path.string();
        if (ec.value() == 0) {
            return ERRORCODE_OK;
        } else {
            return ERRORCODE(-1, "remove %s failed", source_path.string().c_str());
        }
    }

    if (!boost::filesystem::exists(gc_source_path.parent_path(), ec)) {
        baidu::galaxy::file::create_directories(gc_source_path.parent_path(), ec);
    }


    boost::filesystem::rename(source_path, gc_source_path, ec);
    if (0 != ec.value()) {
        return ERRORCODE(-1, "failed in renaming %s -> %s: %s",
                source_path.string().c_str(),
                gc_source_path.string().c_str(),
                ec.message().c_str());
    }
    return ERRORCODE_OK;
}

int64_t BindVolum::Used()
{
    if (vc_.get() != NULL) {
        return vc_->Size();
    }
    return 0L;
}

baidu::galaxy::util::ErrorCode BindVolum::Gc() {
    boost::filesystem::path source_root_path(SourceRootPath());
    boost::system::error_code ec;
    if (!boost::filesystem::exists(source_root_path), ec) {
        return ERRORCODE_OK;
    }

    boost::filesystem::remove_all(source_root_path, ec);
    if (ec.value() != 0) {
        return ERRORCODE(-1, 
                    "remove %s failed: %s", 
                    source_root_path.c_str(), 
                    ec.message().c_str());
    }

    return ERRORCODE_OK;
}

std::string BindVolum::ToString()
{
    return "";
}

}
}
}

