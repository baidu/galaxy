// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "master_impl.h"

#include "common/logging.h"

namespace galaxy {

void MasterImpl::HeartBeat(::google::protobuf::RpcController* controller,
                           const ::galaxy::HeartBeatRequest* request,
                           ::galaxy::HeartBeatResponse* response,
                           ::google::protobuf::Closure* done) {
    LOG(INFO, "HeartBeat from %s", request->agent_addr().c_str());
    done->Run();
}

} // namespace galasy

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
