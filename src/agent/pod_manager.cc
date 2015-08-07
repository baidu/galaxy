#include "agent/pod_manager.h"

namespace baidu {
namespace galaxy {

int PodManager::Run(const PodDesc& pod) {

    std::map<std::string, InitdHandler*>::iterator it;
    {
        MutexLock lock(&handlers_mutex_);
        it = pod_handlers_.find(pod.podid);
        if (it == pod_handlers)
    }

}

int PodManager::Kill(const PodDesc& pod) {

}

int PodManager::Query(const std::string& podid, 
                      PodInfo* info)  {

}

int PodManager::List(std::vector<std::string>* pod_ids) {

}

}
}
