
// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "container_manager.h"
#include "util/path_tree.h"
#include "thread.h"
#include "util/output_stream_file.h"

#include "boost/bind.hpp"
#include <glog/logging.h>

namespace baidu {
namespace galaxy {
namespace container {

ContainerManager::ContainerManager(boost::shared_ptr<baidu::galaxy::resource::ResourceManager> resman) :
    res_man_(resman),
    running_(false),
    serializer_(new Serializer()),
    container_gc_(new ContainerGc()) {
    assert(NULL != resman);
}

ContainerManager::~ContainerManager() {
    running_ = false;
}

void ContainerManager::Setup() {
    std::string path = baidu::galaxy::path::RootPath();
    path += "/data";
    baidu::galaxy::util::ErrorCode ec = serializer_->Setup(path);

    if (ec.Code() != 0) {
        LOG(FATAL) << "set up serilzer failed, path is" << path
                   << " reason is: " << ec.Message();
        exit(-1);
    }

    LOG(INFO) << "succeed in setting up serialize db: " << path;
    int ret = Reload();

    if (0 != ret) {
        LOG(WARNING) << "failed in recovering container from meta, agent will exit";
        exit(-1);
    } else {
        LOG(INFO) << "succeed in recovering container from meta";
    }

    ec = container_gc_->Reload();

    if (ec.Code() != 0) {
        LOG(WARNING) << "reload gc failed: " << ec.Message();
        exit(-1);
    }

    LOG(INFO) << "succeed in loading gc meta";
    ec = container_gc_->Setup();

    if (ec.Code() != 0) {
        LOG(WARNING) << "set up container gc failed: " << ec.Message();
        exit(-1);
    }

    LOG(INFO) << "setup container gc successful";
    running_ = true;
    this->keep_alive_thread_.Start(boost::bind(&ContainerManager::KeepAliveRoutine, this));
}

void ContainerManager::KeepAliveRoutine() {
    while (running_) {
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::IContainer> >::iterator iter = work_containers_.begin();

            while (iter != work_containers_.end()) {
                iter->second->KeepAlive();
                iter++;
            }
        }
        VLOG(10) << "keep alive routine";
        ::sleep(1);
    }
}

baidu::galaxy::util::ErrorCode ContainerManager::CreateContainer(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc) {
    // enter creating stage, every time only one thread does creating
    ScopedCreatingStage lock_stage(stage_, id.SubId());
    baidu::galaxy::util::ErrorCode ec = lock_stage.GetLastError();

    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(WARNING) << "container " << id.CompactId() << " has been in creating stage already: " << ec.Message();
        return ERRORCODE_OK;
    }

    LOG(INFO) << "create container : " << id.CompactId();

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << "container " << id.CompactId() << " enter create stage failed: " << ec.Message();
        return ERRORCODE(-1, "stage err");
    }

    // judge container exist or not
    {
        boost::mutex::scoped_lock lock(mutex_);
        std::map<ContainerId, boost::shared_ptr<IContainer> >::const_iterator iter
            = work_containers_.find(id);

        if (work_containers_.end() != iter) {
            LOG(INFO) << "container " << id.CompactId() << " has already been created";
            return ERRORCODE_OK;
        }
    }

    // allcate resource
    ec = res_man_->Allocate(desc);

    if (0 != ec.Code()) {
        LOG(WARNING) << "fail in allocating resource for container "
                     << id.CompactId() << ", detail reason is: "
                     << ec.Message();
        return ERRORCODE(-1, "resource");
    } else {
        LOG(INFO) << "succeed in allocating resource for " << id.CompactId();
    }

    // create container
    baidu::galaxy::util::ErrorCode ret = CreateContainer_(id, desc);

    if (0 != ret.Code()) {
        LOG(WARNING) <<  id.CompactId() << " create container failed " << ret.Message();
        ec = res_man_->Release(desc);

        if (ec.Code() != 0) {
            LOG(FATAL) << "failed in releasing resource for container "
                       << id.CompactId() << ", detail reason is: "
                       << ec.Message();
        }
    } else {
        LOG(INFO) << "success in creating container " << id.CompactId();
    }

    return ret;
}

baidu::galaxy::util::ErrorCode ContainerManager::ReleaseContainer(const ContainerId& id) {
    // enter destroying stage, only one thread do releasing at a moment
    ScopedDestroyingStage lock_stage(stage_, id.SubId());
    baidu::galaxy::util::ErrorCode ec = lock_stage.GetLastError();

    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(INFO) << "container " << id.CompactId()
                  << " has been in destroying stage: " << ec.Message();
        return ERRORCODE_OK;
    }

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << "failed in entering destroying stage for container " << id.CompactId()
                     << ", reason: " << ec.Message();
        return ERRORCODE(-1, "stage err");
    }

    LOG(INFO) << "do release container " << id.CompactId();
    // judge container existence
    boost::shared_ptr<baidu::galaxy::container::IContainer> ctn;
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::IContainer> >::iterator iter;
    {
        boost::mutex::scoped_lock lock(mutex_);
        iter = work_containers_.find(id);

        if (work_containers_.end() == iter) {
            LOG(WARNING) << "container " << id.CompactId() << " do not exist";
            return ERRORCODE_OK;
        }

        ctn = iter->second;
    }
    // wait appworker exist
    assert(NULL != ctn.get());
    // do releasing
    // Fix me: should delete meta first, what happend when delete meta failed???
    baidu::galaxy::util::ErrorCode ret = iter->second->Destroy();

    if (0 == ret.Code()) {
        ec = res_man_->Release(iter->second->Description());

        if (0 != ec.Code()) {
            LOG(FATAL) << "failed in releasing resource for container " << id.CompactId()
                       << " reason is: " << ec.Message();
            exit(-1);
        } else {
            LOG(INFO) << "succeed in releasing resource for container " << id.CompactId();
        }

        {
            boost::mutex::scoped_lock lock(mutex_);
            // fix me
            work_containers_.erase(iter);
        }

        ec = serializer_->DeleteWork(id.GroupId(), id.SubId());

        if (ec.Code() != 0) {
            LOG(WARNING) << "delete key failed, key is: " << id.CompactId()
                         << " reason is: " << ec.Message();
            ret = ERRORCODE(-1, "serialize");
        } else {
            LOG(INFO) << "succeed in deleting container meta for container " << id.CompactId();
        }

        ec = container_gc_->Gc(ctn->ContainerGcPath());

        // ec is ok always
        if (ec.Code() != 0) {
            LOG(WARNING) << id.CompactId() << " gc failed: " << ec.Message();
        }
    }

    return ret;
}

baidu::galaxy::util::ErrorCode ContainerManager::CreateContainer_(const ContainerId& id,
        const baidu::galaxy::proto::ContainerDescription& desc) {
    VLOG(10)
        << "container manager create container, volum_view:  "
        << baidu::galaxy::proto::VolumViewType_Name(desc.volum_view());
    boost::shared_ptr<baidu::galaxy::container::IContainer> container
        = IContainer::NewContainer(id, desc);
    // dependent volum
    std::map<std::string, std::string> depend_volums;
    baidu::galaxy::util::ErrorCode err = DependentVolums(desc, depend_volums);

    if (err.Code() != 0) {
        LOG(WARNING) << id.CompactId() << " get dependent volums failed: " << err.Message();
        return ERRORCODE(-1, "Dependent volums");
    }

    // log
    std::map<std::string, std::string>::const_iterator iter = depend_volums.begin();

    while (iter != depend_volums.end()) {
        LOG(INFO) << id.CompactId() << " depend volum: " << iter->first << "->" << iter->second;
        iter++;
    }

    container->SetDependentVolums(depend_volums);
    err = container->Construct();

    if (0 != err.Code()) {
        LOG(WARNING) << "fail in constructing container " << id.CompactId() << " " << err.Message();
        container->Destroy();
        return err;
    }

    baidu::galaxy::util::ErrorCode ec = serializer_->SerializeWork(container->ContainerMeta());

    if (ec.Code() != 0) {
        LOG(WARNING) << "fail in serializing container meta " << id.CompactId();
        return ERRORCODE(-1, "serialize");
    }

    LOG(INFO) << "succeed in serializing container meta for cantainer " << id.CompactId();
    {
        boost::mutex::scoped_lock lock(mutex_);
        work_containers_[id] = container;
    }
    DumpProperty(container);
    LOG(INFO) << "succeed in constructing container " << id.CompactId();
    return ERRORCODE_OK;
}


void ContainerManager::DumpProperty(boost::shared_ptr<IContainer> container) {
    boost::shared_ptr<ContainerProperty> property = container->Property();
    std::string path = baidu::galaxy::path::ContainerPropertyPath(container->Id().SubId());
    baidu::galaxy::file::OutputStreamFile of(path, "w");
    baidu::galaxy::util::ErrorCode ec = of.GetLastError();

    if (ec.Code() != 0) {
        LOG(WARNING) << container->Id().CompactId() <<  " save property failed: " << ec.Message();
    } else {
        std::string str = property->ToString();
        size_t size = str.size();
        ec = of.Write(str.c_str(), size);

        if (ec.Code() != 0) {
            LOG(WARNING) << container->Id().CompactId() <<  " save property failed: " << ec.Message();
        }
    }

    boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta = container->ContainerMeta();
    path = baidu::galaxy::path::ContainerMetaPath(container->Id().SubId());
    baidu::galaxy::file::OutputStreamFile of2(path, "w");
    ec = of2.GetLastError();

    if (ec.Code() != 0) {
        LOG(WARNING) << container->Id().CompactId() <<  " save property failed: " << ec.Message();
    } else {
        std::string str = meta->DebugString();
        size_t size = str.size();
        ec = of2.Write(str.c_str(), size);

        if (ec.Code() != 0) {
            LOG(WARNING) << container->Id().CompactId() <<  " save property failed: " << ec.Message();
        }
    }
}

void ContainerManager::ListContainers(std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> >& cis, bool fullinfo) {
    boost::mutex::scoped_lock lock(mutex_);
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::IContainer> >::iterator iter =  work_containers_.begin();

    while (iter != work_containers_.end()) {
        boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ci = iter->second->ContainerInfo(fullinfo);
        cis.push_back(ci);
        iter++;
    }
}


int ContainerManager::Reload() {
    std::vector<boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> > metas;
    baidu::galaxy::util::ErrorCode ec = serializer_->LoadWork(metas);

    if (ec.Code() != 0) {
        LOG(WARNING) << "load from db failed: " << ec.Message();
        return -1;
    }

    for (size_t i = 0; i < metas.size(); i++) {
        ec = res_man_->Allocate(metas[i]->container());
        ContainerId id(metas[i]->group_id(), metas[i]->container_id());

        if (ec.Code() != 0) {
            LOG(WARNING) << "allocat failed for container " << id.CompactId()
                         << " reason is:" << ec.Message();
            return -1;
        }

        boost::shared_ptr<IContainer> container = IContainer::NewContainer(id, metas[i]->container());
        ec = container->Reload(metas[i]);

        if (0 != ec.Code()) {
            LOG(WARNING) << id.CompactId() << " failed in reaload container " << ec.Message();
            return -1;
        }

        work_containers_[id] = container;
    }

    return 0;
}


baidu::galaxy::util::ErrorCode ContainerManager::CheckDescription(const baidu::galaxy::proto::ContainerDescription& desc) {
    if ((desc.has_container_type()
            && desc.container_type() == baidu::galaxy::proto::kVolumContainer)
            || desc.volum_containers_size() == 0) {
        return ERRORCODE_OK;
    }

    int size = desc.volum_containers_size();
    boost::mutex::scoped_lock lock(mutex_);

    for (int i = 0; i < size; i++) {
        //std::string id = desc.volum_containers(i);
        const ContainerId id;
        std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::IContainer> >::iterator iter = work_containers_.find(id);

        if (work_containers_.end() == iter) {
            return ERRORCODE(-1, "%s donot exist", id.CompactId().c_str());
        }
    }

    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode ContainerManager::DependentVolums(const baidu::galaxy::proto::ContainerDescription& desc,
        std::map<std::string, std::string>& dv) {
    if (desc.volum_containers_size() > 0) {
        std::map<std::string, std::string> check_map;

        for (int i = 0; i < desc.volum_containers_size(); i++) {
            ContainerId id("", desc.volum_containers(i));
            baidu::galaxy::util::ErrorCode ec = DependentVolums(id, dv, check_map);

            if (ec.Code() != 0) {
                LOG(WARNING) << "retrieve dependent volum failed: " << ec.Message();
                return ERRORCODE(-1, "%s", ec.Message().c_str());
            }
        }
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode ContainerManager::DependentVolums(const ContainerId& id,
        std::map<std::string, std::string>& volums,
        std::map<std::string, std::string>& check_volums) {
    boost::mutex::scoped_lock lock(mutex_);
    std::map<ContainerId, boost::shared_ptr<baidu::galaxy::container::IContainer> >::iterator iter = work_containers_.find(id);

    if (work_containers_.end() == iter) {
        return ERRORCODE(-1, "donot exist");
    }

    boost::shared_ptr<ContainerProperty> property = iter->second->Property();

    if (NULL == property.get()) {
        return ERRORCODE(-1, "get property failed");
    }

    std::string source = property->workspace_volum_.phy_source_path;
    std::string target = property->workspace_volum_.container_rel_path;
    std::map<std::string, std::string>::const_iterator citer = check_volums.find(target);

    if (citer != check_volums.end()) {
        return ERRORCODE(-1,
                "shared volum(%s) confilict",
                target.c_str());
    }

    check_volums[target] = source;
    volums[source] = target;

    for (size_t i = 0; i < property->data_volums_.size(); i++) {
        source = property->data_volums_[i].phy_source_path;
        target = property->data_volums_[i].container_rel_path;
        citer = check_volums.find(target);

        if (citer != check_volums.end()) {
            return ERRORCODE(-1,
                    "shared volum(%s\t%s\t%s) confilict",
                    citer->first.c_str(),
                    source.c_str(),
                    target.c_str());
        }

        check_volums[target] = source;
        volums[source] = target;
    }

    return ERRORCODE_OK;
}


}
}
}
