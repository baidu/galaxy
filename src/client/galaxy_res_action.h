// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_CLIENT_RES_ACTION_H
#define BAIDU_GALAXY_CLIENT_RES_ACTION_H

#include "galaxy_util.h"
#include <gflags/gflags.h>
#include "sdk/galaxy_sdk_resman.h"

namespace baidu {
namespace galaxy {
namespace client {

class ResAction {

public:
    ResAction();
    ~ResAction();
    bool CreateContainerGroup(const std::string& json_file);
    bool UpdateContainerGroup(const std::string& json_file, const std::string& id);
    bool RemoveContainerGroup(const std::string& id);
    bool ListContainerGroups();
    bool ShowContainerGroup(const std::string& id);
    bool AddAgent(const std::string& pool, const std::string& endpoint);
    bool RemoveAgent(const std::string& endpoint);
    bool ListAgents(const std::string& pool);
    bool EnterSafeMode();
    bool LeaveSafeMode();
    bool OnlineAgent(const std::string& endpoint);
    bool OfflineAgent(const std::string& endpoint);
    bool Status();

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
