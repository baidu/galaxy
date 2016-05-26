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
    resman_ = new ::baidu::galaxy::sdk::ResourceManager();
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

bool JobAction::Init() {
    //用户名和token验证
    //

    //
    if (!app_master_->GetStub()) {
        return false;
    }

    if (!resman_->GetStub()) {
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
        printf("Submit job failed for reason %s:%s\n", StringStatus(response.error_code.status).c_str(), 
                    response.error_code.reason.c_str());
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

    baidu::galaxy::sdk::StopJobRequest request;
    baidu::galaxy::sdk::StopJobResponse response;
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

    baidu::galaxy::sdk::RemoveJobRequest request;
    baidu::galaxy::sdk::RemoveJobResponse response;
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

bool JobAction::ListJobs() {
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListContainerGroupsRequest resman_request;
    ::baidu::galaxy::sdk::ListContainerGroupsResponse resman_response;
    resman_request.user = user_;
    std::map<std::string, ::baidu::galaxy::sdk::ContainerGroupStatistics> containers;
    bool ret = resman_->ListContainerGroups(resman_request, &resman_response);
    if (ret) {
        for (uint32_t i = 0; i < resman_response.containers.size(); ++i) {
            containers[resman_response.containers[i].id] = resman_response.containers[i];
        }
        
    } else {
        printf("List container group failed for reason %s:%s\n",
                    StringStatus(resman_response.error_code.status).c_str(), resman_response.error_code.reason.c_str());
        return false;
    }

    ::baidu::galaxy::sdk::ListJobsRequest request;
    ::baidu::galaxy::sdk::ListJobsResponse response;
    request.user = user_;

    ret = app_master_->ListJobs(request, &response);
    if (ret) {
        baidu::common::TPrinter jobs(12);
        jobs.AddRow(12, "", "id", "name", "type","status", "stat(r/p/dep/dea/f)", "replica", 
                    "cpu(a/u)", "memory(a/u)", "volums(med/a/u)", "create", "update");
        for (uint32_t i = 0; i < response.jobs.size(); ++i) {
            std::string sstat = baidu::common::NumToString(response.jobs[i].running_num) + "/" +
                                baidu::common::NumToString(response.jobs[i].pending_num) + "/" +
                                baidu::common::NumToString(response.jobs[i].deploying_num) + "/" +
                                baidu::common::NumToString(response.jobs[i].death_num) + "/" +
                                baidu::common::NumToString(response.jobs[i].fail_count);
            std::string scpu;
            std::string smem;
            std::string svolums;
            std::map<std::string, ::baidu::galaxy::sdk::ContainerGroupStatistics>::iterator it 
                                        = containers.find(response.jobs[i].jobid);
            if (it != containers.end()) {
                scpu = ::baidu::common::NumToString(it->second.cpu.assigned / 1000.0) + "/"
                       + ::baidu::common::NumToString(it->second.cpu.used / 1000.0);
                smem = ::baidu::common::HumanReadableString(it->second.memory.assigned) + "/"
                       + ::baidu::common::HumanReadableString(it->second.memory.used);
                for (size_t j = 0; j < it->second.volums.size(); ++j) {
                    std::string svolums;
                    svolums = StringVolumMedium(it->second.volums[j].medium) + "/"
                              + ::baidu::common::HumanReadableString(it->second.volums[j].volum.assigned) + "/"
                              + ::baidu::common::HumanReadableString(it->second.volums[j].volum.used);
                    if (j == 0) {
                        jobs.AddRow(12, ::baidu::common::NumToString(i).c_str(),
                                         response.jobs[i].jobid.c_str(),
                                         response.jobs[i].desc.name.c_str(),
                                         StringJobType(response.jobs[i].desc.type).c_str(),
                                         StringJobStatus(response.jobs[i].status).c_str(),
                                         sstat.c_str(),
                                         ::baidu::common::NumToString(response.jobs[i].desc.deploy.replica).c_str(),
                                         scpu.c_str(),
                                         smem.c_str(),
                                         svolums.c_str(),
                                         FormatDate(response.jobs[i].create_time).c_str(),
                                         FormatDate(response.jobs[i].update_time).c_str()
                                   );

                    } else {
                        jobs.AddRow(12, "",
                                        "",
                                        "",
                                        "",
                                        "",
                                        "",
                                        "",
                                        "",
                                        "",
                                        svolums.c_str(),
                                        "",
                                        ""
                                    );
                    } 
                }
                if (it->second.volums.size() == 0) {
                    jobs.AddRow(12, ::baidu::common::NumToString(i).c_str(),
                                    response.jobs[i].jobid.c_str(),
                                    response.jobs[i].desc.name.c_str(),
                                    StringJobType(response.jobs[i].desc.type).c_str(),
                                    StringJobStatus(response.jobs[i].status).c_str(),
                                    sstat.c_str(),
                                    ::baidu::common::NumToString(response.jobs[i].desc.deploy.replica).c_str(),
                                    scpu.c_str(),
                                    smem.c_str(),
                                    "",
                                    FormatDate(response.jobs[i].create_time).c_str(),
                                    FormatDate(response.jobs[i].update_time).c_str()
                               );

                }
            } else {

                jobs.AddRow(12, ::baidu::common::NumToString(i).c_str(),
                                response.jobs[i].jobid.c_str(),
                                response.jobs[i].desc.name.c_str(),
                                StringJobType(response.jobs[i].desc.type).c_str(),
                                StringJobStatus(response.jobs[i].status).c_str(),
                                sstat.c_str(),
                                ::baidu::common::NumToString(response.jobs[i].desc.deploy.replica).c_str(),
                                "",
                                "",
                                "",
                                FormatDate(response.jobs[i].create_time).c_str(),
                                FormatDate(response.jobs[i].update_time).c_str()
                            );

            }
        }
        printf("%s\n", jobs.ToString().c_str());
    } else {
        printf("List job failed for reason %s:%s\n", 
                StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

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
       printf("base infomation\n"); 
       ::baidu::common::TPrinter base(5);
       base.AddRow(5, "id", "version", "status", "create_time", "update_time");
       base.AddRow(5, response.job.jobid.c_str(),
                      response.job.version.c_str(),
                      StringJobStatus(response.job.status).c_str(),
                      FormatDate(response.job.create_time).c_str(),
                      FormatDate(response.job.update_time).c_str()
                  );
       printf("%s\n", base.ToString().c_str());

       printf("job description base infomation\n");
       ::baidu::common::TPrinter desc_base(4);
       desc_base.AddRow(4, "name", "type", "version", "run_user");
       desc_base.AddRow(4, response.job.desc.name.c_str(),
                     StringJobType(response.job.desc.type).c_str(),
                     response.job.desc.version.c_str(),
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
        ::baidu::common::TPrinter desc_tasks(7);
        //desc_tasks.AddRow(10, "", "id", "cpu(cores/excess)", "memory(cores)", "tcp_throt", "blkio", "ports(name/port)", "exe_package", "data_package", "services");
        desc_tasks.AddRow(7, "", "id", "cpu(cores/excess)", "memory(size/excess)", "tcp_throt(r/re/s/se)", "blkio", "ports(name/port)");
        for (uint32_t i = 0; i < response.job.desc.pod.tasks.size(); ++i) {
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
                    desc_tasks.AddRow(7, ::baidu::common::NumToString(i).c_str(), 
                                          response.job.desc.pod.tasks[i].id.c_str(),
                                          scpu.c_str(),
                                          smem.c_str(),
                                          stcp.c_str(),
                                          sblkio.c_str(),
                                          sports.c_str()
                                     );
                } else {
                    desc_tasks.AddRow(7, "",
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
                desc_tasks.AddRow(7, ::baidu::common::NumToString(i).c_str(), 
                                     response.job.desc.pod.tasks[i].id.c_str(), 
                                     scpu.c_str(),
                                     smem.c_str(),
                                     stcp.c_str(),
                                     sblkio.c_str(),
                                     ""
                                 );
            }
            printf("%s\n", desc_tasks.ToString().c_str());

            printf("podinfo infomation\n");
            ::baidu::common::TPrinter pods(7);
            pods.AddRow(7, "", "podid", "endpoint", "status", "version", "start_time", "fail_count");
            for (uint32_t i = 0; i < response.job.pods.size(); ++i) {
                size_t pos = response.job.pods[i].podid.rfind("."); 
                std::string podid(response.job.pods[i].podid, pos + 1, response.job.pods[i].podid.size()- (pos + 1));
                pods.AddRow(7, ::baidu::common::NumToString(i).c_str(),
                               response.job.pods[i].podid.c_str(),
                               //response.job.pods[i].jobid.c_str(),
                               response.job.pods[i].endpoint.c_str(),
                               StringPodStatus(response.job.pods[i].status).c_str(),
                               response.job.pods[i].version.c_str(),
                               FormatDate(response.job.pods[i].start_time).c_str(),
                               ::baidu::common::NumToString(response.job.pods[i].fail_count).c_str()
                          );
            }
            printf("%s\n", pods.ToString().c_str());
        }

    } else {
        printf("Show job failed for reason %s:%s\n", StringStatus(response.error_code.status).c_str(), 
                        response.error_code.reason.c_str());
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
        printf("Execute job %s failed for reason %s:%s\n", 
                jobid.c_str(), StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


/* vim: set ts=4 sw=4 sts=4 tw=100 */
