// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gflags/gflags.h>
#include "galaxy_action.h"

//nexus
DECLARE_string(nexus_servers);
DECLARE_string(nexus_root_path);
DECLARE_string(appmaster_nexus_path);
DECLARE_string(appmaster_endpoint);

namespace baidu {
namespace galaxy {
namespace client {

Action::Action(const std::string& name, const std::string& token) { 
    master_key_ = FLAGS_nexus_root_path + FLAGS_master_path; 
    app_master_ = new ::baidu::galaxy::sdk::AppMaster(FLAGS_nexus_root_path);
    user_. user =  user;
    user_. token = token;
}

Action::~Action() {
    if (NULL != app_master_) {
        delete app_master_
    }
}

bool Action::IsAuthorized() {
    return false;
}

bool Hostname(std::string* hostname) {
    char buf[100];
    if (gethostname(buf, 100) != 0) {
        fprintf(stderr, "gethostname failed\n");
        return false;
    }
    *hostname = buf;
    return true;
}

bool Action::SubmitJob(const std::string& json_file) {
    if (json_file.empty()) {
        fprintf(stderr, "json_file is needed\n");
        return false;
    }

    if(!this->IsAuthorized()) {
        return false;
    }
    
    baidu::galaxy::sdk::SubmitJobRequest request;
    baidu::galaxy::sdk::SubmitJobResponse response;
    request.user = user_;
    if (!this->Hostname(&request.hostname)) {
        return false;
    }

    int ok = baidu::galaxy::client::BuildJobFromConfig(json_file, &request.job);
    if (ok != 0) {
        return false;
    }

    bool ret = app_master_->SubmitJob(request, &response);
    if (ret) {
        printf("Submit job %s\n", response.jobid.c_str());
    } else {
        printf("Submit job failed for reason %d:%s\n", response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

bool Action::UpdateJob(const std::string& json_file, const std::string& jobid) {
    if (json_file.empty() || jobid.empty()) {
        fprintf(stderr, "json_file and jobid are needed\n");
        return false;
    }
    
    if(!this->IsAuthorized()) {
        return false;
    }

    baidu::galaxy::sdk::UpdateJobRequest request;
    baidu::galaxy::sdk::UpdateJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    if (!this->Hostname(&request.hostname)) {
        return false;
    }

    int ok = baidu::galaxy::client::BuildJobFromConfig(json_file, &request->job);
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

bool Action::StopJob(const std::string& jobid) {
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }
    
    if(!this->IsAuthorized()) {
        return false;
    }

    baidu::galaxy::sdk::StopJobRequest request;
    baidu::galaxy::sdk::StopJobResponse reponse;
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

bool Action::RemoveJob(const std::string& jobid) {
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }
    baidu::galaxy::sdk::RemoveJobRequest request;
    baidu::galaxy::sdk::RemoveJobResponse reponse;
    request.jobid = jobid;
    request.user = user_;
    
    if (!this->Hostname(&request.hostname)) {
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

bool Action::ListJobs(const std::string& json_file) {
    
    if (json_file.empty()) {
        fprintf(stderr, "json file is needed\n");
        return false;
    }

    ::baidu::galaxy::sdk::JobDescription job;
    ::baidu::galaxy::sdk::ListJobsRequest request;
    ::baidu::galaxy::sdk::ListJobsResponse response;
    ::baidu::galaxy::sdk::User;
    std::hostname;

    int ok = baidu::galaxy::client::BuildJobFromConfig(json_file, &job, &request.user, &hostname, true);
    if (ok != 0) {
        return false;
    }

    bool ret = app_master_->ListJobs(request, &response);
    if (ret) {
        printf("Submit job %s\n", response.jobid.c_str());
    } else {
        printf("List job failed for reason %d:%s\n", response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

bool Action::ShowJob() {
    return false;
}

bool Action::ExecuteCmd(const std::string& jobid, const std::string& cmd) {
    if (jobid.empty() || cmd.empty()) {
        fprintf(stderr, "jobid and cmd are needed\n");
        return false;
    }
    baidu::galaxy::sdk::ExecuteCmdRequest request;
    baidu::galaxy::sdk::ExecuteCmdResponse response;
    request.jobid = jobid;
    request.cmd = cmd;
    bool ret =  app_master_->ExecuteCmd(request, &response);
    if (ret) {
        printf("Stop job %s\n success", jobid.c_str());
    } else {
        printf("Stop job %s failed for reason %d:%s\n", 
                jobid.c_str(), response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


/* vim: set ts=4 sw=4 sts=4 tw=100 */
