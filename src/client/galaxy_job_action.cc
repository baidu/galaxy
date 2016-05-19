// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tprinter.h>
#include "galaxy_job_action.h"

//user
DECLARE_string(username);
DECLARE_string(token);

namespace baidu {
namespace galaxy {
namespace client {

JobAction::JobAction() { 
    app_master_ = new ::baidu::galaxy::sdk::AppMaster();
    user_.user = FLAGS_username;
    user_.token = FLAGS_token;
}

JobAction::~JobAction() {
    if (NULL != app_master_) {
        delete app_master_;
    }
}

bool JobAction::Init() {
    //用户名和token验证
    //

    //
    if (!app_master_->GetStub()) {
        return false;
    }
    return true;
}

bool JobAction::SubmitJob(const std::string& json_file) {
    if (json_file.empty()) {
        fprintf(stderr, "json_file is needed\n");
        return false;
    }

    if(!this->Init()) {
        return false;
    }
    
    baidu::galaxy::sdk::SubmitJobRequest request;
    baidu::galaxy::sdk::SubmitJobResponse response;
    request.user = user_;
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    int ok = BuildJobFromConfig(json_file, &request.job);
    if (ok != 0) {
        return false;
    }

    request.job.run_user = user_.user;

    bool ret = app_master_->SubmitJob(request, &response);
    if (ret) {
        printf("Submit job %s\n", response.jobid.c_str());
    } else {
        printf("Submit job failed for reason %d:%s\n", response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

bool JobAction::UpdateJob(const std::string& json_file, const std::string& jobid) {
    if (json_file.empty() || jobid.empty()) {
        fprintf(stderr, "json_file and jobid are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    baidu::galaxy::sdk::UpdateJobRequest request;
    baidu::galaxy::sdk::UpdateJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    int ok = BuildJobFromConfig(json_file, &request.job);
    if (ok != 0) {
        return false;
    }

    bool ret =  app_master_->UpdateJob(request, &response);
    if (ret) {
        printf("Update job %s\n success", jobid.c_str());
    } else {
        printf("Update job %s failed for reason %d:%s\n", 
                jobid.c_str(), response.error_code.status, response.error_code.reason.c_str());
    }

    return ret;
}

bool JobAction::StopJob(const std::string& jobid) {
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    baidu::galaxy::sdk::StopJobRequest request;
    baidu::galaxy::sdk::StopJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    bool ret =  app_master_->StopJob(request, &response);
    if (ret) {
        printf("Stop job %s\n success", jobid.c_str());
    } else {
        printf("Stop job %s failed for reason %d:%s\n", 
                jobid.c_str(), response.error_code.status, response.error_code.reason.c_str());
    }

    return ret;
}

bool JobAction::RemoveJob(const std::string& jobid) {
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    baidu::galaxy::sdk::RemoveJobRequest request;
    baidu::galaxy::sdk::RemoveJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    
    if (GetHostname(&request.hostname)) {
        return false;
    }

    bool ret =  app_master_->RemoveJob(request, &response);
    if (ret) {
        printf("Stop job %s\n success", jobid.c_str());
    } else {
        printf("Stop job %s failed for reason %d:%s\n", 
                jobid.c_str(), response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

//待补充

bool JobAction::ListJobs() {
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListJobsRequest request;
    ::baidu::galaxy::sdk::ListJobsResponse response;
    request.user = user_;

    bool ret = app_master_->ListJobs(request, &response);
    if (ret) {
        baidu::common::TPrinter tp(12);
        tp.AddRow(12, "", "id", "name", "type","status", "stat(run/pending/deploy/death/fail)", "replica", 
                    "work_volum", "data_volum", "cpu", "memory", "create", "update");
        std::vector< ::baidu::galaxy::sdk::JobOverview> ::iterator it = response.jobs.begin();
        for (; it != response.jobs.end(); ++it) {
            std::vector<std::string> vs;
            vs.push_back(baidu::common::NumToString(it - response.jobs.begin()));
            vs.push_back(it->desc.name);
            vs.push_back(baidu::common::NumToString(it->desc.type));
            vs.push_back(baidu::common::NumToString(it->status));
            vs.push_back(baidu::common::NumToString(it->running_num) + "/" +
                            baidu::common::NumToString(it->pending_num) + "/" +
                            baidu::common::NumToString(it->deploying_num) + "/" +
                            baidu::common::NumToString(it->death_num) + "/" +
                            baidu::common::NumToString(it->fail_count)
                        );
            vs.push_back(baidu::common::NumToString(it->desc.deploy.replica));
            vs.push_back(baidu::common::NumToString(it->desc.pod.workspace_volum.size));
            int64_t data_vol = 0;
            std::vector< ::baidu::galaxy::sdk::VolumRequired> ::iterator vol_it = it->desc.pod.data_volums.begin();
            for (; vol_it != it->desc.pod.data_volums.end(); ++vol_it) {
                data_vol  += vol_it->size; 
            }

            vs.push_back(baidu::common::NumToString(data_vol));

            std::vector< ::baidu::galaxy::sdk::TaskDescription>::iterator task_it = it->desc.pod.tasks.begin();
            int64_t cpu = 0;
            int64_t mem = 0;
            for (; task_it != it->desc.pod.tasks.end(); ++task_it) {
                cpu += task_it->cpu.milli_core;
                mem += task_it->memory.size;
            }
            vs.push_back(baidu::common::NumToString(cpu));
            vs.push_back(baidu::common::NumToString(mem));
            vs.push_back(FormatDate(it->create_time));
            vs.push_back(FormatDate(it->update_time));
            tp.AddRow(vs);
        }
        printf("%s\n", tp.ToString().c_str());
    } else {
        printf("List job failed for reason %d:%s\n", response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

//待补充
bool JobAction::ShowJob(const std::string& jobid) {
    
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ShowJobRequest request;
    ::baidu::galaxy::sdk::ShowJobResponse response;
    request.user = user_;
    request.jobid = jobid;
    bool ret = app_master_->ShowJob(request, &response);

    if (ret) {
        printf("Show job %s\n", response.job.jobid.c_str());
    } else {
        printf("Show job failed for reason %d:%s\n", response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;

    return false;
}

bool JobAction::ExecuteCmd(const std::string& jobid, const std::string& cmd) {
    if (jobid.empty() || cmd.empty()) {
        fprintf(stderr, "jobid and cmd are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    baidu::galaxy::sdk::ExecuteCmdRequest request;
    baidu::galaxy::sdk::ExecuteCmdResponse response;
    request.user = user_;
    request.jobid = jobid;
    request.cmd = cmd;
    bool ret =  app_master_->ExecuteCmd(request, &response);
    if (ret) {
        printf("Execute job %s\n success", jobid.c_str());
    } else {
        printf("Execute job %s failed for reason %d:%s\n", 
                jobid.c_str(), response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


/* vim: set ts=4 sw=4 sts=4 tw=100 */
