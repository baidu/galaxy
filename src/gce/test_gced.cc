#include "gflags/gflags.h"
#include "sofa/pbrpc/pbrpc.h"
#include "proto/gced.pb.h"
#include "proto/galaxy.pb.h"
#include "rpc/rpc_client.h"

int main(int argc, char** argv) {

    baidu::galaxy::Gced_Stub* gced;
    baidu::galaxy::RpcClient* rpc_client = 
        new baidu::galaxy::RpcClient();
    rpc_client->GetStub("127.0.0.1:8766", &gced);

    baidu::galaxy::LaunchPodRequest launch_request;
    launch_request.set_podid("testgced");
    baidu::galaxy::PodDescriptor pod;
    baidu::galaxy::TaskDescriptor task;

    task.set_binary("1111");
    task.set_start_command("touch testgced");

    baidu::galaxy::TaskDescriptor* task_desc = pod.add_tasks();
    task_desc->CopyFrom(task);
    launch_request.mutable_pod()->CopyFrom(pod);

    // baidu::galaxy::ExecuteResponse exec_response;
    baidu::galaxy::LaunchPodResponse launch_response;

    rpc_client->SendRequest(gced, 
            &baidu::galaxy::Gced_Stub::LaunchPod, 
            &launch_request, 
            &launch_response, 5, 1);

    return 0;
}
