#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <string>
#include "proto/galaxy.pb.h"
#include "proto/initd.pb.h"
#include "mutex.h"
#include "thread_pool.h"

namespace baidu {
namespace galaxy {

class TaskManager {
public:
    TaskManager();

    ~TaskManager();

    int CreatePodTasks(const PodDescriptor& pod);

    int DeletePodTasks(const PodDescriptor& pod);

    int UpdateTasksCpuLimit(const std::string& podid, 
                            const uint32_t millicores);

private:
    struct TaskInfo {
        // meta infomation
        std::string podid;
        TaskDescriptor desc;

        // dynamic resource usage
        ProcessInfo process;
        TaskStatus status;
        std::string cgroup_path;
    };

    int Execute(const TaskDescriptor& desc);

    int Kill(const std::string& task_id);

    int Update(const std::string& task_id, 
               const uint32_t millicores);

    int Show(TaskInfo* info);

    bool AttachCgroup(const std::string& cgroup_path, pid_t pid);

    void LoopCheckTaskStatus();

private:
    // key pod id
    Mutex lock_;
    std::map<std::string, TaskInfo*> tasks_;

    ThreadPool background_thread_;
    std::string cgroup_root_;
};

} // ending namespace galaxy
} // ending namespace baidu

#endif
