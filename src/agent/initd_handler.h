#ifndef INITDHANDLER_H
#define INITDHANDLER_H

#include "rpc_client.h"

namespace baidu {
namespace galaxy  {

class InitdHandler {
public:
    int Create(const PodDesc& pod);

    int Delete(const PodDesc& pod);

    int UpdateCpuLimit(const PodDesc& pod);

    int Show(PodInfo* info);

private:
    int LoopCheckPodInfo();

private:
    std::string podid_;

    PodInfo* pod_info_;

    RpcClient* rpc_client_;
};

}
}
