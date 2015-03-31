// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#ifndef  GALAXY_GALAXY_H_
#define  GALAXY_GALAXY_H_

#include <string>
#include <vector>
#include <stdint.h>

namespace galaxy {
namespace sdk{
//
//
enum PackageType{
    FTP,
    HTTP,
    P2P,
    BINARY
};

enum TaskState{
    DEPLOYING,
    RUNNING,
    ERROR
};

struct Task{
    int64_t task_id;
    int64_t job_id;
    std::string task_name;
    TaskState state;
    std::string host;
};


struct Package{
    PackageType type;
    std::string source;
    std::string version;
};

struct ResourceRequirement{
    std::string name;
    int64_t value;
};


struct Job{
    int64_t job_id;
    std::string job_name;
    std::string cmd_line;
    Package pkg;
    int32_t replicate_count;
    std::vector<ResourceRequirement> resource_vector;
};


class Galaxy {
public:
    static Galaxy* ConnectGalaxy(const std::string& master_addr);
    //create a new job
    virtual bool NewJob(const Job& job) = 0;
    //update job for example update the replicate_count
    virtual bool UpdateJob(const Job& job) = 0;
    //list all jobs in galaxys
    virtual bool ListJob(std::vector<Job> &jobs) = 0;
    //termintate job
    virtual bool TerminateJob(int64_t job_id) = 0;
    //list all tasks of job
    virtual bool ListTask(int64_t job_id,std::vector<Task> &tasks) = 0;

};

} // namepsace sdk
} // namespace galaxy

#endif  // GALAXY_GALAXY_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
