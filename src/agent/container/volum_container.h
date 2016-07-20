// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "icontainer.h"
#include "container_status.h"
#include "volum/volum_group.h"

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include <string>

namespace baidu {
    namespace galaxy {
        namespace container {
            
            class VolumContainer : public IContainer {
            public:
                VolumContainer(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc) ;
                ~VolumContainer(); 
                
                baidu::galaxy::util::ErrorCode Construct();
                baidu::galaxy::util::ErrorCode Destroy();
                baidu::galaxy::util::ErrorCode Reload(boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> meta);
                
                boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> ContainerMeta();
                boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ContainerInfo(bool full_info);
                boost::shared_ptr<baidu::galaxy::proto::ContainerMetrix> ContainerMetrix();
                boost::shared_ptr<ContainerProperty> Property();
                std::string ContainerGcPath();
                
            private:
                baidu::galaxy::container::ContainerStatus status_;
                boost::shared_ptr<baidu::galaxy::volum::VolumGroup> volum_group_;
                int64_t created_time_;
                int64_t destroy_time_;
                int ConstructVolumGroup();
            };
        }
    }
}
