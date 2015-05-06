// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include <sofa/pbrpc/pbrpc.h>
#include <stdio.h>
#include <signal.h>

#include "pb_file_parser.h"
#include "proto/agent.pb.h"
#include "proto/master.pb.h"
#include "rpc/rpc_client.h"

namespace galaxy {

class AgentImplMock: public Agent {
public:

    void SetRunTaskResponse(const ::galaxy::RunTaskResponse& resp) {
        runtask_resp_ = resp; 
    }

    void SetKillTaskResponse(const ::galaxy::KillTaskResponse& resp) {
        killtask_resp_ = resp; 
    }

    virtual void RunTask(::google::protobuf::RpcController* controller,
            const ::galaxy::RunTaskRequest* request,
            ::galaxy::RunTaskResponse* response,
            ::google::protobuf::Closure* done);
    virtual void KillTask(::google::protobuf::RpcController* controller,
            const ::galaxy::KillTaskRequest* request,
            ::galaxy::KillTaskResponse* response,
            ::google::protobuf::Closure* done);
private:
    ::galaxy::RunTaskResponse runtask_resp_; 
    ::galaxy::KillTaskResponse killtask_resp_; 
};

void AgentImplMock::RunTask(::google::protobuf::RpcController* /*controller*/,
        const ::galaxy::RunTaskRequest* /*req*/,
        ::galaxy::RunTaskResponse* resp,
        ::google::protobuf::Closure* done) {
    resp->CopyFrom(runtask_resp_);
    done->Run();
}

void AgentImplMock::KillTask(::google::protobuf::RpcController* /*controller*/,
        const ::galaxy::KillTaskRequest* /*req*/,
        ::galaxy::KillTaskResponse* resp,
        ::google::protobuf::Closure* done) {
    resp->CopyFrom(killtask_resp_);
    done->Run();
}

}

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/) {
    s_quit = true;
}

int main(int argc, char* argv[]) {
    std::string run_task_resp_pb_file;
    std::string kill_task_resp_pb_file;
    std::string heart_beat_request_pb_file;
    std::string mock_port;
    std::string master_addr;

    galaxy::RunTaskResponse run_task_resp;
    galaxy::KillTaskResponse kill_task_resp;
    galaxy::HeartBeatRequest request;

    for (int i = 1; i < argc; i++) {
        char s[1024]; 
        if (sscanf(argv[i], "--run_task_resp=%s", s) == 1) {
            run_task_resp_pb_file = s; 
        } else if (sscanf(argv[i], "--kill_task_resp=%s", s) == 1) {
            kill_task_resp_pb_file = s; 
        } else if (sscanf(argv[i], "--port=%s", s) == 1) {
            mock_port = s;
        } else if (sscanf(argv[i], "--master_addr=%s", s) == 1) {
            master_addr = s; 
        } else if (sscanf(argv[i], "--heart_beat_request=%s", s) == 1) {
            heart_beat_request_pb_file = s;
        } else {
            fprintf(stderr, "Invalid flag '%s' \n", argv[i]); 
            exit(1);
        }
    }

    request.set_agent_addr(std::string("127.0.0.1:") + mock_port);
    
    if (!galaxy::test::ParseFromFile(run_task_resp_pb_file, &run_task_resp)) {
        fprintf(stderr, "parse run task resp failed\n"); 
        exit(1);
    }  

    if (!galaxy::test::ParseFromFile(kill_task_resp_pb_file, &kill_task_resp)) {
        fprintf(stderr, "parse kill task resp failed\n"); 
        exit(1);
    }

    if (!galaxy::test::ParseFromFile(heart_beat_request_pb_file, &request)) {
        fprintf(stderr, "parse heart beat request failed\n"); 
        exit(1);
    }

    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    galaxy::AgentImplMock* mock_service = new galaxy::AgentImplMock();
    mock_service->SetRunTaskResponse(run_task_resp);
    mock_service->SetKillTaskResponse(kill_task_resp);
    if (!rpc_server.RegisterService(mock_service)) {
        return EXIT_FAILURE; 
    }

    std::string server_host = 
        std::string("0.0.0.0:") + mock_port;
    if (!rpc_server.Start(server_host)) {
        return EXIT_FAILURE; 
    }

    galaxy::RpcClient* rpc_client = new galaxy::RpcClient();  
    galaxy::Master_Stub* master;
    if (!rpc_client->GetStub(master_addr, &master)) {
        fprintf(stderr, "get master stub failed\n"); 
        exit(1);
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);
    while (!s_quit) {
        galaxy::HeartBeatResponse response;
        rpc_client->SendRequest(master, &galaxy::Master_Stub::HeartBeat, 
                &request, &response, 5, 1);
        sleep(1); 
    }

    return EXIT_SUCCESS;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
