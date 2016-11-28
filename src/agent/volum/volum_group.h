// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include <google/protobuf/message.h>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>

namespace baidu {
namespace galaxy {
namespace proto {
class VolumRequired;
}

namespace volum {

class Volum;

class VolumGroup {
public:
    VolumGroup();
    ~VolumGroup();

    void SetGcIndex(int gc_index);
    void SetOwner(const std::string& user) {
        user_ = user;
    }

    void AddDataVolum(const baidu::galaxy::proto::VolumRequired& data_volum);
    void AddOriginVolum(const baidu::galaxy::proto::VolumRequired& origin_volum);
    void SetWorkspaceVolum(const baidu::galaxy::proto::VolumRequired& ws_volum);
    void SetContainerId(const std::string& container_id);
    std::string Id() {
        return container_id_;
    }

    baidu::galaxy::util::ErrorCode Construct();
    baidu::galaxy::util::ErrorCode  Destroy();

    int ExportEnv(std::map<std::string, std::string>& env);
    int MountRootfs(bool vs_support);
    baidu::galaxy::util::ErrorCode MountSharedVolum(const std::map<std::string, std::string>& sv);
    const boost::shared_ptr<Volum> WorkspaceVolum() const;
    const int DataVolumsSize() const;
    const boost::shared_ptr<Volum> DataVolum(int i) const;
    std::string ContainerGcPath();
private:
    baidu::galaxy::util::ErrorCode MountDir_(const std::string& source, const std::string& target);
    boost::shared_ptr<Volum> Construct(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> volum);
    boost::shared_ptr<Volum> NewVolum(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> volum);

    int MountCgroups(const std::string& cg);
    int MountDirs(const std::string& t, bool v2_support);

    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> ws_description_;
    std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumRequired> > dv_description_;
    std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumRequired> > ov_description_;

    std::vector<boost::shared_ptr<Volum> > data_volum_;
    std::vector<boost::shared_ptr<Volum> > origin_volum_;
    boost::shared_ptr<Volum> workspace_volum_;

    std::string container_id_;
    int gc_index_;
    std::string user_;
};

}
}
}
