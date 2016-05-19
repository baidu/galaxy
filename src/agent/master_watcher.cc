// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "master_watcher.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

DECLARE_string(nexus_root_path);
DECLARE_string(master_path);
DECLARE_string(nexus_servers);

namespace baidu {
namespace galaxy {

MasterWatcher::MasterWatcher() : nexus_(NULL) {
    nexus_ =  new InsSDK(FLAGS_nexus_servers);
}

MasterWatcher::~MasterWatcher() {
    delete nexus_;
}

bool MasterWatcher::Init(boost::function<void(const std::string& new_master_endpoint)> handler) {
    handler_ = handler;
    return IssueNewWatch();
}

std::string MasterWatcher::GetMasterEndpoint() {
    MutexLock lock(&mutex_);
    return master_endpoint_;
}

void MasterWatcher::EventMasterChange(const ::galaxy::ins::sdk::WatchParam& param, 
                             ::galaxy::ins::sdk::SDKError /*error*/) {
    MasterWatcher* watcher = static_cast<MasterWatcher*>(param.context);
    bool ok = watcher->IssueNewWatch();
    if (!ok) {
        LOG(FATAL) << "failed to issue next watch";
        abort();
    }
}

bool MasterWatcher::IssueNewWatch() {
    std::string master_path_key = FLAGS_nexus_root_path + FLAGS_master_path;
    ::galaxy::ins::sdk::SDKError err;
    bool ok = nexus_->Watch(master_path_key, &EventMasterChange, this, &err);
    if (!ok) {
        LOG(WARNING) << "fail to watch on nexus, err_code: " << err;
        return false;
    } else {
        LOG(INFO) << "watch master success.";
    }
    std::string master_ep;
    ok = nexus_->Get(master_path_key, &master_ep, &err);
    if (!ok) {
        LOG(WARNING) << "fail to get master endpoint from nexus, err_code: " << err;
        return false;
    } else {
        LOG(INFO) << "master endpoint is: " << master_ep;
    }

    std::string old_master;
    {
        MutexLock lock(&mutex_);
        old_master = master_endpoint_;
        master_endpoint_ = master_ep;
    }

    if (old_master != master_ep) {
        handler_(master_ep);
    }
    return ok;
}

}
}
