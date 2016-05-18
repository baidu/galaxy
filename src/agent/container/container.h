// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once
#include "protocol/galaxy.pb.h"
#include "container_status.h"

#include <sys/types.h>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <string>
#include <vector>
#include <map>

namespace baidu {
namespace galaxy {
namespace cgroup {
class Cgroup;
}

namespace volum {
class VolumGroup;
}

namespace container {
class Process;
class Container {
public:
    Container(const std::string& id, const baidu::galaxy::proto::ContainerDescription& desc);
    ~Container();

    void AddEnv(const std::string& key, const std::string& value);
    void AddEnv(const std::map<std::string, std::string>& env);

    std::string Id() const;
    int Construct();
    int Destroy();

    int Tasks(std::vector<pid_t>& pids);
    int Pids(std::vector<pid_t>& pids);
    boost::shared_ptr<google::protobuf::Message> Report();
    baidu::galaxy::proto::ContainerStatus Status();

private:
    int Construct_();
    int Destroy_();

    int RunRoutine(void*);
    void ExportEnv(std::map<std::string, std::string>& env);
    void ExportEnv();
    bool Alive();

    const baidu::galaxy::proto::ContainerDescription desc_;
    std::vector<boost::shared_ptr<baidu::galaxy::cgroup::Cgroup> > cgroup_;
    boost::shared_ptr<baidu::galaxy::volum::VolumGroup> volum_group_;
    boost::shared_ptr<Process> process_;
    std::string id_;
    baidu::galaxy::container::ContainerStatus status_;

};

} //namespace container
} //namespace galaxy
} //namespace baidu
