#ifndef POD_MANAGER_H
#define POD_MANAGER_H

#include <string>
#include "agent/pod_info.h"
#include "agent/initd_handler.h"

namespace baidu {
namespace galaxy {

class PodManager {
public:
    int Run(const PodDesc& pod);

    int Kill(const PodDesc& pod);

    int Query(const std::string& podid, PodInfo* info);

    int List(std::vector<std::string>* pod_ids);

private:
    int Load();

    int Dump();

    int Set(PodInfo* info);

    int Delete(const std::string& podid);

    int LoopCheckPodInfos();

private:

    // Mutex infos_mutex_;
    std::map<std::string, PodInfo*> pod_infos_;

    // Mutex handlers_mutex_;
    std::map<std::string, InitdHandler*> pod_handlers_;
};

}
}

#endif
