// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>

#include "proto/agent.pb.h"
#include "rpc/rpc_client.h"

int main (int argc, char* argv[]) {
    baidu::galaxy::Agent_Stub* agent;
    baidu::galaxy::RpcClient* rpc_client =
        new baidu::galaxy::RpcClient();
    rpc_client->GetStub("localhost:8765", &agent);
    baidu::galaxy::QueryRequest request;
    baidu::galaxy::QueryResponse response;
    bool ret = rpc_client->SendRequest(agent,
                                       &baidu::galaxy::Agent_Stub::Query,
                                       &request,
                                       &response, 5, 1);
    return 0;
}
