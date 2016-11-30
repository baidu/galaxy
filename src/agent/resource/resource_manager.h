// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "cpu_resource.h"
#include "memory_resource.h"
#include "volum_resource.h"

#include "util/error_code.h"

#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

namespace baidu {
namespace galaxy {
namespace proto {
class ContainerDescription;
class VolumRequired;
}

namespace resource {

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    int Load();
    std::string ToString();

    baidu::galaxy::util::ErrorCode Allocate(const baidu::galaxy::proto::ContainerDescription& desc);
    baidu::galaxy::util::ErrorCode Release(const baidu::galaxy::proto::ContainerDescription& desc);
    int Resource(boost::shared_ptr<void> resource);

    boost::shared_ptr<baidu::galaxy::proto::Resource> GetCpuResource();
    boost::shared_ptr<baidu::galaxy::proto::Resource> GetMemoryResource();
    void GetVolumResource(std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumResource> >& resource);

private:
    baidu::galaxy::util::ErrorCode Allocate(std::vector<const baidu::galaxy::proto::VolumRequired*>& vv);
    void CalResource(const baidu::galaxy::proto::ContainerDescription& desc,
            int64_t& cpu_millicores,
            int64_t& memroy_require,
            std::vector<const baidu::galaxy::proto::VolumRequired*>& vv);
    boost::mutex mutex_;
    boost::scoped_ptr<CpuResource> cpu_;
    boost::scoped_ptr<MemoryResource> memory_;
    boost::scoped_ptr<VolumResource> volum_;

};

}
}
}
