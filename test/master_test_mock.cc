// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include <sofa/pbrpc/pbrpc.h>
#include <stdio.h>
#include <signal.h>

#include "pb_file_parser.h"
#include "proto/master.pb.h"

namespace galaxy {

class MasterImplMock : public Master {
public:
    void SetHeartBeatResponse(
            const ::galaxy::HeartBeatResponse& resp) {
        heart_beat_resp_ = resp;
    }
    virtual void HeartBeat(::google::protobuf::RpcController* controller,
            const ::galaxy::HeartBeatRequest* request,
            ::galaxy::HeartBeatResponse* response,
            ::google::protobuf::Closure* done) {
        response->CopyFrom(heart_beat_resp_); 
        done->Run();
    }
private:
    ::galaxy::HeartBeatResponse heart_beat_resp_;    
};

}   // ending namespace galaxy

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/) {
    s_quit = true;
}

int main(int argc, char* argv[]) {
    std::string heart_beat_resp_pb_file;
    std::string mock_port;
    galaxy::HeartBeatResponse heart_beat_resp;

    for (int i = 1; i < argc; i++) {
        char s[1024]; 
        if (sscanf(argv[i], "--heart_beat_resp=%s", s) == 1) {
            heart_beat_resp_pb_file = s; 
        } else if (sscanf(argv[i], "--port=%s", s) == 1){
            mock_port = s;
        } else {
            fprintf(stderr, "Invalid flag '%s' \n", argv[i]); 
            exit(1);
        }
    }

    // TODO parse from file to setup pb resp
    if (!galaxy::test::ParseFromFile(heart_beat_resp_pb_file, &heart_beat_resp)) {
        fprintf(stderr, "parse heart beat resp failed\n"); 
        exit(1);
    }

    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    galaxy::MasterImplMock* mock_service = new galaxy::MasterImplMock();
    mock_service->SetHeartBeatResponse(heart_beat_resp);
    if (!rpc_server.RegisterService(mock_service)) {
        return EXIT_FAILURE;
    }

    std::string server_host =
        std::string("0.0.0.0:") + mock_port;
    if (!rpc_server.Start(server_host)) {
        return EXIT_FAILURE; 
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);
    while (!s_quit) {
        sleep(1); 
    }
    return EXIT_SUCCESS;
}
/* vim: set ts=4 sw=4 sts=4 tw=100 */
