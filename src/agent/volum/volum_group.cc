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
#include "util/path_tree.h"
#include "boost/lexical_cast/lexical_cast_old.hpp"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <iostream>

DECLARE_string(mount_templat);
DECLARE_string(mount_cgroups);
DECLARE_string(v2_prefix);

namespace baidu {
namespace galaxy {
namespace volum {
VolumGroup::VolumGroup() :
    gc_index_(-1) {
}

VolumGroup::~VolumGroup() {
}

void VolumGroup::SetGcIndex(int gc_index) {
    assert(gc_index >= 0);
    gc_index_ = gc_index;
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

baidu::galaxy::util::ErrorCode VolumGroup::Construct() {
    workspace_volum_ = Construct(this->ws_description_);

    if (NULL == workspace_volum_.get()) {
        return ERRORCODE(-1, "failed in constructing workspace volum");
    }

    baidu::galaxy::util::ErrorCode ec;

    for (size_t i = 0; i < dv_description_.size(); i++) {
        boost::shared_ptr<Volum> v = Construct(dv_description_[i]);

        if (v.get() == NULL) {
            ec = ERRORCODE(-1,
                    "construct volum(%s->%s) failed: ",
                    dv_description_[i]->source_path().c_str(),
                    dv_description_[i]->dest_path().c_str());
            break;
        }

        data_volum_.push_back(v);
    }

    if (data_volum_.size() != dv_description_.size()) {
        for (size_t i = 0; i < data_volum_.size(); i++) {
            baidu::galaxy::util::ErrorCode err = data_volum_[i]->Destroy();

            if (0 != err.Code()) {
                LOG(WARNING) << "faild in destroying volum for container "
                             << container_id_ << ": " << err.Message();
            }
        }

        return ec;
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode VolumGroup::Destroy() {
    baidu::galaxy::util::ErrorCode ec;

    for (size_t i = 0; i < data_volum_.size(); i++) {
        ec = data_volum_[i]->Destroy();

        if (0 != ec.Code()) {
            return ERRORCODE(-1,
                    "failed in destroying data volum: %s",
                    ec.Message().c_str());
        }
    }

    if (workspace_volum_.get() != NULL) {
        ec = workspace_volum_->Destroy();

        if (0 != ec.Code()) {
            return ERRORCODE(-1,
                    "failed in destroying workspace volum: %s",
                    ec.Message().c_str());
        }
    }

    // rm empty dir
    // /home/diskx/galaxy/container_id/
    for (size_t i = 0; i < data_volum_.size(); i++) {
        ec = data_volum_[i]->Gc();

        if (0 != ec.Code()) {
            return ERRORCODE(-1,
                    "failed in gc data volum: %s",
                    ec.Message().c_str());
        }
    }

    if (workspace_volum_.get() != NULL) {
        ec = workspace_volum_->Gc();

        if (0 != ec.Code()) {
            return ERRORCODE(-1,
                    "failed in gc workspace volum: %s",
                    ec.Message().c_str());
        }
    }

    // mv container root dir to gc_dir
    std::string container_root_path
        = baidu::galaxy::path::ContainerRootPath(container_id_);
    boost::system::error_code boost_ec;

    if (boost::filesystem::exists(container_root_path, boost_ec)) {
        std::string container_gc_root_path
            = baidu::galaxy::path::ContainerGcRootPath(container_id_, gc_index_);
        boost::filesystem::rename(container_root_path, container_gc_root_path, boost_ec);

        if (boost_ec.value() != 0) {
            return ERRORCODE(-1, "failed in renaming %s to %s: %s",
                    container_root_path.c_str(),
                    container_gc_root_path.c_str(),
                    boost_ec.message().c_str());
        }
    }

    return ERRORCODE_OK;
}


std::string VolumGroup::ContainerGcPath() {
    assert(gc_index_ > 0);
    assert(!container_id_.empty());
    return baidu::galaxy::path::ContainerGcRootPath(container_id_, gc_index_);
}


int VolumGroup::ExportEnv(std::map<std::string, std::string>& env) {
    env["baidu_galaxy_container_workspace_path"] = workspace_volum_->Description()->dest_path();
    env["baidu_galaxy_container_workspace_abstargetpath"] = workspace_volum_->TargetPath();
    env["baidu_galaxy_container_workspace_abssourcepath"] = workspace_volum_->SourcePath();
    return 0;
}

boost::shared_ptr<Volum> VolumGroup::Construct(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> dp) {
    boost::shared_ptr<Volum> volum = NewVolum(dp);

    if (NULL == volum.get()) {
        return volum;
    }

    baidu::galaxy::util::ErrorCode ec = volum->Construct();

    if (0 != ec.Code()) {
        LOG(WARNING) << "failed in constructing volum for container "
                     << container_id_ << ": " << ec.Message();
        volum.reset();
    }

    return volum;
}


boost::shared_ptr<Volum> VolumGroup::NewVolum(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> dp) {
    assert(NULL != dp.get());
    assert(gc_index_ >= 0);
    boost::shared_ptr<Volum> volum = Volum::CreateVolum(dp);

    if (NULL == volum.get()) {
        return volum;
    }

    volum->SetDescription(dp);
    volum->SetContainerId(this->container_id_);
    volum->SetGcIndex(gc_index_);
    volum->SetUser(user_);
    return volum;
}

const boost::shared_ptr<Volum> VolumGroup::WorkspaceVolum() const {
    return workspace_volum_;
}

const int VolumGroup::DataVolumsSize() const {
    return data_volum_.size();
}

const boost::shared_ptr<Volum> VolumGroup::DataVolum(int i) const {
    assert(i >= 0);
    assert(i < (int)data_volum_.size());
    return data_volum_[i];
}

int VolumGroup::MountDirs(const std::string& t, bool v2_support) {
    std::vector<std::string> vm;
    boost::split(vm, t, boost::is_any_of(","));
    std::string container_path = baidu::galaxy::path::ContainerRootPath(container_id_);

    for (size_t i = 0; i < vm.size(); i++) {
        if (vm[i].empty()) {
            continue;
        }

        boost::system::error_code ec;
        boost::filesystem::path path(container_path);
        path.append(vm[i]);
        if (!boost::filesystem::exists(path, ec)
                && !baidu::galaxy::file::create_directories(path, ec)) {
            std::cerr << "create_directories failed: " << path.string() << ": " << ec.message() << std::endl;
            return -1;
        }

        if (boost::filesystem::is_directory(path, ec)) {
            std::cout << "create_directories sucessfully: " << path.string() << std::endl;
        }

        boost::filesystem::path vm_path;
        if (v2_support) {
            vm_path = FLAGS_v2_prefix;
            vm_path.append(vm[i]);
        } else {
            vm_path = vm[i];
        }

        if ("/proc" == vm[i]) {
            baidu::galaxy::util::ErrorCode errc = MountProc(vm_path.string(), path.string());

            if (0 != errc.Code()) {
                std::cerr << "mount " << vm[i] << "for container "
                    << container_id_ << " failed " << errc.Message() << std::endl;
                return -1;
            }

            std::cout << "mount successfully: " << vm_path.string() << " -> " << path.string() << std::endl;
        } else {
            baidu::galaxy::util::ErrorCode errc = MountDir(vm_path.string(), path.string());

            if (0 != errc.Code()) {
                std::cerr << "mount " << vm[i] << "for container " << container_id_ << " failed " << errc.Message() << std::endl;
                return -1;
            }

            //LOG(INFO) << "mount successfully: " << vm[i] << " -> " << path.string();
            std::cout << "mount successfully: " << vm_path.string() << " -> " << path.string() << std::endl;
        }
    }

    return 0;

}

int VolumGroup::MountRootfs(bool v2_support) {
    int ret = MountDirs(FLAGS_mount_templat, v2_support);
    if (0 != ret) {
        return ret;
    }

    ret = MountDirs(FLAGS_mount_cgroups, false);
    if (0 != ret) {
        return ret;
    }

    if (v2_support) {
        ret = MountDirs("top", true);
    }
    return ret;
}

/*
// FIX: a single class
int VolumGroup::MountRootfs(bool v2_support) {
    v2_support = true;
    std::vector<std::string> vm;
    boost::split(vm, FLAGS_mount_templat, boost::is_any_of(","));
    if (v2_support) {
        vm.push_back("top");
    }

    std::string container_path = baidu::galaxy::path::ContainerRootPath(container_id_);

    for (size_t i = 0; i < vm.size(); i++) {
        if (vm[i].empty()) {
            continue;
        }

        boost::system::error_code ec;
        boost::filesystem::path path(container_path);
        path.append(vm[i]);


        if (!boost::filesystem::exists(path, ec)
                && !baidu::galaxy::file::create_directories(path, ec)) {
            std::cerr << "create_directories failed: " << path.string() << ": " << ec.message() << std::endl;
            return -1;
        }

        if (boost::filesystem::is_directory(path, ec)) {
            std::cout << "create_directories sucessfully: " << path.string() << std::endl;
        }


        boost::filesystem::path vm_path;
        if (v2_support) {
            vm_path = FLAGS_v2_prefix;
            vm_path.append(vm[i]);
        } else {
            vm_path = vm[i];
        }

        if ("/proc" == vm[i]) {
            baidu::galaxy::util::ErrorCode errc = MountProc(vm_path.string(), path.string());

            if (0 != errc.Code()) {
                std::cerr << "mount " << vm[i] << "for container "
                          << container_id_ << " failed " << errc.Message() << std::endl;
                return -1;
            }

            std::cout << "mount successfully: " << vm_path.string() << " -> " << path.string() << std::endl;
        } else {
            baidu::galaxy::util::ErrorCode errc = MountDir(vm_path.string(), path.string());

            if (0 != errc.Code()) {
                std::cerr << "mount " << vm[i] << "for container " << container_id_ << " failed " << errc.Message() << std::endl;
                return -1;
            }

            //LOG(INFO) << "mount successfully: " << vm[i] << " -> " << path.string();
            std::cout << "mount successfully: " << vm_path.string() << " -> " << path.string() << std::endl;
        }
    }

    return 0;
}*/

baidu::galaxy::util::ErrorCode VolumGroup::MountDir_(const std::string& source,
        const std::string& target) {
    boost::system::error_code ec;

    if (!boost::filesystem::exists(source, ec)
            || !boost::filesystem::is_directory(source, ec)) {
        return ERRORCODE(-1, "%s donot exist", source.c_str());
    }

    if (!boost::filesystem::exists(target, ec)
            && !baidu::galaxy::file::create_directories(target, ec)) {
        return ERRORCODE(-1, "%s does not exist, or create failed: %s", ec.message().c_str());
    }

    if (!boost::filesystem::is_directory(target, ec)) {
        return ERRORCODE(-1, "%s isnot directory", target.c_str());
    }

    baidu::galaxy::util::ErrorCode errc = MountDir(source, target);

    if (errc.Code() != 0) {
        return ERRORCODE(-1, "mount %s --> %s failed: %s",
                source.c_str(),
                target.c_str(),
                errc.Message().c_str());
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode VolumGroup::MountSharedVolum(const std::map<std::string, std::string>& sv) {
    std::map<std::string, std::string>::const_iterator iter = sv.begin();

    while (iter != sv.end()) {
        std::string source = iter->first;
        std::string container_path = baidu::galaxy::path::ContainerRootPath(container_id_);
        boost::filesystem::path path(container_path);
        path.append(iter->second);
        std::string target = path.string();
        baidu::galaxy::util::ErrorCode ec = MountDir_(source, target);

        if (ec.Code() != 0) {
            return ERRORCODE(-1, "mount shared volum failed: %s", ec.Message().c_str());
        }

        std::cout << "mount shared volum successfully: " << source << " --> " << target << std::endl;
        iter++;
    }

    return ERRORCODE_OK;
}

}
}
}
