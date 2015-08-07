#ifndef POD_INFO_H
#define POD_INFO_H

#include <string>
#include "proto/galaxy.pb.h"

namespace baidu {
namespace galaxy {

class PodDesc {
public:
    // pod id
    std::string id;

    // pod meta infomation
    PodDescriptor desc;
};

class PodInfo {
public:
    PodDesc desc;

    // pod dynamic infomation
    PodStatus usage;
};

}
}

#endif
