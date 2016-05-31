// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_PROCESS_MANAGER_H
#define BAIDU_GALAXY_PROCESS_MANAGER_H

#include <string>
#include <map>
#include <vector>

#include <thread_pool.h>
#include <mutex.h>

#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {

typedef proto::ProcessStatus ProcessStatus;

struct ProcessEnv {
    std::string user;
    std::vector<std::string> cgroup_paths;
    std::vector<std::string> envs;
};

struct ProcessContext {
    std::string process_id;
    std::string cmd;
    std::string work_dir;
    std::string user;
    virtual ~ProcessContext() {};
};

struct DownloadProcessContext : ProcessContext {
    std::string src_path;
    std::string dst_path;
    std::string package;
    std::string version;
};

struct Process {
    std::string process_id;
    ProcessEnv env;
    int32_t pid;
    ProcessStatus status;
    int32_t exit_code;
};

class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    int CreateProcess(const ProcessEnv& env,
                      const ProcessContext* context);
    int KillProcess(const std::string& process_id);
    int QueryProcess(const std::string& process_id,
                     Process& process);
    int ClearProcesses();

private:
    void LoopWaitProcesses();

private:
    Mutex mutex_;
    ThreadPool background_pool_;
    std::map<std::string, Process*> processes_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif // BAIDU_GALAXY_PROCESS_MANAGER_H
