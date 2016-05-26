// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "container.h"
#include "resource/resource_manager.h"
#include "container_stage.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include <map>
#include <string>

namespace baidu {
namespace galaxy {
namespace container {

class ContainerManager {
public:
    explicit ContainerManager(boost::shared_ptr<baidu::galaxy::resource::ResourceManager> resman);
    ~ContainerManager();

    int CreateContainer(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc);
    int ReleaseContainer(const ContainerId& id);
    void ListContainers(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> >& cis, bool fullinfo);

private:
    int CreateContainer_(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc);
    int ReleaseContainer_(const ContainerId& id);

    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::Container> > work_containers_;
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::Container> > gc_containers_;
    //boost::scoped_ptr<baidu::common::ThreadPool> gc_threadpool_;
    //boost::scoped_ptr<baidu::common::ThreadPool> check_read_threadpool_;
    boost::shared_ptr<baidu::galaxy::resource::ResourceManager> res_man_;
    boost::mutex mutex_;

    ContainerStage stage_;

    boost::unordered_map<ContainerId, boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> > contianer_info_;
};

} //namespace agent
} //namespace galaxy
} //namespace baidu
