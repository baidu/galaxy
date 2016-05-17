// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_PROCESS_MANAGER_H
#define BAIDU_GALAXY_PROCESS_MANAGER_H

#include <string>
#include <map>

#include <thread_pool.h>

#include "protocol/galaxy.pb.h"

namespace baidu {
namespace galaxy {

typedef proto::ProcessStatus ProcessStatus;

struct Process {
    std::string process_id;
    int32_t pid;
    ProcessStatus status;
    int32_t exit_code;
};

class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    int CreateProcess(const std::string& process_id, const std::string& cmd);
    int QueryProcess(const std::string& process_id, Process& process);
    void LoopWaitProcesses();

private:
    ThreadPool background_pool_;
    std::map<std::string, Process*> processes_;
};

} // ending namespace galaxy
} // ending namespace baidu


#endif // BAIDU_GALAXY_PROCESS_MANAGER_H
