// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "galaxy.h"

namespace galaxy {

class GalaxyImpl : public Galaxy {
public:
    GalaxyImpl(const std::string& master_addr) {}
    bool NewTask(const std::string& task_raw,
                 const std::string& cmd_line,
                 int32_t count);
};

bool GalaxyImpl::NewTask(const std::string& task_raw,
                         const std::string& cmd_line,
                         int32_t count) {
    return true;
}

Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
