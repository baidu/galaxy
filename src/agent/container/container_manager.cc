
// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "container_manager.h"
#include <glog/logging.h>

namespace baidu {
namespace galaxy {
namespace container {

ContainerManager::ContainerManager(boost::shared_ptr<baidu::galaxy::resource::ResourceManager> resman) :
    res_man_(resman)
{
    assert(NULL != resman);
}

ContainerManager::~ContainerManager()
{
}

int ContainerManager::CreateContainer(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc)
{
    baidu::galaxy::util::ErrorCode ec = stage_.EnterCreatingStage(id.SubId());
    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(WARNING) << "container " << id.CompactId() << " has been in creating stage already: " << ec.Message();
        return 0;
    }

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << "container " << id.CompactId() << " enter create stage failed: " << ec.Message();
        return -1;
    }

    int ret = CreateContainer_(id, desc);
    stage_.LeaveCreatingStage(id.SubId());
    return ret;
}

int ContainerManager::ReleaseContainer(const ContainerId& id)
{
    baidu::galaxy::util::ErrorCode ec = stage_.EnterDestroyingStage(id.SubId());
    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(WARNING) << ec.Message();
        return 0;
    }

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << ec.Message();
        return -1;
    }
    int ret = ReleaseContainer_(id);
    stage_.LeaveDestroyingStage(id.SubId());
    return ret;
}

int ContainerManager::CreateContainer_(const ContainerId& id,
        const baidu::galaxy::proto::ContainerDescription& desc)
{
    {
        boost::mutex::scoped_lock lock(mutex_);
        std::map<ContainerId, boost::shared_ptr<Container> >::const_iterator iter
            = work_containers_.find(id);

        if (work_containers_.end() != iter) {
            LOG(WARNING) << "container " << id.CompactId() << " has already been created";
            return 0;
        }
    }

    baidu::galaxy::util::ErrorCode ec = res_man_->Allocate(desc);
    if (0 != ec.Code()) {
        LOG(WARNING) << "fail in allocating resource for container "
                     << id.CompactId() << ", detail reason is: "
                     << ec.Message();
        return -1;
    }

    boost::shared_ptr<baidu::galaxy::container::Container>
    container(new baidu::galaxy::container::Container(id, desc));

    if (0 != container->Construct()) {
        baidu::galaxy::util::ErrorCode ec = res_man_->Release(desc);
        if (ec.Code() != 0) {
            LOG(FATAL) << "failed in releasing resource for container "
                       << id.CompactId() << ", detail reason is: "
                       << ec.Message();
        }

        LOG(WARNING) << "fail in constructing container " << id.CompactId();
        return -1;
    }

    {
        boost::mutex::scoped_lock lock(mutex_);
        work_containers_[id] = container;
        LOG(INFO) << "success in constructing container " << id.CompactId();
    }

    return 0;
}

// FIXME release resource
int ContainerManager::ReleaseContainer_(const ContainerId& id)
{
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::Container> >::iterator iter;
    {
        boost::mutex::scoped_lock lock(mutex_);
        iter = work_containers_.find(id);

        if (work_containers_.end() == iter) {
            LOG(WARNING) << "container " << id.CompactId() << "do not exist";
            return 0;
        }
    }
    // fix me run in threadpool
    int ret;
    if (0 == (ret = iter->second->Destroy())) {
        baidu::galaxy::util::ErrorCode rc = res_man_->Release(iter->second->Description());
        if (0 != rc.Code()) {
            LOG(FATAL) << "failed in releasing resource for container " << id.CompactId()
                       << " reason is: " << rc.Message();
        }
        work_containers_.erase(iter);
        gc_containers_[id] = iter->second;
        LOG(INFO) << "container " << id.CompactId() << " is destroyed successfully and is moved to gc queue";
    } else {
        LOG(WARNING) << "destroy container " << id.CompactId() << " failed";
    }

    return ret;
}

void ContainerManager::ListContainers(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> >& cis, bool fullinfo)
{
    boost::mutex::scoped_lock lock(mutex_);
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::Container> >::iterator iter =  work_containers_.begin();
    while (iter != work_containers_.end()) {
        boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ci = iter->second->ContainerInfo(fullinfo);
        cis.push_back(ci);
        iter++;
    }
}

}
}
}
