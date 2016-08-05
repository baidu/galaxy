// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icontainer.h"
#include "container.h"
#include "volum_container.h"

namespace baidu {
    namespace galaxy {
        namespace container {
            IContainer::IContainer(const ContainerId& id,
                        const baidu::galaxy::proto::ContainerDescription& desc) :
                id_(id),
                desc_(desc) {}

 
            IContainer::~IContainer() {}

            boost::shared_ptr<IContainer> IContainer::NewContainer(const ContainerId& id,
                        const baidu::galaxy::proto::ContainerDescription& desc) {
                boost::shared_ptr<IContainer> ret;
                if (desc.has_container_type() 
                            && baidu::galaxy::proto::kVolumContainer == desc.container_type()) {
                    ret.reset(new VolumContainer(id, desc));
                } else {
                    ret.reset(new Container(id, desc));
                }
                return ret;
            }


            const ContainerId& IContainer::Id() const {
                return id_;
            }
            const baidu::galaxy::proto::ContainerDescription& IContainer::Description() const {
                return desc_;
            }


        }
    }
}

