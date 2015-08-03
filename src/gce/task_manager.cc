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

int TaskManager::UpdateTasksCpuLimit(const std::string& podid, 
                                     const uint32_t millicores) {
    return 0;
}

void TaskManager::LoopCheckTaskStatus() {
    // int status = 0;
    // pid_t pid = ::waitpid(-1, &status, WNOHANG);    
    // if (pid > 0 && WIFEXITED(status)) {
    //     LOG(WARNING, "process %d exit ret %d",
    //             pid, WEXITSTATUS(status));
    //     MutexLock scope_lock(&lock_);
    //     std::map<std::string, ProcessInfo>::iterator it
    //         = process_infos_.begin();
    //     for (; it != process_infos_.end(); ++it) {
    //         if (it->second.pid() == pid) {
    //             it->second.set_exit_code(WEXITSTATUS(status)); 
    //             it->second.set_status(kProcessTerminate);
    //             break;
    //         } 
    //     }
    // }
    // background_thread_.DelayTask(
    //         FLAGS_gce_initd_zombie_check_interval,
    //         boost::bind(&InitdImpl::ZombieCheck, this));
}

} // ending namespace galaxy
} // ending namespace baidu
