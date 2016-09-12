// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "ins_sdk.h"

#include <string>
#include <boost/function.hpp>
#include <mutex.h>

namespace baidu {
namespace galaxy {

using ::galaxy::ins::sdk::InsSDK;

class MasterWatcher {
public:
    MasterWatcher();
    ~MasterWatcher();
    bool Init(boost::function<void(const std::string& new_master_endpoint)> handler);
    std::string GetMasterEndpoint();

private:
    bool IssueNewWatch();
    static void EventMasterChange(const ::galaxy::ins::sdk::WatchParam& param,
            ::galaxy::ins::sdk::SDKError /*error*/);


    InsSDK* nexus_;
    std::string master_endpoint_;
    Mutex mutex_;
    boost::function<void(const std::string& new_master_endpoint)> handler_;
};

}
}
