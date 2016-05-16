// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_util.h"

#ifndef BAIDU_GALAXY_CLIENT_RES_ACTION_H
#define BAIDU_GALAXY_CLIENT_RES_ACTION_H

namespace baidu {
namespace galaxy {
namespace client {

class ResAction {

public:
    explicit ResAction(const std::string& name, const std::string& token, const std::string& nexus_key);
    ~ResAction();
    bool CreateContainerGroup(const std::string& json_file);
    bool UpdateContainerGroup(const std::string& json_file, const std::string& jobid);
    bool RemoveContainerGroup();
    bool ListContainerGroups();
    bool ShowContainerGroup();

private:
    bool Init();

private:
    ::baidu::galaxy::sdk::ResourceManager* resman_;
    ::baidu::galaxy::sdk::User user_;

}; // end class ResAction

} // end namespace client
} // end namespace galaxy
} // end namespace baidu

#endif  // BAIDU_GALAXY_CLIENT_RES_ACTION_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
