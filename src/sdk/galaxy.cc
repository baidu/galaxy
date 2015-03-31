// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "galaxy.h"

#include "proto/master.pb.h"
#include "rpc/rpc_client.h"

namespace galaxy {
namespace sdk{
class GalaxyImpl : public Galaxy {
public:
    GalaxyImpl(const std::string& master_addr)
      : master_(NULL) {
        rpc_client_ = new RpcClient();
        rpc_client_->GetStub(master_addr, &master_);
    }
    virtual ~GalaxyImpl() {}
    bool NewJob(const Job& job);
    bool UpdateJob(const Job& job);
    bool ListJob(std::vector<Job>& jobs);
    bool TerminateJob(int64_t job_id);
    bool ListTask(int64_t job_id,std::vector<Task>& tasks);
private:
    RpcClient* rpc_client_;
    Master_Stub* master_;
};

bool GalaxyImpl::TerminateJob(int64_t /*job_id*/) {

    return true;
}

bool GalaxyImpl::NewJob(const Job& /*job*/) {
    return true;
}

bool GalaxyImpl::UpdateJob(const Job& /*job*/) {
    return true;
}

bool GalaxyImpl::ListJob(std::vector<Job>& /*job*/) {
      return true;
}

bool GalaxyImpl::ListTask(int64_t /*job_id*/, std::vector<Task>& /*job*/) {
      return true;
}



Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

}
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
