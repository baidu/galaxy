#include "task_manager.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/bind.hpp>

#include "gflags/gflags.h"
#include "gce/utils.h"
#include "logging.h"

DECLARE_string(gce_cgroup_root);
DECLARE_string(gce_support_subsystems);
DECLARE_int64(gce_initd_zombie_check_interval);

namespace baidu {
namespace galaxy {

TaskManager::TaskManager() {
}

TaskManager::~TaskManager() {
}

int TaskManager::CreatePodTasks(const PodDescriptor& pod) {
    return 0;
}

int TaskManager::DeletePodTasks(const PodDescriptor& pod) {
    return 0;
}

int TaskManager::UpdateTasksCpuLimit(const uint32_t millicores) {
    return 0;
}

void TaskManager::LoopCheckTaskStatus() {
}


int Execute(const std::string& command) {
    return 0;
        
}

} // ending namespace galaxy
} // ending namespace baidu


