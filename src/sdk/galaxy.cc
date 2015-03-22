// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "galaxy.h"

#include "proto/master.pb.h"
#include "rpc/rpc_client.h"

namespace galaxy {

class GalaxyImpl : public Galaxy {
public:
    GalaxyImpl(const std::string& master_addr)
      : master_(NULL) {
        rpc_client_ = new RpcClient();
        rpc_client_->GetStub(master_addr, &master_);
    }
    virtual ~GalaxyImpl() {}
    bool NewTask(const std::string& task_name,
                 const std::string& task_raw,
                 const std::string& cmd_line,
                 int32_t count);
private:
    RpcClient* rpc_client_;
    Master_Stub* master_;
};

bool GalaxyImpl::NewTask(const std::string& task_name,
                         const std::string& task_raw,
                         const std::string& cmd_line,
                         int32_t count) {
    NewTaskRequest request;
    request.set_task_name(task_name);
    request.set_task_raw(task_raw);
    request.set_cmd_line(cmd_line);
    request.set_replic_count(count);
    NewTaskResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::NewTask,
                             &request, & response, 5, 1);
    return true;
}

Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
