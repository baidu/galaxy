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
    bool ListAgentsByPool(const std::string& pool);
    bool ShowAgent(const std::string& endpoint);
    bool ListAgentsByTag(const std::string& tag);
    bool ListAgents();
    bool EnterSafeMode();
    bool LeaveSafeMode();
    bool OnlineAgent(const std::string& endpoint);
    bool OfflineAgent(const std::string& endpoint);
    bool Status();
    bool CreateTag(const std::string& tag, const std::string& file);
    bool ListTags();
    bool GetPoolByAgent(const std::string& endpoint);
    bool AddUser(const std::string& user, const std::string& token);
    bool RemoveUser(const std::string& user, const std::string& token);
    bool ListUsers();
    bool ShowUser(const std::string& user);
    bool GrantUser(const std::string& user, const std::string& token, 
                   const std::string& pool, const std::string& opration, 
                   const std::string& authority);
    bool AssignQuota(const std::string& user,
                     const std::string& token,
                     uint32_t millicores,
                     const std::string& memory,
                     const std::string& disk,
                     const std::string& ssd,
                     int replica
                  );
    bool Preempt(const std::string& container_group_id, const std::string& endpoint);

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
