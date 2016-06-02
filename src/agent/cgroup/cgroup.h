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
class CgroupMetrix;
};


namespace cgroup {
class FreezerSubsystem;
class Subsystem;
class SubsystemFactory;
class CgroupCollector;

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
    
    baidu::galaxy::util::ErrorCode Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix>& metrix); 
    boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> Statistics(); // call by container
private:
    std::vector<boost::shared_ptr<Subsystem> > subsystem_;
    boost::shared_ptr<FreezerSubsystem> freezer_;

    std::string container_id_;
    boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup_;
    const boost::shared_ptr<SubsystemFactory> factory_;
    boost::shared_ptr<CgroupCollector> collector_;
};
}
}
}
