// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "boost/shared_ptr.hpp"
#include "container/container.h"
#include "container/serializer.h"
#include "thread.h"

#include <string>
#include <map>

namespace baidu {
namespace galaxy {
namespace container {

class ContainerGc {
public:
    ContainerGc(boost::shared_ptr<Serializer> ser);
    ~ContainerGc();
    baidu::galaxy::util::ErrorCode Reload();
    baidu::galaxy::util::ErrorCode Gc(boost::shared_ptr<baidu::galaxy::container::Container> container);
    baidu::galaxy::util::ErrorCode Setup();

private:
    void GcRoutine();
    std::string GcId(boost::shared_ptr<baidu::galaxy::container::Container> container);
    baidu::galaxy::util::ErrorCode Gc(const std::string& id);
    boost::shared_ptr<Serializer> ser_;
    std::map<std::string, int64_t> gc_index_;

    boost::mutex mutex_;
    std::map<std::string, int64_t> gc_swap_index_;
    bool running_;
    baidu::common::Thread gc_thread_;

};
}
}
}
