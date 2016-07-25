// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "volum_container.h"
#include "glog/logging.h"
#include "protocol/galaxy.pb.h"
#include "util/path_tree.h"
#include "volum/volum.h"


#include "timer.h"

namespace baidu {
    namespace galaxy {
        namespace container {

            VolumContainer::VolumContainer(const ContainerId& id, 
                        const baidu::galaxy::proto::ContainerDescription& desc) :
            IContainer(id, desc),
            status_(id.SubId()),
            volum_group_(new baidu::galaxy::volum::VolumGroup()),
            created_time_(0L),
            destroy_time_(0L) {
            }

            VolumContainer::~VolumContainer() {
            }

            baidu::galaxy::util::ErrorCode VolumContainer::Construct() {
                baidu::galaxy::util::ErrorCode ec = status_.EnterAllocating();
                if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
                    LOG(WARNING) << ec.Message();
                    return ERRORCODE_OK;
                }

                if (ec.Code() != baidu::galaxy::util::kErrorOk) {
                    LOG(WARNING) << "construct failed " << id_.CompactId() << ": " << ec.Message();
                    return ERRORCODE(-1, "state machine error");
                }

                created_time_ = baidu::common::timer::get_micros();
                if (0 != ConstructVolumGroup()) {
                    LOG(WARNING) << id_.CompactId() << " construct volum group failed: ";
                    ec = status_.EnterError();
                    assert(ec.Code() == baidu::galaxy::util::kErrorOk);
                    return ERRORCODE(-1, "construct volum group failed");
                } else {
                    LOG(INFO) << id_.CompactId() << " construct volum group successfully: ";
                    ec = status_.EnterReady();
                    assert(ec.Code() == baidu::galaxy::util::kErrorOk);
                }
                return ec;
            }

            int VolumContainer::ConstructVolumGroup() {
                assert(created_time_ > 0);
                volum_group_->SetContainerId(id_.SubId());
                volum_group_->SetWorkspaceVolum(desc_.workspace_volum());
                volum_group_->SetGcIndex(created_time_ / 1000000);
                volum_group_->SetOwner(desc_.run_user());

                for (int i = 0; i < desc_.data_volums_size(); i++) {
                    volum_group_->AddDataVolum(desc_.data_volums(i));
                }

                baidu::galaxy::util::ErrorCode ec = volum_group_->Construct();
                if (0 != ec.Code()) {
                    LOG(WARNING) << "failed in constructing volum group for container " << id_.CompactId()
                            << ", reason is: " << ec.Message();
                    return -1;
                }
                return 0;
            }

            baidu::galaxy::util::ErrorCode VolumContainer::Destroy() {
                baidu::galaxy::util::ErrorCode ec = status_.EnterDestroying();

                if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
                    LOG(WARNING) << "container  " << id_.CompactId() << " is in kContainerDestroying status: " << ec.Message();
                    ERRORCODE(-1, "repeated destroy");
                }

                if (ec.Code() != baidu::galaxy::util::kErrorOk) {
                    LOG(WARNING) << "destroy container " << id_.CompactId() << " failed: " << ec.Message();
                    return ERRORCODE(-1, "status machine");
                }

                ec = volum_group_->Destroy();
                if (0 != ec.Code()) {
                    LOG(WARNING) << "failed in destroying volum group in container "
                            << id_.CompactId()
                            << " " << ec.Message();
                    ec = status_.EnterError();
                    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
                        LOG(FATAL) << id_.CompactId() << " status error: " << ec.Message();
                    }
                    return ERRORCODE(-1, "volum");
                } else {
                    ec = status_.EnterTerminated();
                    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
                        LOG(FATAL) << id_.CompactId() << " status error: " << ec.Message();
                    }
                    return ERRORCODE_OK;
                }
                LOG(INFO) << "voulum container " << id_.CompactId() << " suceed in destroy volum";
                return ERRORCODE_OK;
            }

            baidu::galaxy::util::ErrorCode VolumContainer::Reload(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta) {
                assert(!id_.Empty());
                created_time_ = meta->created_time();
                status_.EnterAllocating();

                int ret = ConstructVolumGroup();
                if (0 != ret) {
                    status_.EnterError();
                    return ERRORCODE(-1, "failed in constructing volum group");
                }
                LOG(INFO) << "succeed in constructing volum group for volum container " << id_.CompactId();
                status_.EnterReady();
                return ERRORCODE_OK;
            }

            boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> VolumContainer::ContainerMeta() {
                boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> ret(new baidu::galaxy::proto::ContainerMeta());
                ret->set_container_id(id_.SubId());
                ret->set_group_id(id_.GroupId());
                ret->set_created_time(created_time_);
                ret->set_pid(-1);
                ret->mutable_container()->CopyFrom(desc_);
                ret->set_destroy_time(destroy_time_);
                return ret;
            }

            boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> VolumContainer::ContainerInfo(bool full_info) {
                boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ret(new baidu::galaxy::proto::ContainerInfo());
                ret->set_id(id_.SubId());
                ret->set_group_id(id_.GroupId());
                ret->set_created_time(0);
                ret->set_status(status_.Status());
                ret->set_cpu_used(0);
                ret->set_memory_used(0);

                baidu::galaxy::proto::ContainerDescription* cd = ret->mutable_container_desc();
                if (full_info) {
                    cd->CopyFrom(desc_);
                } else {
                    cd->set_version(desc_.version());
                }


                boost::shared_ptr<baidu::galaxy::volum::Volum> wv = volum_group_->WorkspaceVolum();
                if (NULL != wv) {
                    baidu::galaxy::proto::Volum* vr = ret->add_volum_used();
                    vr->set_used_size(wv->Used());
                    vr->set_path(wv->Description()->dest_path());
                }

                for (int i = 0; i < volum_group_->DataVolumsSize(); i++) {
                    baidu::galaxy::proto::Volum* vr = ret->add_volum_used();
                    boost::shared_ptr<baidu::galaxy::volum::Volum> dv = volum_group_->DataVolum(i);
                    vr->set_used_size(dv->Used());
                    vr->set_path(dv->Description()->dest_path());
                }

                return ret;
            }

            boost::shared_ptr<baidu::galaxy::proto::ContainerMetrix> VolumContainer::ContainerMetrix() {
                boost::shared_ptr<baidu::galaxy::proto::ContainerMetrix> ret;
                return ret;
            }

            boost::shared_ptr<ContainerProperty> VolumContainer::Property() {
                boost::shared_ptr<ContainerProperty> property(new ContainerProperty);
                property->container_id_ = id_.SubId();
                property->group_id_ = id_.GroupId();
                property->created_time_ = created_time_;
                property->pid_ = -1;

                const boost::shared_ptr<baidu::galaxy::volum::Volum> wv = volum_group_->WorkspaceVolum();
                property->workspace_volum_.container_abs_path = wv->TargetPath();
                property->workspace_volum_.phy_source_path = wv->SourcePath();
                property->workspace_volum_.container_rel_path = wv->Description()->dest_path();
                property->workspace_volum_.phy_gc_path = wv->SourceGcPath();
                property->workspace_volum_.medium = baidu::galaxy::proto::VolumMedium_Name(wv->Description()->medium());
                property->workspace_volum_.quota = wv->Description()->size();
                property->workspace_volum_.phy_gc_root_path = wv->SourceGcRootPath();

                // 
                for (int i = 0; i < volum_group_->DataVolumsSize(); i++) {
                    ContainerProperty::Volum cv;
                    const boost::shared_ptr<baidu::galaxy::volum::Volum> v = volum_group_->DataVolum(i);
                    cv.container_abs_path = v->TargetPath();
                    cv.phy_source_path = v->SourcePath();
                    cv.container_rel_path = v->Description()->dest_path();
                    cv.phy_gc_path = v->SourceGcPath();
                    cv.phy_gc_root_path = v->SourceGcRootPath();
                    cv.medium = baidu::galaxy::proto::VolumMedium_Name(v->Description()->medium());
                    cv.quota = v->Description()->size();
                    property->data_volums_.push_back(cv);
                }
                return property;
            }

            std::string VolumContainer::ContainerGcPath() {
                return volum_group_->ContainerGcPath();
            }

        }
    }
}
