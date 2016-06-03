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
    void SetWorkspaceVolum(const baidu::galaxy::proto::VolumRequired& ws_volum);
    void SetContainerId(const std::string& container_id);
    std::string Id() {
        return container_id_;
    }

    baidu::galaxy::util::ErrorCode Construct();
    baidu::galaxy::util::ErrorCode  Destroy();
    int ExportEnv(std::map<std::string, std::string>& env);
    int MountRootfs();
    
    const boost::shared_ptr<Volum> WorkspaceVolum() const;
    const int DataVolumsSize() const;
    const boost::shared_ptr<Volum> DataVolum(int i) const;
private:
    boost::shared_ptr<Volum> Construct(boost::shared_ptr<baidu::galaxy::proto::VolumRequired> volum);

    boost::shared_ptr<baidu::galaxy::proto::VolumRequired> ws_description_;
    std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumRequired> > dv_description_;

    std::vector<boost::shared_ptr<Volum> > data_volum_;
    boost::shared_ptr<Volum> workspace_volum_;

    std::string container_id_;
    int gc_index_;
    std::string user_;

};
}
}
}
