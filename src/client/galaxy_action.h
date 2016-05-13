// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_util.h"

#ifndef BAIDU_GALAXY_CLIENT_ACTION_H
#define BAIDU_GALAXY_CLIENT_ACTION_H

namespace baidu {
namespace galaxy {
namespace client {

class Action {

public:
    explicit Action();
    ~Action();
    bool SubmitJob(const std::string& json_file);
    bool UpdateJob(const std::string& json_file, const std::string& jobid);
    bool StopJob();
    bool RemoveJob();
    bool ListJobs();
    bool ShowJob();
    bool ExecuteCmd();

private:
    bool IsAuthorized();
    bool Hostname(std::string* hostname);

private:
    std::string master_key_;
    ::baidu::galaxy::sdk::AppMaster* app_master_;
    ::baidu::galaxy::sdk::User user_;

}; // end class Action

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


#endif  // BAIDU_GALAXY_CLIENT_ACTION_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
