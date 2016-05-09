// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum_group.h"
#include "volum.h"
#include "mounter.h"

#include "protocol/galaxy.pb.h"
#include "agent/volum/volum.h"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "util/error_code.h"
#include "boost/lexical_cast/lexical_cast_old.hpp"

#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_string(mount_templat);

namespace baidu {
namespace galaxy {
namespace volum {
VolumGroup::VolumGroup() {
}

VolumGroup::~VolumGroup() {
}

void VolumGroup::AddDataVolum(const baidu::galaxy::proto::VolumRequired& data_volum) {
    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> volum(new baidu::galaxy::proto::VolumRequired());
    volum->CopyFrom(data_volum);
    dv_description_.push_back(volum);
}

void VolumGroup::SetWorkspaceVolum(const baidu::galaxy::proto::VolumRequired& ws_volum) {
    ws_description_.reset(new baidu::galaxy::proto::VolumRequired);
    ws_description_->CopyFrom(ws_volum);
}

void VolumGroup::SetContainerId(const std::string& container_id) {
    container_id_ = container_id;
}

int VolumGroup::Construct() {
    workspace_volum_ = Construct(this->ws_description_);

    if (NULL == workspace_volum_.get()) {
        return -1;
    }

    for (size_t i = 0; i < dv_description_.size(); i++) {
        boost::shared_ptr<Volum> v = Construct(dv_description_[i]);

        if (v.get() == NULL) {
            break;
        }

        data_volum_.push_back(v);
    }

    if (data_volum_.size() != dv_description_.size()) {
        for (size_t i = 0; i < data_volum_.size(); i++) {
            data_volum_[i]->Destroy();
        }
        return -1;
    }

    return 0;
}

int VolumGroup::Destroy() {
    int ret = 0;

    for (size_t i = 0; i < data_volum_.size(); i++) {
        if (0 != data_volum_[i]->Destroy()) {
            ret = -1;
        }
    }

    return ret;
}

int VolumGroup::ExportEnv(std::map<std::string, std::string>& env) {
    env["baidu_galaxy_container_workspace_path"] = workspace_volum_->Description()->dest_path();
    env["baidu_galaxy_container_workspace_abstargetpath"] = workspace_volum_->TargetPath();
    env["baidu_galaxy_container_workspace_abssourcepath"] = workspace_volum_->SourcePath();
    env["baidu_galaxy_container_workspace_datavolum_size"] = boost::lexical_cast<std::string>(data_volum_.size());
    
    return 0;
}

boost::shared_ptr<google::protobuf::Message> VolumGroup::Report() {
    boost::shared_ptr<google::protobuf::Message> ret;
    return ret;
}

boost::shared_ptr<Volum> VolumGroup::Construct(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> dp) {
    assert(NULL != dp.get());
    boost::shared_ptr<Volum> volum = Volum::CreateVolum(dp);

    if (NULL == volum.get()) {
        return volum;
    }

    volum->SetDescription(dp);
    volum->SetContainerId(this->container_id_);

    if (0 != volum->Construct()) {
        volum.reset();
    }

    return volum;
}

// FIX: a single class
int VolumGroup::MountRootfs() {
    std::vector<std::string> vm;
    boost::algorithm::split(vm, FLAGS_mount_templat, boost::is_any_of(","));
    
    std::string container_path = workspace_volum_->TargetPath();
    for (size_t i = 0; i < vm.size(); i++) {
        if (vm[i].empty()) {
            continue;
        }
        boost::system::error_code ec;
        boost::filesystem::path path(container_path);
        path.append(vm[i]);
        
        if (!boost::filesystem::exists(path, ec) && !boost::filesystem::create_directories(path, ec)) {
            LOG(WARNING) << "create_directories failed: " << path.string() << ": " << ec.message();
            return -1;
        }
        
        if(boost::filesystem::is_directory(path, ec)) {
            LOG(INFO) << "create_directories sucessfully: " << path.string();
        }
        
        if ("/proc" == vm[i]) {
            baidu::galaxy::util::ErrorCode errc = MountProc(path.string());
            if (0 != errc.Code()) {
                LOG(WARNING) << "mount " << vm[i] << "for container " << container_id_ << " failed " << errc.Message();
                return -1;
            }
            LOG(INFO) << "mount successfully: " << vm[i] << " -> " << path.string();
        } else {
            baidu::galaxy::util::ErrorCode errc = MountDir(vm[i], path.string());
            if (0 != errc.Code()) {
                LOG(WARNING) << "mount " << vm[i] << "for container " << container_id_ << " failed " << errc.Message();
                return -1;
            }
            LOG(INFO) << "mount successfully: " << vm[i] << " -> " << path.string();
        }
    }
    return 0;
}

}
}
}
