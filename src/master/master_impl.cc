// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "master_impl.h"

namespace galaxy {

void MasterImpl::HeartBeat(::google::protobuf::RpcController* controller,
                           const ::galaxy::HeartBeatRequest* request,
                           ::galaxy::HeartBeatResponse* response,
                           ::google::protobuf::Closure* done) {
    done->Run();
}
void MasterImpl::TaskReport(::google::protobuf::RpcController* controller,
                            const ::galaxy::TaskReportRequest* request,
                            ::galaxy::TaskReportResponse* response,
                            ::google::protobuf::Closure* done) {
    done->Run();
}

} // namespace galasy

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
