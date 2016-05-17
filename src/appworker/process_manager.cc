// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "process_manager.h"

#include <boost/bind.hpp>
#include <glog/logging.h>

#include "protocol/galaxy.pb.h"

DECLARE_int32(process_manager_loop_wait_interval);

namespace baidu {
namespace galaxy {

ProcessManager::ProcessManager() :
        background_pool_(1) {
    background_pool_.DelayTask(
        FLAGS_process_manager_loop_wait_interval,
        boost::bind(&ProcessManager::LoopWaitProcesses, this)
    );
}

ProcessManager::~ProcessManager() {
}

int ProcessManager::CreateProcess(const std::string& process_id, const std::string& cmd) {
    LOG(INFO) << "create process of command: " << cmd.c_str();
    Process* process = new Process();
    // TODO fork and get pid
    process->pid = 100;
    process->status = proto::kProcessRunning;
    processes_.insert(std::make_pair(process_id, process));
    return 0;
}

int ProcessManager::QueryProcess(const std::string& process_id, Process& process) {
    std::map<std::string, Process*>::iterator it = processes_.find(process_id);
    if (it == processes_.end()) {
        LOG(INFO) << "process: " << process_id.c_str() << " not exist";
        return -1;
    }
    process.process_id = process_id;
    process.pid = it->second->pid;
    process.status = it->second->status;
    process.exit_code = it->second->exit_code;
    return 0;
}

void ProcessManager::LoopWaitProcesses() {
    LOG(INFO) << "process manager loop wait processes";
    background_pool_.DelayTask(
        FLAGS_process_manager_loop_wait_interval,
        boost::bind(&ProcessManager::LoopWaitProcesses, this)
    );
}

} // ending namespace galaxy
} // ending namespace baidu
