// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum.h"
#include "protocol/galaxy.pb.h"
#include "agent/util/path_tree.h"
#include "tmpfs_volum.h"
#include "symlink_volum.h"
#include "bind_volum.h"


#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>

#include <iostream>


namespace baidu {
namespace galaxy {
namespace volum {

Volum::Volum() {
}

Volum::~Volum() {
}

void Volum::SetDescription(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr) {
    assert(NULL != vr.get());
    vr_ = vr;
}

void Volum::SetContainerId(const std::string& container_id) {
    container_id_ = container_id;
}

const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> Volum::Descrition() {
    return vr_;
}

std::string Volum::ToString() {
    if (vr_.get() != NULL) {
        return container_id_ + "||" + vr_->DebugString();
    } else {
        return "empty volum";
    }
}

int Volum::Construct() {
    assert(NULL != vr_.get());
    boost::system::error_code ec;
    boost::filesystem::path targe_path(this->TargetPath());
    int ret = -1;
    do {
        if (vr_->medium() == baidu::galaxy::proto::kTmpfs) {
            if (!boost::filesystem::exists(targe_path, ec) && !boost::filesystem::create_directories(targe_path, ec)) {
                ret = -1;
                break;
            }

            std::stringstream ss;
            ss << "size=" << vr_->size();
            mounter_.SetOption(ss.str());
            mounter_.SetType(Mounter::FT_TMPFS);
            mounter_.SetRdonly(vr_->readonly());
            mounter_.SetTarget(targe_path.string());
            ret = 0;

        } else if (vr_->has_use_symlink() && vr_->use_symlink()) {
            boost::filesystem::path source_path(this->SourcePath());
            if (!boost::filesystem::exists(source_path, ec) 
                        && !boost::filesystem::create_directories(source_path, ec)) {
                ret = -1;
                break;
            }

            boost::filesystem::path target_path(this->TargetPath());
            if (!boost::filesystem::exists(target_path.parent_path(), ec) 
                        && !boost::filesystem::create_directories(target_path.parent_path(), ec)) {
                ret = -1;
                break;
            }


            mounter_.SetSource(this->SourcePath());
            mounter_.SetTarget(this->TargetPath());
            mounter_.SetType(Mounter::FT_SYMLINK);
            ret = 0;

        } else {
            boost::filesystem::path source_path(this->SourcePath());

            if (!boost::filesystem::exists(source_path, ec) && !boost::filesystem::create_directories(source_path, ec)) {
                ret = -1;
                break;
            }

            
            boost::filesystem::path target_path(this->TargetPath());

            if (!boost::filesystem::exists(target_path, ec) && !boost::filesystem::create_directories(target_path, ec)) {
                ret = -1;
                break;
            }

std::cerr << source_path << " " << target_path << std::endl;

            mounter_.SetType(Mounter::FT_DIR);
            mounter_.SetBind(true);
            mounter_.SetRdonly(vr_->readonly());
            mounter_.SetSource(source_path.string());
            mounter_.SetTarget(targe_path.string());
            ret = 0;
        }
    } while (0);
    
    return ret;
}

int Volum::Mount() {
    if (0 != Mounter::Mount(&mounter_)) {
        return -1;
    }

    return 0;
=======
const std::string Volum::ContainerId() const {
    return container_id_;
}

const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> Volum::Description() const {
    return vr_;
}

std::string Volum::SourcePath() {
    boost::filesystem::path path(vr_->source_path());
    path.append("galaxy");
    path.append("container");
    path.append(container_id_);
    return path.string();
}

std::string Volum::TargetPath() {
    boost::filesystem::path path(baidu::galaxy::path::ContainerRootPath(container_id_));
    path.append(vr_->dest_path());
    return path.string();
}

int Volum::Destroy() {
    boost::filesystem::path source_path(this->SourcePath());
    boost::system::error_code ec;

    if (vr_->type() == baidu::galaxy::proto::KEmptyDir
            && vr_->medium() != baidu::galaxy::proto::kTmpfs
            && boost::filesystem::exists(this->SourcePath(), ec)) {
        return boost::filesystem::remove_all(source_path, ec) ? 0 : 1;
    }

    return 0;
}

boost::shared_ptr<Volum> Volum::CreateVolum(const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> vr) {
    assert(NULL != vr.get());
    boost::shared_ptr<Volum> ret;

    if (vr->medium() == baidu::galaxy::proto::kTmpfs) {
        ret.reset(new TmpfsVolum());
    } else if (vr->has_use_symlink() && vr->use_symlink()) {
        ret.reset(new SymlinkVolum());
    } else {
        ret.reset(new BindVolum());
    }

    return ret;
}
}
}
}

int Volum::Destroy() {
    boost::filesystem::path source_path(this->SourcePath());
    boost::system::error_code ec;

    if (vr_->type() == baidu::galaxy::proto::kEmptyDir 
                && vr_->medium() != baidu::galaxy::proto::kTmpfs
                && boost::filesystem::exists(this->SourcePath(), ec)) {
        return boost::filesystem::remove_all(source_path, ec) ? 0 : 1;
    }

    return 0;
}

int64_t Volum::Used() {
    assert(0);
    return 0;
}

std::string Volum::SourcePath() {
    boost::filesystem::path path(vr_->source_path());
    path.append("galaxy");
    path.append("container");
    path.append(container_id_);
    return path.string();
}

std::string Volum::TargetPath() {
    boost::filesystem::path path(baidu::galaxy::path::ContainerRootPath(container_id_));
    path.append(vr_->dest_path());
    return path.string();
}


} //namespace volum
} //namespace galaxy
} //namespace baidu


