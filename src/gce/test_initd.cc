#include "gflags/gflags.h"
#include "sofa/pbrpc/pbrpc.h"
#include "proto/initd.pb.h"
#include "rpc/rpc_client.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        return 0;
    }
    baidu::galaxy::Initd_Stub* initd;
    baidu::galaxy::RpcClient* rpc_client = 
        new baidu::galaxy::RpcClient();
    std::string addr("localhost:9040");
    // addr += atoi(argv[1]);
    rpc_client->GetStub(addr.c_str(), &initd);


    baidu::galaxy::ExecuteRequest exec_request;
    exec_request.set_key("dapigu");
    exec_request.set_commands("touch dapigu");
    exec_request.set_path(".");

    baidu::galaxy::ExecuteResponse exec_response;

    rpc_client->SendRequest(initd, 
            &baidu::galaxy::Initd_Stub::Execute, 
            &exec_request, 
            &exec_response, 5, 1);

    return 0;
}
