// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "boost/shared_ptr.hpp"
#include "container/container.h"
#include "thread.h"

#include <string>
#include <map>

namespace baidu {
namespace galaxy {
namespace container {

class ContainerGc {
public:
    ContainerGc();
    ~ContainerGc();
    baidu::galaxy::util::ErrorCode Reload();
    baidu::galaxy::util::ErrorCode Gc(const std::string& path);
    baidu::galaxy::util::ErrorCode Setup();

private:
    void GcRoutine();
    baidu::galaxy::util::ErrorCode Remove(const std::string& path);
    baidu::galaxy::util::ErrorCode ListGcPath(const std::string& path,
                std::vector<std::string>& paths);

    baidu::galaxy::util::ErrorCode DoGc(const std::string& path);
    std::map<std::string, int64_t> gc_index_;

    boost::mutex mutex_;
    std::map<std::string, int64_t> gc_swap_index_;
    bool running_;
    baidu::common::Thread gc_thread_;

};
}
}
}
