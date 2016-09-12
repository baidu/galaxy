// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once
#include "icontainer.h"
#include "container_status.h"
#include "timer.h"

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

class Container : public IContainer {
public:
    Container(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc);
    ~Container();

    const ContainerId& Id() const;
    baidu::galaxy::util::ErrorCode Construct();
    baidu::galaxy::util::ErrorCode Destroy();
    baidu::galaxy::util::ErrorCode Reload(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta);
    const baidu::galaxy::proto::ContainerDescription& Description();
    boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ContainerInfo(bool full_info);
    boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> ContainerMeta();
    boost::shared_ptr<baidu::galaxy::proto::ContainerMetrix> ContainerMetrix();
    boost::shared_ptr<ContainerProperty> Property();
    std::string ContainerGcPath();
    void KeepAlive();

private:
    baidu::galaxy::util::ErrorCode Construct_();
    baidu::galaxy::util::ErrorCode Destroy_();

    bool Alive();

    // container will be killed after rel_sec seconds
    void SetExpiredTimeIfAbsent(int32_t rel_sec);
    bool Expired();
    bool TryKill();

    int ConstructCgroup();
    int ConstructVolumGroup();
    int ConstructProcess();

    int RunRoutine(void*);
    void ExportEnv(std::map<std::string, std::string>& env);
    void ExportEnv();

    std::vector<boost::shared_ptr<baidu::galaxy::cgroup::Cgroup> > cgroup_;
    boost::shared_ptr<baidu::galaxy::volum::VolumGroup> volum_group_;
    boost::shared_ptr<Process> process_;
    baidu::galaxy::container::ContainerStatus status_;
    int64_t created_time_;
    int64_t destroy_time_;
    int64_t force_kill_time_;

};

} //namespace container
} //namespace galaxy
} //namespace baidu
