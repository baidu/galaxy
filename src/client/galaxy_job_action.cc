// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <thread.h>
#include <tprinter.h>
#include "galaxy_job_action.h"

DEFINE_string(nexus_root, "/galaxy3", "root prefix on nexus");
DEFINE_string(nexus_addr, "", "nexus server list");
DEFINE_string(resman_path, "/resman", "resman path on nexus");
DEFINE_string(appmaster_path, "/appmaster", "appmaster path on nexus");
DEFINE_string(username, "default", "username");
DEFINE_string(token, "default", "token");

namespace baidu {
namespace galaxy {
namespace client {

JobAction::JobAction() : app_master_(NULL), resman_(NULL) { 
    user_.user = FLAGS_username;
    user_.token = FLAGS_token;
}

JobAction::~JobAction() {
    if (NULL != app_master_) {
        delete app_master_;
    }

    if (NULL != resman_) {
        delete resman_;
    }
}

bool JobAction::InitResman() {
    std::string path = FLAGS_nexus_root + FLAGS_resman_path;
    resman_ = ::baidu::galaxy::sdk::ResourceManager::ConnectResourceManager(FLAGS_nexus_addr, path);
    if (resman_ == NULL) {
        return false;
    }
    return true;
}

bool JobAction::Init() {
    std::string path = FLAGS_nexus_root + FLAGS_appmaster_path;
    app_master_ = ::baidu::galaxy::sdk::AppMaster::ConnectAppMaster(FLAGS_nexus_addr, path); 
    if (app_master_ == NULL) {
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
    
    ::baidu::galaxy::sdk::SubmitJobRequest request;
    ::baidu::galaxy::sdk::SubmitJobResponse response;
    request.user = user_;
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    if (json_file.empty()) {
        fprintf(stderr, "json file is needed\n");
        return false;
    }

    int ok = BuildJobFromConfig(json_file, &request.job);
    if (ok != 0) {
        return false;
    }

    bool ret = app_master_->SubmitJob(request, &response);
    if (ret) {
        printf("Submit job %s\n", response.jobid.c_str());
    } else {
        printf("Submit job failed for reason %s:%s\n", StringStatus(response.error_code.status).c_str(), 
                    response.error_code.reason.c_str());
    }
    return ret;
}

bool JobAction::UpdateJob(const std::string& json_file, const std::string& jobid, 
                            const std::string& operation, uint32_t update_break_count) {
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::UpdateJobRequest request;
    ::baidu::galaxy::sdk::UpdateJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    if (operation.compare("start") == 0 && update_break_count >= 0) {
        fprintf(stderr, "breakpoint update start\n");
        if (json_file.empty() || BuildJobFromConfig(json_file, &request.job) != 0) {
            fprintf(stderr, "json_file error\n");
            fprintf(stderr, "json_file [%s] error\n", json_file.c_str());
            return false;
        }
        request.operate = baidu::galaxy::sdk::kUpdateJobStart;
        request.job.deploy.update_break_count = update_break_count;
    } else if (operation.compare("continue") == 0) {
        fprintf(stderr, "breakpoint update continue\n");
        request.operate = baidu::galaxy::sdk::kUpdateJobContinue;
    } else if (operation.compare("rollback") == 0) {
        fprintf(stderr, "breakpoint update rollback\n");
        request.operate = baidu::galaxy::sdk::kUpdateJobRollback;
    } else if (operation.compare("default") == 0 || operation.empty()) {
        fprintf(stderr, "batch update default\n");
        if (json_file.empty() || BuildJobFromConfig(json_file, &request.job) != 0) {
            fprintf(stderr, "json_file [%s] error\n", json_file.c_str());
            return false;
        }
        request.operate = baidu::galaxy::sdk::kUpdateJobDefault;
    } else {
        fprintf(stderr, "update operation must be start, continue, rollback or default\n");
        return false;
    }

    bool ret =  app_master_->UpdateJob(request, &response);
    if (ret) {
        printf("Update job %s\n success", jobid.c_str());
    } else {
        printf("Update job %s failed for reason %s:%s\n", 
                jobid.c_str(), StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
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

    ::baidu::galaxy::sdk::StopJobRequest request;
    ::baidu::galaxy::sdk::StopJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    bool ret =  app_master_->StopJob(request, &response);
    if (ret) {
        printf("Stop job %s\n success", jobid.c_str());
    } else {
        printf("Stop job %s failed for reason %s:%s\n", 
                jobid.c_str(), StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
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

    ::baidu::galaxy::sdk::RemoveJobRequest request;
    ::baidu::galaxy::sdk::RemoveJobResponse response;
    request.jobid = jobid;
    request.user = user_;
    
    if (!GetHostname(&request.hostname)) {
        return false;
    }

    bool ret =  app_master_->RemoveJob(request, &response);
    if (ret) {
        printf("Remove job %s success\n", jobid.c_str());
    } else {
        printf("Remove job %s failed for reason %s:%s\n", 
                jobid.c_str(), StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

struct ListContainerParams {
    JobAction* action;
    ::baidu::galaxy::sdk::ListContainerGroupsRequest request;
    std::map<std::string, ::baidu::galaxy::sdk::ContainerGroupStatistics>* containers;
};

struct ListJobParams {
    JobAction* action;
    ::baidu::galaxy::sdk::ListJobsRequest request;
    std::vector< ::baidu::galaxy::sdk::JobOverview>* jobs;
};

void* JobAction::ListContainers(void* param) {
    ListContainerParams* container_params = (ListContainerParams*)param;
    JobAction* self = container_params->action;
    if (!self->InitResman()) {
        return NULL;
    }
    ::baidu::galaxy::sdk::ListContainerGroupsResponse response;
    bool ret = self->resman_->ListContainerGroups(container_params->request, &response);
    if (ret) {
        for (uint32_t i = 0; i < response.containers.size(); ++i) {
            container_params->containers->insert(make_pair(response.containers[i].id, response.containers[i]));
        }
    } else {
        printf("List containers failed for reason %s:%s\n", 
                StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return NULL;
}

void* JobAction::ListJobs(void* param) {
    ListJobParams* job_params = (ListJobParams*)param;
    JobAction* self = job_params->action;
    if(!self->Init()) {
        return NULL;
    }
    ::baidu::galaxy::sdk::ListJobsResponse response;
    bool ret = self->app_master_->ListJobs(job_params->request, &response);
    if (ret) {
        for (uint32_t i = 0; i < response.jobs.size(); ++i) {
            job_params->jobs->push_back(response.jobs[i]);
        }
    } else {
        printf("List jobs failed for reason %s:%s\n", 
                StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return NULL;
}

bool JobAction::ListJobs(const std::string& soptions) {
    std::vector<std::string> options;
    ::baidu::common::SplitString(soptions, ",", &options);
    
    ::baidu::galaxy::sdk::ListContainerGroupsRequest resman_request;
    resman_request.user = user_;
    std::map<std::string, ::baidu::galaxy::sdk::ContainerGroupStatistics> containers;

    ListContainerParams list_containers_params;
    ::baidu::common::Thread container_thread;
    list_containers_params.action = this;
    list_containers_params.containers = &containers;
    list_containers_params.request = resman_request;
    
    container_thread.Start(ListContainers, &list_containers_params);
    
    ::baidu::galaxy::sdk::ListJobsRequest request;
    request.user = user_;
    std::vector< ::baidu::galaxy::sdk::JobOverview> jobs;

    ListJobParams list_job_params;
    ::baidu::common::Thread job_thread;
    list_job_params.action = this;
    list_job_params.jobs = &jobs;
    list_job_params.request = request;
    job_thread.Start(ListJobs, &list_job_params);
    
    job_thread.Join();
    container_thread.Join();
    
    if (containers.size() == 0) {
        return false;
    }

    if (jobs.size() == 0) {
        return false;
    }
 
    std::string array_headers[7] = {"", "id", "name", "type","status", "r/p/d/die/f", "repli"};
    std::vector<std::string> headers(array_headers, array_headers + 7);

    if (find(options.begin(), options.end(), "cpu") != options.end()) {
        headers.push_back("cpu(a/u)");
    }

    if (find(options.begin(), options.end(), "mem") != options.end()) {
        headers.push_back("memory(a/u)");
    }

    if (find(options.begin(), options.end(), "volums") != options.end()) {
        headers.push_back("volums(med/a/u)");
    }

    if (options.size() == 0) {
        headers.push_back("cpu(a/u)");
        headers.push_back("memory(a/u)");
        headers.push_back("volums(med/a/u)");
    }

    headers.push_back("create_time");
    headers.push_back("update_time");

    baidu::common::TPrinter tp_jobs(headers.size());
    tp_jobs.AddRow(headers);

    std::vector<std::string> values;
    for (uint32_t i = 0; i < jobs.size(); ++i) {
        values.clear();
        std::string sstat = baidu::common::NumToString(jobs[i].running_num) + "/" +
                            baidu::common::NumToString(jobs[i].pending_num) + "/" +
                            baidu::common::NumToString(jobs[i].deploying_num) + "/" +
                            baidu::common::NumToString(jobs[i].death_num) + "/" +
                            baidu::common::NumToString(jobs[i].fail_count);
        std::string scpu;
        std::string smem;
        std::string svolums;

        std::map<std::string, ::baidu::galaxy::sdk::ContainerGroupStatistics>::iterator it 
                                        = containers.find(jobs[i].jobid);
        if (it != containers.end()) {
            if (options.size() == 0 || find(options.begin(), options.end(), "cpu") != options.end()) {
                scpu = ::baidu::common::NumToString(it->second.cpu.assigned / 1000.0) + "/"
                       + ::baidu::common::NumToString(it->second.cpu.used / 1000.0);
            }
            if (options.size() == 0 || find(options.begin(), options.end(), "mem") != options.end()) {
                smem = ::baidu::common::HumanReadableString(it->second.memory.assigned) + "/"
                       + ::baidu::common::HumanReadableString(it->second.memory.used);
            }
            if (options.size() == 0 || find(options.begin(), options.end(), "volums") != options.end()) {
                for (size_t j = 0; j < it->second.volums.size(); ++j) {
                    values.clear();
                    std::string svolums;
                    svolums = StringVolumMedium(it->second.volums[j].medium) + "/"
                              + ::baidu::common::HumanReadableString(it->second.volums[j].volum.assigned) + "/"
                              + ::baidu::common::HumanReadableString(it->second.volums[j].volum.used);
                    if (j == 0) {
                        values.push_back(::baidu::common::NumToString(i));
                        values.push_back(jobs[i].jobid);
                        values.push_back(jobs[i].desc.name);
                        values.push_back(StringJobType(jobs[i].desc.type));
                        values.push_back(StringJobStatus(jobs[i].status));
                        values.push_back(sstat);
                        values.push_back(::baidu::common::NumToString(jobs[i].desc.deploy.replica));
                        if (!scpu.empty()) {
                            values.push_back(scpu);
                        }
                        if (!smem.empty()) {
                            values.push_back(smem);
                        }
                        values.push_back(svolums);
                        values.push_back(FormatDate(jobs[i].create_time));
                        values.push_back(FormatDate(jobs[i].update_time));
                    } else {
                        int base_size = sizeof(array_headers) / sizeof(std::string);
                        for (int base_it = 0; base_it < base_size; ++base_it) {
                            values.push_back("");
                        }

                        if (!scpu.empty()) {
                            values.push_back("");
                        }
                        if (!smem.empty()) {
                            values.push_back("");
                        }
                        values.push_back(svolums);
                        values.push_back("");
                        values.push_back("");
                    } 
                    tp_jobs.AddRow(values);
                }
                if (it->second.volums.size() == 0) {
                    values.push_back(::baidu::common::NumToString(i));
                    values.push_back(jobs[i].jobid);
                    values.push_back(jobs[i].desc.name);
                    values.push_back(StringJobType(jobs[i].desc.type));
                    values.push_back(StringJobStatus(jobs[i].status));
                    values.push_back(sstat);
                    values.push_back(::baidu::common::NumToString(jobs[i].desc.deploy.replica));
                    if (!scpu.empty()) {
                        values.push_back(scpu);
                    }
                    if (!smem.empty()) {
                        values.push_back(smem);
                    }
                    values.push_back("");
                    values.push_back(FormatDate(jobs[i].create_time));
                    values.push_back(FormatDate(jobs[i].update_time));
                    tp_jobs.AddRow(values);
                }
            } 
            if (options.size() != 0 && find(options.begin(), options.end(), "volums") == options.end()) {
                values.push_back(::baidu::common::NumToString(i));
                values.push_back(jobs[i].jobid);
                values.push_back(jobs[i].desc.name);
                values.push_back(StringJobType(jobs[i].desc.type));
                values.push_back(StringJobStatus(jobs[i].status));
                values.push_back(sstat);
                values.push_back(::baidu::common::NumToString(jobs[i].desc.deploy.replica));
                if (!scpu.empty()) {
                    values.push_back(scpu);
                }
                if (!smem.empty()) {
                    values.push_back(smem);
                }
                values.push_back(FormatDate(jobs[i].create_time));
                values.push_back(FormatDate(jobs[i].update_time));
                tp_jobs.AddRow(values);
            }
        } else {
            values.push_back(::baidu::common::NumToString(i));
            values.push_back(jobs[i].jobid);
            values.push_back(jobs[i].desc.name);
            values.push_back(StringJobType(jobs[i].desc.type));
            values.push_back(StringJobStatus(jobs[i].status));
            values.push_back(sstat);
            values.push_back(::baidu::common::NumToString(jobs[i].desc.deploy.replica));
            if (options.size() == 0 || find(options.begin(), options.end(), "cpu") != options.end()) {
                values.push_back("");
            }
            if (options.size() == 0 || find(options.begin(), options.end(), "mem") != options.end()) {
                values.push_back("");
            }
            if (options.size() == 0 || find(options.begin(), options.end(), "volums") != options.end()) {
                values.push_back("");
            }
            values.push_back(FormatDate(jobs[i].create_time));
            values.push_back(FormatDate(jobs[i].update_time));
            tp_jobs.AddRow(values);
        }
    }
    printf("%s\n", tp_jobs.ToString().c_str());
    return true;
}

struct ShowContainerParams {
    JobAction* action;
    ::baidu::galaxy::sdk::ShowContainerGroupRequest request;
    ::baidu::galaxy::sdk::ShowContainerGroupResponse* response;
    bool* ret;
};

struct ShowJobParams {
    JobAction* action;
    ::baidu::galaxy::sdk::ShowJobRequest request;
    ::baidu::galaxy::sdk::ShowJobResponse* response;
    bool* ret;
};

void* JobAction::ShowJob(void* param) {
    ShowJobParams* params = (ShowJobParams*)param;
    JobAction* self = (JobAction*)params->action;
    if(!self->Init()) {
        return NULL;
    }
    
    bool ret = self->app_master_->ShowJob(params->request, params->response);
    if (!ret) {
        printf("Show job failed for reason %s:%s\n",
                StringStatus(params->response->error_code.status).c_str(), params->response->error_code.reason.c_str());
        return NULL;
    }
    *params->ret = true;
    return NULL;
}

void* JobAction::ShowContainerGroup(void* param) {
    ShowContainerParams* params = (ShowContainerParams*)param;
    JobAction* self = (JobAction*)params->action;
    if (!self->InitResman()) {
        return NULL;
    }
    bool ret = self->resman_->ShowContainerGroup(params->request, params->response);
    if (!ret) {
        printf("Show Container failed for reason %s:%s\n",
                StringStatus(params->response->error_code.status).c_str(), params->response->error_code.reason.c_str());
        return NULL;
    }
    *params->ret = true;
    return NULL;
}

bool JobAction::ShowJob(const std::string& jobid, const std::string& soptions) {
    
    if (jobid.empty()) {
        fprintf(stderr, "jobid is needed\n");
        return false;
    }

    std::vector<std::string> options;
    ::baidu::common::SplitString(soptions, ",", &options);

    ::baidu::galaxy::sdk::ShowJobRequest request;
    ::baidu::galaxy::sdk::ShowJobResponse response;
    bool show_job = false;
    request.user = user_;
    request.jobid = jobid;
    ShowJobParams show_job_param;
    show_job_param.action = this;
    show_job_param.request = request;
    show_job_param.response = &response;
    show_job_param.ret = &show_job;
    ::baidu::common::Thread job_thread;
    job_thread.Start(ShowJob, &show_job_param);
    
    ::baidu::galaxy::sdk::ShowContainerGroupRequest resman_request;
    ::baidu::galaxy::sdk::ShowContainerGroupResponse resman_response;
    resman_request.user = user_;
    resman_request.id = jobid;
    bool show_container = false;
    ShowContainerParams show_container_param;
    show_container_param.action = this;
    show_container_param.ret = &show_container;
    show_container_param.request = resman_request;
    show_container_param.response = &resman_response;
    ::baidu::common::Thread container_thread;
    container_thread.Start(ShowContainerGroup, &show_container_param);

    job_thread.Join();
    container_thread.Join();
    
    if (!show_job || !show_container) {
        return false;
    }

    printf("base infomation\n"); 
    ::baidu::common::TPrinter base(4);
    base.AddRow(4, "id", "status", "create_time", "update_time");
    base.AddRow(4, response.job.jobid.c_str(),
                   StringJobStatus(response.job.status).c_str(),
                   FormatDate(response.job.create_time).c_str(),
                   FormatDate(response.job.update_time).c_str()
               );
    printf("%s\n", base.ToString().c_str());

    printf("job description base infomation\n");
    ::baidu::common::TPrinter desc_base(3);
    desc_base.AddRow(3, "name", "type", "run_user");
    desc_base.AddRow(3, response.job.desc.name.c_str(),
                        StringJobType(response.job.desc.type).c_str(),
                        response.job.desc.run_user.c_str()
                    );
    printf("%s\n", desc_base.ToString().c_str());
     
    printf("job description deploy infomation\n");
    std::string pools;
    for (size_t i = 0; i < response.job.desc.deploy.pools.size(); ++i) {
        pools += response.job.desc.deploy.pools[i] + ",";
    }
    ::baidu::common::TPrinter desc_deploy(6);
    desc_deploy.AddRow(6, "replica", "step", "interval", "max_per_host", "tag", "pools");
    desc_deploy.AddRow(6, ::baidu::common::NumToString(response.job.desc.deploy.replica).c_str(),
                        ::baidu::common::NumToString(response.job.desc.deploy.step).c_str(),
                        ::baidu::common::NumToString(response.job.desc.deploy.interval).c_str(),
                        ::baidu::common::NumToString(response.job.desc.deploy.max_per_host).c_str(),
                        response.job.desc.deploy.tag.c_str(),
                        pools.c_str()
                    );
     printf("%s\n", desc_deploy.ToString().c_str());

     printf("job description pod workspace_volum infomation\n");
     ::baidu::common::TPrinter desc_workspace_volum(7);
     desc_workspace_volum.AddRow(7, "size", "type", "medium", "dest_path", "readonly", "exclusive", "use_symlink");
     desc_workspace_volum.AddRow(7, ::baidu::common::HumanReadableString(response.job.desc.pod.workspace_volum.size).c_str(),
                                    StringVolumType(response.job.desc.pod.workspace_volum.type).c_str(),
                                    StringVolumMedium(response.job.desc.pod.workspace_volum.medium).c_str(),
                                    response.job.desc.pod.workspace_volum.dest_path.c_str(),
                                    StringBool(response.job.desc.pod.workspace_volum.readonly).c_str(),
                                    StringBool(response.job.desc.pod.workspace_volum.exclusive).c_str(),
                                    StringBool(response.job.desc.pod.workspace_volum.use_symlink).c_str()
                                );  
     printf("%s\n", desc_workspace_volum.ToString().c_str());

     printf("job description pod data_volums infomation\n");
     ::baidu::common::TPrinter desc_data_volums(7);
     desc_data_volums.AddRow(7, "size", "type", "medium", "dest_path", "readonly", "exclusive", "use_symlink");
     for (size_t i = 0; i < response.job.desc.pod.data_volums.size(); ++i) {
     desc_data_volums.AddRow(7, ::baidu::common::HumanReadableString(response.job.desc.pod.data_volums[i].size).c_str(),
                                    StringVolumType(response.job.desc.pod.data_volums[i].type).c_str(),
                                    StringVolumMedium(response.job.desc.pod.data_volums[i].medium).c_str(),
                                    response.job.desc.pod.data_volums[i].dest_path.c_str(),
                                    StringBool(response.job.desc.pod.data_volums[i].readonly).c_str(),
                                    StringBool(response.job.desc.pod.data_volums[i].exclusive).c_str(),
                                    StringBool(response.job.desc.pod.data_volums[i].use_symlink).c_str()
                                );  
     }
     printf("%s\n", desc_data_volums.ToString().c_str());

     printf("job description pod task infomation\n");
     for (uint32_t i = 0; i < response.job.desc.pod.tasks.size(); ++i) {
         printf("=========================================================\n");
         printf("job description pod task [%u] base infomation\n", i);
         ::baidu::common::TPrinter desc_task(7);
         desc_task.AddRow(7, "", "id", "cpu(cores/excess)", "memory(size/excess)", "tcp_throt(r/re/s/se)", "blkio", "ports(name/port)");
         std::string scpu = ::baidu::common::NumToString(response.job.desc.pod.tasks[i].cpu.milli_core / 1000.0) + "/"
                            + StringBool(response.job.desc.pod.tasks[i].cpu.excess);
         std::string smem = ::baidu::common::HumanReadableString(response.job.desc.pod.tasks[i].memory.size) + "/"
                            + StringBool(response.job.desc.pod.tasks[i].cpu.excess);
         std::string stcp = ::baidu::common::HumanReadableString(response.job.desc.pod.tasks[i].tcp_throt.recv_bps_quota) + "/"
                            + StringBool(response.job.desc.pod.tasks[i].tcp_throt.recv_bps_excess) + "/"
                            + ::baidu::common::HumanReadableString(response.job.desc.pod.tasks[i].tcp_throt.send_bps_quota) + "/"
                            + StringBool(response.job.desc.pod.tasks[i].tcp_throt.send_bps_excess);
         std::string sblkio;
         if (response.job.desc.pod.tasks[i].blkio.weight >= 0 && response.job.desc.pod.tasks[i].blkio.weight <= 1000) {
             sblkio = ::baidu::common::NumToString(response.job.desc.pod.tasks[i].blkio.weight);
         }

         for (uint32_t j = 0; j < response.job.desc.pod.tasks[i].ports.size(); ++j) {
             std::string sports = response.job.desc.pod.tasks[i].ports[j].port_name + "/"
                                 + response.job.desc.pod.tasks[i].ports[j].port;
                                 //+ response.job.desc.pod.tasks[i].ports[j].real_port;
             if (j == 0) {
                 desc_task.AddRow(7, ::baidu::common::NumToString(i).c_str(), 
                                       response.job.desc.pod.tasks[i].id.c_str(),
                                       scpu.c_str(),
                                       smem.c_str(),
                                       stcp.c_str(),
                                       sblkio.c_str(),
                                       sports.c_str()
                                  );
             } else {
                 desc_task.AddRow(7, "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     sports.c_str()
                                  );
             }

         }
         
         if (response.job.desc.pod.tasks[i].ports.size() == 0) {
             desc_task.AddRow(7, ::baidu::common::NumToString(i).c_str(), 
                                  response.job.desc.pod.tasks[i].id.c_str(), 
                                  scpu.c_str(),
                                  smem.c_str(),
                                  stcp.c_str(),
                                  sblkio.c_str(),
                                  ""
                              );
         }
         printf("%s\n", desc_task.ToString().c_str());

         printf("job description pod task [%u] exe_package infomation\n", i);
         printf("-----------------------------------------------\n");
         printf("start_cmd: %s\n\n", response.job.desc.pod.tasks[i].exe_package.start_cmd.c_str());
         printf("stop_cmd: %s\n\n", response.job.desc.pod.tasks[i].exe_package.stop_cmd.c_str());
         printf("dest_path: %s\n\n", response.job.desc.pod.tasks[i].exe_package.package.dest_path.c_str());
         printf("version: %s\n", response.job.desc.pod.tasks[i].exe_package.package.version.c_str());

         printf("\njob description pod task [%u] data_package infomation\n", i);
         printf("-----------------------------------------------\n");
         printf("reload_cmd: %s\n", response.job.desc.pod.tasks[i].data_package.reload_cmd.c_str());
         ::baidu::common::TPrinter packages(3);
         packages.AddRow(3, "", "version", "dest_path");
         for (uint32_t j = 0; j < response.job.desc.pod.tasks[i].data_package.packages.size(); ++j) {
              packages.AddRow(3, ::baidu::common::NumToString(j).c_str(),
                                 response.job.desc.pod.tasks[i].data_package.packages[j].version.c_str(),
                                 response.job.desc.pod.tasks[i].data_package.packages[j].dest_path.c_str()
                             );
         }
         printf("%s\n", packages.ToString().c_str());

         printf("job description pod task [%u] services infomation\n", i);
         ::baidu::common::TPrinter services(4);
         services.AddRow(4, "", "name", "port_name", "use_bns");
         for (uint32_t j = 0; j < response.job.desc.pod.tasks[i].services.size(); ++j) {
             services.AddRow(4, ::baidu::common::NumToString(j).c_str(),
                                response.job.desc.pod.tasks[i].services[j].service_name.c_str(),
                                response.job.desc.pod.tasks[i].services[j].port_name.c_str(),
                                StringBool(response.job.desc.pod.tasks[i].services[j].use_bns).c_str()
                            );
         }
         printf("%s\n", services.ToString().c_str());

     }

     std::map<std::string, ::baidu::galaxy::sdk::PodInfo> pods;
     for (size_t i = 0; i < response.job.pods.size(); ++i) {
         pods[response.job.pods[i].podid] = response.job.pods[i];
     }

     printf("podinfo infomation\n");
     std::string array_headers[8] = {"", "podid", "endpoint", "status", "fails", "container", "last_error", "update"};
     std::vector<std::string> headers(array_headers, array_headers + 8);

     if (find(options.begin(), options.end(), "cpu") != options.end()) {
         headers.push_back("cpu(a/u)");
     }

    if (find(options.begin(), options.end(), "mem") != options.end()) {
        headers.push_back("memory(a/u)");
    }

    if (find(options.begin(), options.end(), "volums") != options.end()) {
        headers.push_back("volums(med/a/u)");
    }

    if (options.size() == 0) {
        headers.push_back("cpu(a/u)");
        headers.push_back("memory(a/u)");
        headers.push_back("volums(med/a/u)");
    }

    headers.push_back("create_time");
    headers.push_back("update_time");

    ::baidu::common::TPrinter tp_pods(headers.size());
    tp_pods.AddRow(headers);
    for (uint32_t i = 0; i < resman_response.containers.size(); ++i) {
        size_t pos = resman_response.containers[i].id.rfind("."); 
        std::string podid(resman_response.containers[i].id, pos + 1, resman_response.containers[i].id.size()- (pos + 1));
                
        std::string scpu;
        if (options.size() == 0 || find(options.begin(), options.end(), "cpu") != options.end()) {
            scpu = ::baidu::common::NumToString(resman_response.containers[i].cpu.assigned / 1000.0) + "/" +
                   ::baidu::common::NumToString(resman_response.containers[i].cpu.used / 1000.0);
        }

        std::string smem;
        if (options.size() == 0 || find(options.begin(), options.end(), "mem") != options.end()) {
            smem = ::baidu::common::HumanReadableString(resman_response.containers[i].memory.assigned) + "/" +
                   ::baidu::common::HumanReadableString(resman_response.containers[i].memory.used);
        }

        std::string pod_fail_count;
        std::string pod_start_time;
        std::string pod_update_time;
        std::map<std::string, ::baidu::galaxy::sdk::PodInfo> ::iterator it = pods.find(resman_response.containers[i].id);
        std::string update_finish = "No";
        if (pods.find(resman_response.containers[i].id) != pods.end()) {
           pod_fail_count = ::baidu::common::NumToString(pods[resman_response.containers[i].id].fail_count);
           pod_start_time = FormatDate(pods[resman_response.containers[i].id].start_time);
           pod_update_time = FormatDate(pods[resman_response.containers[i].id].update_time);
           if (pods[resman_response.containers[i].id].update_time == response.job.update_time) {
               update_finish = "Yes";
           }
        }
        
        std::vector<std::string> values; 
        if (options.size() == 0 || find(options.begin(), options.end(), "volums") != options.end()) {
            for (uint32_t j = 0; j < resman_response.containers[i].volums.size(); ++j) {
                values.clear();
                std::string svolums;
                svolums = StringVolumMedium(resman_response.containers[i].volums[j].medium) + "/"
                          + ::baidu::common::HumanReadableString(resman_response.containers[i].volums[j].volum.assigned) + "/"
                          + ::baidu::common::HumanReadableString(resman_response.containers[i].volums[j].volum.used);
                if (j == 0) {
                    values.push_back(::baidu::common::NumToString(i));
                    values.push_back(podid);
                    values.push_back(resman_response.containers[i].endpoint);
                    values.push_back(StringPodStatus(pods[resman_response.containers[i].id].status));
                    values.push_back(pod_fail_count);
                    values.push_back(StringContainerStatus(resman_response.containers[i].status));
                    values.push_back(StringResourceError(resman_response.containers[i].last_res_err));
                    values.push_back(update_finish);
                    if (!scpu.empty()) {
                        values.push_back(scpu);
                    }
                    if (!smem.empty()) {
                        values.push_back(smem);
                    }
                    values.push_back(svolums);
                    values.push_back(pod_start_time);
                    values.push_back(pod_start_time);     
                } else {
                    int base_size = sizeof(array_headers) / sizeof(std::string);
                    for (int base_it = 0; base_it < base_size; ++base_it) {
                        values.push_back("");
                    }
                    if (!scpu.empty()) {
                        values.push_back("");
                    }
                    if (!smem.empty()) {
                        values.push_back("");
                    }
                    values.push_back(svolums);
                    values.push_back("");
                    values.push_back("");    
                }
                tp_pods.AddRow(values);
            }
            
            if (resman_response.containers[i].volums.size() == 0) {
                values.push_back(::baidu::common::NumToString(i));
                values.push_back(podid);
                values.push_back(resman_response.containers[i].endpoint);
                values.push_back(StringPodStatus(pods[resman_response.containers[i].id].status));
                values.push_back(pod_fail_count);
                values.push_back(StringContainerStatus(resman_response.containers[i].status));
                values.push_back(StringResourceError(resman_response.containers[i].last_res_err));
                values.push_back(update_finish);
                if (!scpu.empty()) {
                    values.push_back(scpu);
                }
                if (!smem.empty()) {
                    values.push_back(smem);
                }
                values.push_back("");
                values.push_back(pod_start_time);
                values.push_back(pod_update_time);  
                tp_pods.AddRow(values);
            }
        }

        if (options.size() != 0 && find(options.begin(), options.end(), "volums") == options.end()) {
            values.push_back(::baidu::common::NumToString(i));
            values.push_back(podid);
            values.push_back(resman_response.containers[i].endpoint);
            values.push_back(StringPodStatus(pods[resman_response.containers[i].id].status));
            values.push_back(pod_fail_count);
            values.push_back(StringContainerStatus(resman_response.containers[i].status));
            values.push_back(StringResourceError(resman_response.containers[i].last_res_err));
            values.push_back(update_finish);
            if (!scpu.empty()) {
                values.push_back(scpu);
            }
            if (!smem.empty()) {
                values.push_back(smem);
            }
            values.push_back(pod_start_time);
            values.push_back(pod_update_time);  
            tp_pods.AddRow(values);
        }
    }
    printf("%s\n", tp_pods.ToString().c_str());

    printf("services infomation\n");
    ::baidu::common::TPrinter services(6);
    services.AddRow(6, "", "podid", "service_addr", "name", "port", "status");

    for (uint32_t i = 0; i < resman_response.containers.size(); ++i) {
        size_t pos = resman_response.containers[i].id.rfind("."); 
        std::string podid(resman_response.containers[i].id, pos + 1, resman_response.containers[i].id.size()- (pos + 1));
        for (uint32_t j = 0; j < pods[resman_response.containers[i].id].services.size(); ++j) {
            if (j == 0) {
                services.AddRow(6, ::baidu::common::NumToString(i).c_str(),
                                    podid.c_str(),
                                    pods[resman_response.containers[i].id].services[j].ip.c_str(),
                                    pods[resman_response.containers[i].id].services[j].name.c_str(),
                                    pods[resman_response.containers[i].id].services[j].port.c_str(),
                                    StringStatus(pods[resman_response.containers[i].id].services[j].status).c_str()
                               );
            } else {
                services.AddRow(6, "",
                                   "",
                                   pods[resman_response.containers[i].id].services[j].ip.c_str(),
                                   pods[resman_response.containers[i].id].services[j].name.c_str(),
                                   pods[resman_response.containers[i].id].services[j].port.c_str(),
                                   StringStatus(pods[resman_response.containers[i].id].services[j].status).c_str()
                               ); 
            }
        }
    }
    printf("%s\n", services.ToString().c_str());

    return true;
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
    bool ret = app_master_->ExecuteCmd(request, &response);
    if (ret) {
        printf("Execute job %s\n success", jobid.c_str());
    } else {
        printf("Execute job %s failed for reason %s:%s\n", 
                jobid.c_str(), StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


/* vim: set ts=4 sw=4 sts=4 tw=100 */
