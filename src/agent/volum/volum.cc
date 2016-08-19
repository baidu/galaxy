// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum.h"
#include "protocol/galaxy.pb.h"
#include "agent/util/path_tree.h"
#include "tmpfs_volum.h"
#include "symlink_volum.h"
#include "bind_volum.h"
#include "boost/lexical_cast/lexical_cast_old.hpp"


#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>

namespace baidu {
namespace galaxy {
namespace volum {

Volum::Volum() :
    gc_index_(-1) {
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

void Volum::SetGcIndex(int32_t index) {
    assert(index > 0);
    gc_index_ = index;
}

const std::string Volum::ContainerId() const {
    return container_id_;
}

const boost::shared_ptr<baidu::galaxy::proto::VolumRequired> Volum::Description() const {
    return vr_;
}

std::string Volum::SourceRootPath() {
    boost::filesystem::path path(vr_->source_path());
    path.append("galaxy");
    path.append(container_id_);
    return path.string();
}

std::string Volum::SourcePath() {
    boost::filesystem::path path(SourceRootPath());
    path.append(vr_->dest_path());
    return path.string();
}

std::string Volum::TargetPath() {
    boost::filesystem::path path(baidu::galaxy::path::ContainerRootPath(container_id_));
    path.append(vr_->dest_path());
    return path.string();
}


std::string Volum::SourceGcRootPath() {
    assert(gc_index_ >= 0);
    boost::filesystem::path path(vr_->source_path());
    path.append("galaxy_gc");
    path.append(container_id_ + "." + boost::lexical_cast<std::string>(gc_index_));
    return path.string();
}

std::string Volum::SourceGcPath() {
    assert(gc_index_ >= 0);
    boost::filesystem::path path(SourceGcRootPath());
    path.append(vr_->dest_path());
    return path.string();
}

std::string Volum::TargetGcPath() {
    assert(gc_index_ >= 0);
    boost::filesystem::path path(baidu::galaxy::path::ContainerGcDir(ContainerId(), gc_index_));
    path.append(vr_->dest_path());
    return path.string();
}

baidu::galaxy::util::ErrorCode Volum::Destroy() {
    return ERRORCODE_OK;
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

