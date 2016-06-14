// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "container.h"
#include "resource/resource_manager.h"
#include "container_stage.h"
#include "serializer.h"

#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"

#include "thread.h"
#include "container_gc.h"

#include <map>
#include <string>

namespace baidu {
namespace galaxy {
namespace container {

class ContainerManager {
public:
    explicit ContainerManager(boost::shared_ptr<baidu::galaxy::resource::ResourceManager> resman);
    ~ContainerManager();
    
    void Setup();
    baidu::galaxy::util::ErrorCode CreateContainer(const ContainerId& id, 
                const baidu::galaxy::proto::ContainerDescription& desc);

    baidu::galaxy::util::ErrorCode ReleaseContainer(const ContainerId& id);
    void ListContainers(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> >& cis, bool fullinfo);

private:
    baidu::galaxy::util::ErrorCode CreateContainer_(const ContainerId& id, 
                const baidu::galaxy::proto::ContainerDescription& desc);

    void KeepAliveRoutine();
    int Reload();
    void DumpProperty(boost::shared_ptr<Container> container);
    void GcThreadRoutine();

    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::Container> > work_containers_;
    //boost::scoped_ptr<baidu::common::ThreadPool> check_read_threadpool_;
    boost::shared_ptr<baidu::galaxy::resource::ResourceManager> res_man_;
    boost::mutex mutex_;

    ContainerStage stage_;

    boost::unordered_map<ContainerId, boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> > contianer_info_;
    baidu::common::Thread keep_alive_thread_;
    bool running_;

    boost::shared_ptr<Serializer> serializer_;
    boost::shared_ptr<ContainerGc> container_gc_;
};

} //namespace agent
} //namespace galaxy
} //namespace baidu
