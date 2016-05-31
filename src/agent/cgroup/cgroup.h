// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <string>
#include <map>

#include "util/error_code.h"

namespace baidu {
namespace galaxy {
namespace proto {
class Cgroup;
};
namespace cgroup {
class FreezerSubsystem;
class Subsystem;
class SubsystemFactory;
class Metrix;

class Cgroup {
public:
    Cgroup(const boost::shared_ptr<SubsystemFactory> factory);
    ~Cgroup();
    void SetContainerId(const std::string& container_id);
    void SetDescrition(boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup);

    baidu::galaxy::util::ErrorCode Construct();
    baidu::galaxy::util::ErrorCode Destroy();
    void ExportEnv(std::map<std::string, std::string>& evn);
    std::string Id();

    baidu::galaxy::util::ErrorCode Statistics(Metrix& matrix);

private:
    std::vector<boost::shared_ptr<Subsystem> > subsystem_;
    boost::shared_ptr<FreezerSubsystem> freezer_;

    std::string container_id_;
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup_;
    const boost::shared_ptr<SubsystemFactory> factory_;
};
}
}
}
