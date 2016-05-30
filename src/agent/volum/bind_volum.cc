// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "bind_volum.h"
#include "protocol/galaxy.pb.h"
#include "mounter.h"

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

baidu::galaxy::util::ErrorCode BindVolum::Construct()
{
    const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr = Description();
    assert(vr->medium() == baidu::galaxy::proto::kDisk
            || vr->medium() == baidu::galaxy::proto::kSsd);
    assert(!vr->use_symlink());
    boost::system::error_code ec;
    boost::filesystem::path target_path(this->TargetPath());

    if (!boost::filesystem::exists(target_path, ec) && !boost::filesystem::create_directories(target_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                target_path.string().c_str(),
                ec.message().c_str());
    }

    boost::filesystem::path source_path(this->SourcePath());
    if (!boost::filesystem::exists(source_path, ec) && !boost::filesystem::create_directories(source_path, ec)) {
        return ERRORCODE(-1, "failed in creating dir(%s): %s",
                source_path.string().c_str(),
                ec.message().c_str());
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
    boost::filesystem::rename(source_path, gc_source_path, ec);
    if (0 != ec.value()) {
        return ERRORCODE(-1, "failed in renaming %s -> %s: %s",
                source_path.string().c_str(),
                gc_source_path.string().c_str());
    }
    return ERRORCODE_OK;
}

int64_t BindVolum::Used()
{
    return 0;
}

std::string BindVolum::ToString()
{
    return "";
}

}
}
}

