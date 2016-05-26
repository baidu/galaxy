// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tprinter.h>
#include <boost/algorithm/string.hpp>
#include "galaxy_res_action.h"

//user
DECLARE_string(username);
DECLARE_string(token);

namespace baidu {
namespace galaxy {
namespace client {

ResAction::ResAction() { 
    resman_ = new ::baidu::galaxy::sdk::ResourceManager();
    user_.user =  FLAGS_username;
    user_.token = FLAGS_token;
}

ResAction::~ResAction() {
    if (NULL != resman_) {
        delete resman_;
    }
}

bool ResAction::Init() {
    //用户名和token验证
    //

    //
    if (!resman_->GetStub()) {
        return false;
    }
    return true;
}

bool ResAction::CreateContainerGroup(const std::string& json_file) {
    if (json_file.empty()) {
        fprintf(stderr, "json_file and jobid are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    
    baidu::galaxy::sdk::JobDescription job;
    int ok = BuildJobFromConfig(json_file, &job);
    if (ok != 0) {
        return false;
    }

    ::baidu::galaxy::sdk::CreateContainerGroupRequest request;
    ::baidu::galaxy::sdk::CreateContainerGroupResponse response;
    request.user = user_;
    request.replica = job.deploy.replica;
    request.name = job.name;
    request.desc.priority = job.type;
    request.desc.run_user = user_.user;
    request.desc.version = job.version;
    request.desc.max_per_host = job.deploy.max_per_host;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assign(job.pod.data_volums.begin(), job.pod.data_volums.end());
    //request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = job.deploy.tag;
    request.desc.pool_names.assign(job.deploy.pools.begin(), job.deploy.pools.end());
    
    for (uint32_t i = 0; i < job.pod.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        time_t timestamp;
        time(&timestamp);
        //cgroup.id = job.name + baidu::common::NumToString(timestamp);
        cgroup.id = ::baidu::common::NumToString(i);
        cgroup.cpu = job.pod.tasks[i].cpu;
        cgroup.memory = job.pod.tasks[i].memory;
        cgroup.tcp_throt = job.pod.tasks[i].tcp_throt;
        cgroup.blkio = job.pod.tasks[i].blkio;
        
        for (uint32_t j = 0; j < job.pod.tasks[i].ports.size(); ++j) {
            ::baidu::galaxy::sdk::PortRequired port;
            port.port_name = job.pod.tasks[i].ports[j].port_name;
            port.port = job.pod.tasks[i].ports[j].port;
            port.real_port = job.pod.tasks[i].ports[j].real_port;
            cgroup.ports.push_back(port);
        }

        request.desc.cgroups.push_back(cgroup);
    }

    bool ret = resman_->CreateContainerGroup(request, &response);
    if (ret) {
        printf("Create container group %s\n", response.id.c_str());
    } else {
        printf("Create container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::UpdateContainerGroup(const std::string& json_file, const std::string& id) {
    if (json_file.empty() || id.empty()) {
        fprintf(stderr, "json_file and id are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    
    baidu::galaxy::sdk::JobDescription job;
    int ok = BuildJobFromConfig(json_file, &job);
    if (ok != 0) {
        return false;
    }

    ::baidu::galaxy::sdk::UpdateContainerGroupRequest request;
    ::baidu::galaxy::sdk::UpdateContainerGroupResponse response;
    request.user = user_;
    request.replica = job.deploy.replica;
    request.id = id;
    request.interval = job.deploy.interval;
    request.desc.max_per_host = job.deploy.max_per_host;
    //request.name = job.name;
    request.desc.priority = job.type;
    request.desc.run_user = user_.user;
    request.desc.version = job.version;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assign(job.pod.data_volums.begin(), job.pod.data_volums.end());
    //request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = job.deploy.tag;
    request.desc.pool_names.assign(job.deploy.pools.begin(), job.deploy.pools.end());

    for (uint32_t i = 0; i < job.pod.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        cgroup.id = ::baidu::common::NumToString(i);
        cgroup.cpu = job.pod.tasks[i].cpu;
        cgroup.memory = job.pod.tasks[i].memory;
        cgroup.tcp_throt = job.pod.tasks[i].tcp_throt;
        cgroup.blkio = job.pod.tasks[i].blkio;
        
        for (uint32_t j = 0; j < job.pod.tasks[i].ports.size(); ++j) {
            ::baidu::galaxy::sdk::PortRequired port;
            port.port_name = job.pod.tasks[i].ports[j].port_name;
            port.port = job.pod.tasks[i].ports[j].port;
            port.real_port = job.pod.tasks[i].ports[j].real_port;
            cgroup.ports.push_back(port);
        }

        request.desc.cgroups.push_back(cgroup);
    }

    bool ret = resman_->UpdateContainerGroup(request, &response);
    if (ret) {
        printf("Update container group %s\n", id.c_str());
    } else {
        printf("Update container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::RemoveContainerGroup(const std::string& id) {
    if (id.empty()) {
        fprintf(stderr, "id is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::RemoveContainerGroupRequest request;
    ::baidu::galaxy::sdk::RemoveContainerGroupResponse response;
    request.user = user_;
    request.id = id;

    bool ret = resman_->RemoveContainerGroup(request, &response);
    if (ret) {
        printf("Remove container group %s\n", id.c_str());
    } else {
        printf("Remove container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::ListContainerGroups() {
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListContainerGroupsRequest request;
    ::baidu::galaxy::sdk::ListContainerGroupsResponse response;
    request.user = user_;

    bool ret = resman_->ListContainerGroups(request, &response);
    if (ret) {
        ::baidu::common::TPrinter containers(9);
        containers.AddRow(9, "", "id", "replica", "stat(r/a/p)", "cpu(a/u)", "mem(a/u)", "volums(med/a/u)", "create", "update");
        for (uint32_t i = 0; i < response.containers.size(); ++i) {
            std::string sstatus = ::baidu::common::NumToString(response.containers[i].ready) + "/" 
                                  +::baidu::common::NumToString(response.containers[i].allocating) + "/" +
                                  ::baidu::common::NumToString(response.containers[i].pending);

            std::string scpu = ::baidu::common::NumToString(response.containers[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.containers[i].cpu.used / 1000.0);

            std::string smem = ::baidu::common::HumanReadableString(response.containers[i].memory.assigned) + "/" +
                                ::baidu::common::HumanReadableString(response.containers[i].memory.used);

            for (size_t j = 0; j < response.containers[i].volums.size(); ++j) {
                std::string svolums;
                svolums = StringVolumMedium(response.containers[i].volums[j].medium) + "/"
                          + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.assigned) + "/"
                          + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.used);
                          //+ response.containers[i].volums[j].device_path;
                if (j == 0) {
                    containers.AddRow(9, baidu::common::NumToString(i).c_str(),
                                         response.containers[i].id.c_str(),
                                         ::baidu::common::NumToString(response.containers[i].replica).c_str(),
                                         sstatus.c_str(),
                                         scpu.c_str(),
                                         smem.c_str(),
                                         svolums.c_str(),
                                         FormatDate(response.containers[i].submit_time).c_str(),
                                         FormatDate(response.containers[i].update_time).c_str()
                                     );
                } else {
                    containers.AddRow(9, "",
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
            if (response.containers[i].volums.size() == 0) {
                containers.AddRow(9, baidu::common::NumToString(i).c_str(),
                                     response.containers[i].id.c_str(),
                                     ::baidu::common::NumToString(response.containers[i].replica).c_str(),
                                     sstatus.c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     "",
                                     FormatDate(response.containers[i].submit_time).c_str(),
                                     FormatDate(response.containers[i].update_time).c_str()
                                 );
            }
        }
        printf("%s\n", containers.ToString().c_str());
    } else {
        printf("List container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::ShowAgent(const std::string& endpoint) {
    if (endpoint.empty()) {
        fprintf(stderr, "endpoint is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::ShowAgentRequest request;
    ::baidu::galaxy::sdk::ShowAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint;

    bool ret = resman_->ShowAgent(request, &response);
    if (ret) {
        printf("containers infomation\n");
        ::baidu::common::TPrinter containers(8);
        containers.AddRow(8, "", "id", "endpoint", "status", "cpu(a/u)", "mem(a/u)", "volums(id/medium/a/u)", "last_error");
        for (uint32_t i = 0; i < response.containers.size(); ++i) {
            //size_t pos = response.containers[i].id.rfind("."); 
            //std::string id(response.containers[i].id, pos + 1, response.containers[i].id.size()- (pos + 1));

            //std::string scpu = ::baidu::common::NumToString(response.containers[i].cpu.total / 1000.0) + "/" +
            std::string scpu = ::baidu::common::NumToString(response.containers[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.containers[i].cpu.used / 1000.0);
             
            //std::string smem = ::baidu::common::HumanReadableString(response.containers[i].memory.total) + "/" +
            std::string smem = ::baidu::common::HumanReadableString(response.containers[i].memory.assigned) + "/" +
                               ::baidu::common::HumanReadableString(response.containers[i].memory.used);
                   
            for (uint32_t j = 0; j < response.containers[i].volums.size(); ++j) {
                std::string svolums;
                //svolums =  "vol_" + ::baidu::common::NumToString(j) + " " 
                svolums =  StringVolumMedium(response.containers[i].volums[j].medium) + "/" 
                            //+ ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.total) + "/"
                            + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.assigned) + "/"
                            + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.used) + " "
                            + response.containers[i].volums[j].device_path;
                if (j == 0) {
                    containers.AddRow(8, ::baidu::common::NumToString(i).c_str(),
                                         response.containers[i].id.c_str(),
                                         response.containers[i].endpoint.c_str(),
                                         StringContainerStatus(response.containers[i].status).c_str(),
                                         scpu.c_str(),
                                         smem.c_str(),
                                         svolums.c_str(),
                                         StringResourceError(response.containers[i].last_res_err).c_str()
                                     );

                } else {
                    containers.AddRow(8, "",
                                         "",
                                         "",
                                         "",
                                         "",
                                         "",
                                         svolums.c_str(),
                                         ""
                                     );
                }
            }

            if (response.containers[i].volums.size() == 0) {
                containers.AddRow(8, ::baidu::common::NumToString(i).c_str(),
                                     response.containers[i].id.c_str(),
                                     response.containers[i].endpoint.c_str(),
                                     ::baidu::common::NumToString(response.containers[i].status).c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     "",
                                     StringResourceError(response.containers[i].last_res_err).c_str()
                                 );
            }
        }
        printf("%s\n", containers.ToString().c_str());
    } else {
        printf("Show container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return false;
}

bool ResAction::ShowContainerGroup(const std::string& id) {
    if (id.empty()) {
        fprintf(stderr, "id is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::ShowContainerGroupRequest request;
    ::baidu::galaxy::sdk::ShowContainerGroupResponse response;
    request.user = user_;
    request.id = id;

    bool ret = resman_->ShowContainerGroup(request, &response);
    if (ret) {
        printf("base infomation\n");
        ::baidu::common::TPrinter base(7);
        base.AddRow(7, "user", "version", "priority", "cmd_line", "max_per_host", "tag", "pools");
        std::string pools; 
        for (size_t i = 0; i < response.desc.pool_names.size(); ++i) {
            pools += response.desc.pool_names[i] + ", ";
        }
        base.AddRow(7,  response.desc.run_user.c_str(),
                        response.desc.version.c_str(),
                        ::baidu::common::NumToString(response.desc.priority).c_str(),
                        response.desc.cmd_line.c_str(),
                        ::baidu::common::NumToString(response.desc.max_per_host).c_str(),
                        response.desc.tag.c_str(),
                        pools.c_str()
                   );

        printf("%s\n", base.ToString().c_str());

        printf("workspace volum infomation\n");
        ::baidu::common::TPrinter workspace_volum(8);
        workspace_volum.AddRow(8, "size", "type", "medium", "source_path", "dest_path", "readonly", "exclusive", "use_symlink");
        workspace_volum.AddRow(8, ::baidu::common::HumanReadableString(response.desc.workspace_volum.size).c_str(),
                                  StringVolumType(response.desc.workspace_volum.type).c_str(),
                                  StringVolumMedium(response.desc.workspace_volum.medium).c_str(),
                                  response.desc.workspace_volum.source_path.c_str(),
                                  response.desc.workspace_volum.dest_path.c_str(),
                                  StringBool(response.desc.workspace_volum.readonly).c_str(),
                                  StringBool(response.desc.workspace_volum.exclusive).c_str(),
                                  StringBool(response.desc.workspace_volum.use_symlink).c_str()
                               );
        printf("%s\n", workspace_volum.ToString().c_str());


        printf("data volums infomation\n");
         ::baidu::common::TPrinter data_volums(9);
         data_volums.AddRow(9, "", "size", "type", "medium", "source_path", "dest_path", "readonly", "exclusive", "use_symlink");

        for (uint32_t i = 0; i < response.desc.data_volums.size(); ++i) {
            data_volums.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                  ::baidu::common::HumanReadableString(response.desc.data_volums[i].size).c_str(),
                                  StringVolumType(response.desc.data_volums[i].type).c_str(),
                                  StringVolumMedium(response.desc.data_volums[i].medium).c_str(),
                                  response.desc.data_volums[i].source_path.c_str(),
                                  response.desc.data_volums[i].dest_path.c_str(),
                                  StringBool(response.desc.data_volums[i].readonly).c_str(),
                                  StringBool(response.desc.data_volums[i].exclusive).c_str(),
                                  StringBool(response.desc.data_volums[i].use_symlink).c_str()
                              );
        }
        printf("%s\n", data_volums.ToString().c_str());
        
        printf("cgroups infomation\n");
        ::baidu::common::TPrinter cgroups(11);
        cgroups.AddRow(11, "", "id", "cpu_cores", "cpu_excess", "mem_size", "mem_excess", "tcp_recv_bps", "tcp_recv_excess", "tcp_send_bps", "tcp_send_excess", "blk_weight");

        for (uint32_t i = 0; i < response.desc.cgroups.size(); ++i) {
            cgroups.AddRow(11, ::baidu::common::NumToString(i).c_str(),
                               response.desc.cgroups[i].id.c_str(),
                               ::baidu::common::NumToString(response.desc.cgroups[i].cpu.milli_core / 1000.0).c_str(),
                               StringBool(response.desc.cgroups[i].cpu.excess).c_str(),
                               ::baidu::common::HumanReadableString(response.desc.cgroups[i].memory.size).c_str(),
                               StringBool(response.desc.cgroups[i].memory.excess).c_str(),
                               ::baidu::common::HumanReadableString(response.desc.cgroups[i].tcp_throt.recv_bps_quota).c_str(),
                               StringBool(response.desc.cgroups[i].tcp_throt.recv_bps_excess).c_str(),
                               ::baidu::common::HumanReadableString(response.desc.cgroups[i].tcp_throt.send_bps_quota).c_str(),
                               StringBool(response.desc.cgroups[i].tcp_throt.send_bps_excess).c_str(),
                               ::baidu::common::NumToString(response.desc.cgroups[i].blkio.weight).c_str()
                            );
        }
        printf("%s\n", cgroups.ToString().c_str());
                
        printf("containers infomation\n");
        ::baidu::common::TPrinter containers(8);
        containers.AddRow(8, "", "id", "endpoint", "status", "cpu(a/u)", "mem(a/u)", "volums(id/medium/a/u)", "last_error");
        for (uint32_t i = 0; i < response.containers.size(); ++i) {
            size_t pos = response.containers[i].id.rfind("."); 
            std::string id(response.containers[i].id, pos + 1, response.containers[i].id.size()- (pos + 1));

            //std::string scpu = ::baidu::common::NumToString(response.containers[i].cpu.total / 1000.0) + "/" +
            std::string scpu = ::baidu::common::NumToString(response.containers[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.containers[i].cpu.used / 1000.0);
             
            //std::string smem = ::baidu::common::HumanReadableString(response.containers[i].memory.total) + "/" +
            std::string smem = ::baidu::common::HumanReadableString(response.containers[i].memory.assigned) + "/" +
                               ::baidu::common::HumanReadableString(response.containers[i].memory.used);
                   
            for (uint32_t j = 0; j < response.containers[i].volums.size(); ++j) {
                std::string svolums;
                svolums +=  "vol_" + ::baidu::common::NumToString(j) + " " 
                            + StringVolumMedium(response.containers[i].volums[j].medium) + " " 
                            //+ ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.total) + "/"
                            + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.assigned) + "/"
                            + ::baidu::common::HumanReadableString(response.containers[i].volums[j].volum.used) + " "
                            + response.containers[i].volums[j].device_path;
                if (j == 0) {
                    containers.AddRow(8, ::baidu::common::NumToString(i).c_str(),
                                         id.c_str(),
                                         response.containers[i].endpoint.c_str(),
                                         StringContainerStatus(response.containers[i].status).c_str(),
                                         scpu.c_str(),
                                         smem.c_str(),
                                         svolums.c_str(),
                                         StringResourceError(response.containers[i].last_res_err).c_str()
                                     );

                } else {
                    containers.AddRow(8, "",
                                         "",
                                         "",
                                         "",
                                         "",
                                         "",
                                         svolums.c_str(),
                                         ""
                                     );
                }
            }

            if (response.containers[i].volums.size() == 0) {
                containers.AddRow(8, ::baidu::common::NumToString(i).c_str(),
                                     id.c_str(),
                                     response.containers[i].endpoint.c_str(),
                                     ::baidu::common::NumToString(response.containers[i].status).c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     "",
                                     StringResourceError(response.containers[i].last_res_err).c_str()
                                 );
            }
        }
        printf("%s\n", containers.ToString().c_str());

    } else {
        printf("Show container group failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::AddAgent(const std::string& pool, const std::string& endpoint) {
    if (pool.empty() || endpoint.empty()) {
        return false;
    }
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::AddAgentRequest request;
    ::baidu::galaxy::sdk::AddAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint;
    request.pool = pool;

    bool ret = resman_->AddAgent(request, &response);
    if (ret) {
        printf("Add agent successfully\n");
    } else {
        printf("Add agent failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::RemoveAgent(const std::string& endpoint) {
    if (endpoint.empty()) {
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::RemoveAgentRequest request;
    ::baidu::galaxy::sdk::RemoveAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint;

    bool ret = resman_->RemoveAgent(request, &response);
    if (ret) {
        printf("Remove agent successfully\n");
    } else {
        printf("remove agent failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }

    return ret;

}

bool ResAction::ListAgents() {
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListAgentsRequest request;
    ::baidu::galaxy::sdk::ListAgentsResponse response;
    request.user = user_;

    bool ret = resman_->ListAgents(request, &response);
    if (ret) {
        ::baidu::common::TPrinter agents(9);
        agents.AddRow(9, "", "endpoint", "status", "pool", "tags", "cpu(t/a/u)", "mem(t/a/u)", "vol(med/t/a/u/path)", "containers");
        for (uint32_t i = 0; i < response.agents.size(); ++i) {
            std::string tags;
            for (uint32_t j = 0; j < response.agents[i].tags.size(); ++j) {
                tags += response.agents[i].tags[j] + ", ";
            }

            std::string scpu = ::baidu::common::NumToString(response.agents[i].cpu.total / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.used / 1000.0);

            std::string smem = ::baidu::common::HumanReadableString(response.agents[i].memory.total) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.assigned) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.used);
            
            for (uint32_t j = 0; j < response.agents[i].volums.size(); ++j) {
                std::string svolums;
                svolums +=  "vol_" + ::baidu::common::NumToString(j) + " "
                            + StringVolumMedium(response.agents[i].volums[j].medium) + " "
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.total) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.assigned) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.used) + " "
                            + response.agents[i].volums[j].device_path;
                if (j == 0) {
                    agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                     response.agents[i].endpoint.c_str(),
                                     StringAgentStatus(response.agents[i].status).c_str(),
                                     response.agents[i].pool.c_str(),
                                     tags.c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     svolums.c_str(),
                                     ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                                 );

                } else {
                    agents.AddRow(9, "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     svolums.c_str(),
                                     ""
                                 );

                }
            }

            if (response.agents[i].volums.size() == 0) {
                agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                 response.agents[i].endpoint.c_str(),
                                 ::baidu::common::NumToString(response.agents[i].status).c_str(),
                                 response.agents[i].pool.c_str(),
                                 tags.c_str(),
                                 scpu.c_str(),
                                 smem.c_str(),
                                 "",
                                 ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                              );

            }
        }
        printf("%s\n", agents.ToString().c_str());

    } else {
        printf("List agents failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }

    return ret;

}

bool ResAction::ListAgentsByTag(const std::string& tag) {
    if (tag.empty()) {
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListAgentsByTagRequest request;
    ::baidu::galaxy::sdk::ListAgentsByTagResponse response;
    request.user = user_;
    request.tag = tag;

    bool ret = resman_->ListAgentsByTag(request, &response);

    if (ret) {
        ::baidu::common::TPrinter agents(9);
        agents.AddRow(9, "", "endpoint", "status", "pool", "tags", "cpu(t/a/u)", "mem(t/a/u)", "vol(med/t/a/u/path)", "containers");
        for (uint32_t i = 0; i < response.agents.size(); ++i) {
            fprintf(stderr, "i is %u\n", i);
            std::string tags;
            for (uint32_t j = 0; j < response.agents[i].tags.size(); ++j) {
                tags += response.agents[i].tags[j] + ", ";
            }

            std::string scpu = ::baidu::common::NumToString(response.agents[i].cpu.total / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.used / 1000.0);

            std::string smem = ::baidu::common::HumanReadableString(response.agents[i].memory.total) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.assigned) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.used);
            
            for (uint32_t j = 0; j < response.agents[i].volums.size(); ++j) {
                std::string svolums;
                svolums +=  "vol_" + ::baidu::common::NumToString(j) + " "
                            + StringVolumMedium(response.agents[i].volums[j].medium) + " "
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.total) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.assigned) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.used) + " "
                            + response.agents[i].volums[j].device_path;
                if (j == 0) {
                    agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                     response.agents[i].endpoint.c_str(),
                                     ::baidu::common::NumToString(response.agents[i].status).c_str(),
                                     response.agents[i].pool.c_str(),
                                     tags.c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     svolums.c_str(),
                                     ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                                 );

                } else {
                    agents.AddRow(9, "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     svolums.c_str(),
                                     ""
                                 );

                }
            }

            if (response.agents[i].volums.size() == 0) {
                agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                 response.agents[i].endpoint.c_str(),
                                 ::baidu::common::NumToString(response.agents[i].status).c_str(),
                                 response.agents[i].pool.c_str(),
                                 tags.c_str(),
                                 scpu.c_str(),
                                 smem.c_str(),
                                 "",
                                 ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                              );

            }
        }
        printf("%s\n", agents.ToString().c_str());

    } else {
        printf("List agents failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }

    return ret;
}


bool ResAction::ListAgentsByPool(const std::string& pool) {
    if (pool.empty()) {
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListAgentsByPoolRequest request;
    ::baidu::galaxy::sdk::ListAgentsByPoolResponse response;
    request.user = user_;
    request.pool = pool;

    bool ret = resman_->ListAgentsByPool(request, &response);

    if (ret) {
        ::baidu::common::TPrinter agents(9);
        agents.AddRow(9, "", "endpoint", "status", "pool", "tags", "cpu(t/a/u)", "mem(t/a/u)", "vol(med/t/a/u/path)", "containers");
        for (uint32_t i = 0; i < response.agents.size(); ++i) {
            std::string tags;
            for (uint32_t j = 0; j < response.agents[i].tags.size(); ++j) {
                tags += response.agents[i].tags[j] + ", ";
            }

            std::string scpu = ::baidu::common::NumToString(response.agents[i].cpu.total / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.assigned / 1000.0) + "/" +
                               ::baidu::common::NumToString(response.agents[i].cpu.used / 1000.0);

            std::string smem = ::baidu::common::HumanReadableString(response.agents[i].memory.total) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.assigned) + "/" +
                               ::baidu::common::HumanReadableString(response.agents[i].memory.used);
            
            for (uint32_t j = 0; j < response.agents[i].volums.size(); ++j) {
                std::string svolums;
                svolums +=  "vol_" + ::baidu::common::NumToString(j) + " "
                            + StringVolumMedium(response.agents[i].volums[j].medium) + " "
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.total) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.assigned) + "/"
                            + ::baidu::common::HumanReadableString(response.agents[i].volums[j].volum.used) + " "
                            + response.agents[i].volums[j].device_path;
                if (j == 0) {
                    agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                     response.agents[i].endpoint.c_str(),
                                     ::baidu::common::NumToString(response.agents[i].status).c_str(),
                                     response.agents[i].pool.c_str(),
                                     tags.c_str(),
                                     scpu.c_str(),
                                     smem.c_str(),
                                     svolums.c_str(),
                                     ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                                 );

                } else {
                    agents.AddRow(9, "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     svolums.c_str(),
                                     ""
                                 );

                }
            }

            if (response.agents[i].volums.size() == 0) {
                agents.AddRow(9, ::baidu::common::NumToString(i).c_str(),
                                 response.agents[i].endpoint.c_str(),
                                 ::baidu::common::NumToString(response.agents[i].status).c_str(),
                                 response.agents[i].pool.c_str(),
                                 tags.c_str(),
                                 scpu.c_str(),
                                 smem.c_str(),
                                 "",
                                 ::baidu::common::NumToString(response.agents[i].total_containers).c_str()
                              );

            }
        }
        printf("%s\n", agents.ToString().c_str());

    } else {
        printf("List agents failed for reason %s:%s\n", 
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }

    return ret;
}

bool ResAction::EnterSafeMode() {

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::EnterSafeModeRequest request;
    ::baidu::galaxy::sdk::EnterSafeModeResponse response;
    request.user = user_;

    bool ret = resman_->EnterSafeMode(request, &response);
    if (ret) {
        printf("Enter safemode successfully");
    } else {
        printf("Enter safemode failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::LeaveSafeMode() {
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::LeaveSafeModeRequest request;
    ::baidu::galaxy::sdk::LeaveSafeModeResponse response;
    request.user = user_;

    bool ret = resman_->LeaveSafeMode(request, &response);
    if (ret) {
        printf("Leave safemode successfully");
    } else {
        printf("Leave safemode failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::OnlineAgent(const std::string& endpoint) {
    if (endpoint.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::OnlineAgentRequest request;
    ::baidu::galaxy::sdk::OnlineAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint; 

    bool ret = resman_->OnlineAgent(request, &response);
    if (ret) {
        printf("Online agent successfully");
    } else {
        printf("Online agent failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}
bool ResAction::OfflineAgent(const std::string& endpoint) {
    if (endpoint.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::OfflineAgentRequest request;
    ::baidu::galaxy::sdk::OfflineAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint; 

    bool ret = resman_->OfflineAgent(request, &response);
    if (ret) {
        printf("Offline agent successfully");
    } else {
        printf("Offline agent failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::Status() {
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::StatusRequest request;
    ::baidu::galaxy::sdk::StatusResponse response;
    request.user = user_;

    bool ret = resman_->Status(request, &response);
    if (ret) {
        printf("cluster agent infomation\n");
        ::baidu::common::TPrinter agent(3); 
        agent.AddRow(3, "total", "alive", "dead");
        agent.AddRow(3, baidu::common::NumToString(response.total_agents).c_str(), 
                        baidu::common::NumToString(response.alive_agents).c_str(),
                        baidu::common::NumToString(response.dead_agents).c_str());
        printf("%s\n", agent.ToString().c_str());

        printf("cluster cpu infomation\n");
        ::baidu::common::TPrinter cpu(3);
        cpu.AddRow(3, "total", "assigned", "used");
        cpu.AddRow(3, ::baidu::common::NumToString(response.cpu.total / 1000.0).c_str(), 
                        ::baidu::common::NumToString(response.cpu.assigned / 1000.0).c_str(),
                        ::baidu::common::NumToString(response.cpu.used / 1000.0).c_str());
        printf("%s\n", cpu.ToString().c_str());

        printf("cluster memory infomation\n");
        ::baidu::common::TPrinter mem(3);
        mem.AddRow(3, "total", "assigned", "used");
        mem.AddRow(3, ::baidu::common::HumanReadableString(response.memory.total).c_str(), 
                      ::baidu::common::HumanReadableString(response.memory.assigned).c_str(),
                      ::baidu::common::HumanReadableString(response.memory.used).c_str());
        printf("%s\n", mem.ToString().c_str());

        printf("cluster volumes infomation\n");
        ::baidu::common::TPrinter volum(6);
        volum.AddRow(6, "", "medium", "total", "assigned", "used", "device_path");
        for (uint32_t i = 0; i < response.volum.size(); ++i) {
            volum.AddRow(6, ::baidu::common::NumToString(i).c_str(),  
                            StringVolumMedium(response.volum[i].medium).c_str(), 
                            //::baidu::common::NumToString(response.volum[i].medium).c_str(), 
                            ::baidu::common::HumanReadableString(response.volum[i].volum.total).c_str(),
                            ::baidu::common::HumanReadableString(response.volum[i].volum.assigned).c_str(),
                            ::baidu::common::HumanReadableString(response.volum[i].volum.used).c_str(),
                            response.volum[i].device_path.c_str());
        }
        printf("%s\n", volum.ToString().c_str());

        printf("cluster pools infomation\n");
        ::baidu::common::TPrinter pool(4);
        pool.AddRow(4, "", "name", "total", "alive");
        for (uint32_t i = 0; i < response.pools.size(); ++i) {
            pool.AddRow(4, ::baidu::common::NumToString(i).c_str(),
                           response.pools[i].name.c_str(), 
                           ::baidu::common::NumToString(response.pools[i].total_agents).c_str(),
                           ::baidu::common::NumToString(response.pools[i].alive_agents).c_str());
        }
        printf("%s\n", pool.ToString().c_str());

        printf("cluster other infomation\n");
        ::baidu::common::TPrinter other(3);
        other.AddRow(3, "total_cgroups", "total_containers", "in_safe_mode");
        other.AddRow(3, ::baidu::common::NumToString(response.total_groups).c_str(), 
                        ::baidu::common::NumToString(response.total_containers).c_str(),
                        ::baidu::common::NumToString(response.in_safe_mode).c_str());
        printf("%s\n", other.ToString().c_str());

    } else {
        printf("Get Status failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::CreateTag(const std::string& tag, const std::string& file) {

    if (tag.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::CreateTagRequest request;
    ::baidu::galaxy::sdk::CreateTagResponse response;
    request.user = user_;
    request.tag = tag;

    if (!LoadAgentEndpointsFromFile(file, &request.endpoint)) {
        printf("load endpoint file failed\n");
        return false;
    }

    bool ret = resman_->CreateTag(request, &response);
    if (ret) {
        printf("Create tag successfully\n");
    } else {
        printf("Create tag failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::ListTags() {

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListTagsRequest request;
    ::baidu::galaxy::sdk::ListTagsResponse response;
    request.user = user_;

    bool ret = resman_->ListTags(request, &response);
    if (ret) {
        ::baidu::common::TPrinter tags(2);
        tags.AddRow(2, "", "tag");
        for (uint32_t i = 0; i < response.tags.size(); ++i) {
            tags.AddRow(2, ::baidu::common::NumToString(i).c_str(),
                           response.tags[i].c_str()
                       );
        }
        printf("%s\n", tags.ToString().c_str());

    } else {
        printf("List tags failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::GetPoolByAgent(const std::string& endpoint) {
    if (endpoint.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::GetPoolByAgentRequest request;
    ::baidu::galaxy::sdk::GetPoolByAgentResponse response;
    request.user = user_;
    request.endpoint = endpoint;

    bool ret = resman_->GetPoolByAgent(request, &response);
    if (ret) {
        printf("Pool is %s\n", response.pool.c_str());
    } else {
        printf("Get Pool failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::AddUser(const std::string& user, const std::string& token) {
    if (user.empty() || token.empty()) {
        return false;
    }
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::AddUserRequest request;
    ::baidu::galaxy::sdk::AddUserResponse response;
    request.admin = user_;
    request.user.user  = user;
    request.user.token = token;

    bool ret = resman_->AddUser(request, &response);
    if (ret) {
        printf("Add User Success\n");
    } else {
        printf("Add User failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;

}

bool ResAction::RemoveUser(const std::string& user, const std::string& token) {
    if (user.empty() || token.empty()) {
        return false;
    }
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::RemoveUserRequest request;
    ::baidu::galaxy::sdk::RemoveUserResponse response;
    request.admin = user_;
    request.user.user  = user;
    request.user.token = token;

    bool ret = resman_->RemoveUser(request, &response);
    if (ret) {
        printf("Remove User Success\n");
    } else {
        printf("Remove User failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::ListUsers() {
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListUsersRequest request;
    ::baidu::galaxy::sdk::ListUsersResponse response;
    request.user = user_;

    bool ret = resman_->ListUsers(request, &response);
    
    if (ret) {
        ::baidu::common::TPrinter users(2);
        users.AddRow(2, "", "user");
        for (uint32_t i = 0; i < response.user.size(); ++i) {
            users.AddRow(2, ::baidu::common::NumToString(i).c_str(),
                            response.user[i].c_str()
                        );
        }
        printf("%s\n", users.ToString().c_str());

    } else {
        printf("List users failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::ShowUser(const std::string& user, const std::string& token) {
    if (user.empty() || token.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ShowUserRequest request;
    ::baidu::galaxy::sdk::ShowUserResponse response;
    request.admin = user_;
    request.user.user = user;
    request.user.token = token;

    bool ret = resman_->ShowUser(request, &response);
    
    if (ret) {
        printf("pools infomation\n");
        ::baidu::common::TPrinter pools(2);
        pools.AddRow(2, "", "pool");
        for (uint32_t i = 0; i < response.pools.size(); ++i) {
            pools.AddRow(2, ::baidu::common::NumToString(i).c_str(),
                            response.pools[i].c_str()
                       );
        }
        printf("%s\n", pools.ToString().c_str());

        printf("authority infomation\n");
        ::baidu::common::TPrinter authority(2);
        pools.AddRow(2, "", "authority");
        for (uint32_t i = 0; i < response.authority.size(); ++i) {
            pools.AddRow(2, ::baidu::common::NumToString(i).c_str(),
                            response.authority[i]
                       );
        }
        printf("%s\n", authority.ToString().c_str());

        printf("quota infomation\n");
        ::baidu::common::TPrinter quota(5);
        quota.AddRow(5, "millicore", "memory", "disk", "ssd", "replica");
        quota.AddRow(5, ::baidu::common::NumToString(response.quota.millicore).c_str(),
                        ::baidu::common::HumanReadableString(response.quota.memory).c_str(),
                        ::baidu::common::HumanReadableString(response.quota.disk).c_str(),
                        ::baidu::common::HumanReadableString(response.quota.ssd).c_str(),
                        ::baidu::common::NumToString(response.quota.replica).c_str()
                    );
        printf("%s\n", quota.ToString().c_str());

        printf("assignd infomation\n");
        ::baidu::common::TPrinter assign(5);
        assign.AddRow(5, "millicore", "memory", "disk", "ssd", "replica");
        assign.AddRow(5, ::baidu::common::NumToString(response.assigned.millicore).c_str(),
                        ::baidu::common::HumanReadableString(response.assigned.memory).c_str(),
                        ::baidu::common::HumanReadableString(response.assigned.disk).c_str(),
                        ::baidu::common::HumanReadableString(response.assigned.ssd).c_str(),
                        ::baidu::common::NumToString(response.assigned.replica).c_str()
                    );
        printf("%s\n", assign.ToString().c_str());

    } else {
        printf("List users failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::GrantUser(const std::string& user, 
                          const std::string& token, 
                          const std::string& pool, 
                          const std::string& opration,
                          const std::string& authority) {
    
    if (user.empty() || token.empty() || pool.empty()) {
        return false;
    }

    if(!this->Init()) {
        return false;
    }
        
    ::baidu::galaxy::sdk::Grant grant;

    if (opration.compare("add") == 0) {
        grant.action = ::baidu::galaxy::sdk::kActionAdd; 
    } else if (opration.compare("remove") == 0){
        grant.action = ::baidu::galaxy::sdk::kActionRemove;
    } else if (opration.compare("set") == 0) { 
        grant.action = ::baidu::galaxy::sdk::kActionSet;
    } else if (opration.compare("clear") == 0) {
        grant.action = ::baidu::galaxy::sdk::kActionClear;
    } else {
        return false;
    }

    std::vector<std::string> authorities;
    boost::split(authorities, authority, boost::is_any_of(","));
    for (size_t i = 0; i < authorities.size(); ++i) {
        ::baidu::galaxy::sdk::Authority kAuthority;
        
        if (authorities[i].compare("create_container") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityCreateContainer;
        } else if (authorities[i].compare("remove_container") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityRemoveContainer;
        } else if (authorities[i].compare("update_container") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityUpdateContainer;
        } else if (authorities[i].compare("list_containers") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityListContainer;
        }else if (authorities[i].compare("submit_job") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthoritySubmitJob;
        }else if (authorities[i].compare("remove_job") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityRemoveJob;
        }else if (authorities[i].compare("update_job") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityUpdateJob;
        }else if (authorities[i].compare("list_jobs") == 0) {
            kAuthority = ::baidu::galaxy::sdk::kAuthorityListJobs;
        }else {
            return false;
        } 
        grant.authority.push_back(kAuthority);
    }
    grant.pool = pool;

    ::baidu::galaxy::sdk::GrantUserRequest request;
    ::baidu::galaxy::sdk::GrantUserResponse response;
    request.admin = user_;
    request.user.user  = user;
    request.user.token = token;
    request.grant = grant;

    bool ret = resman_->GrantUser(request, &response);
    if (ret) {
        printf("Grant User Success\n");
    } else {
        printf("Grant User failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
}

bool ResAction::AssignQuota(const std::string& user,
                 const std::string& token,
                 uint32_t millicores,
                 const std::string& memory,
                 const std::string& disk,
                 const std::string& ssd,
                 int replica
                 ) {

    if (user.empty() || token.empty()) {
        return false;
    }
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::Quota quota;
    quota.millicore = millicores;
    quota.replica = replica;
    if (UnitStringToByte(memory, &quota.memory)) {
        return false;
    }

    if (UnitStringToByte(disk, &quota.disk)) {
        return false;
    }

    if (UnitStringToByte(ssd, &quota.ssd)) {
        return false;
    }

    ::baidu::galaxy::sdk::AssignQuotaRequest request;
    ::baidu::galaxy::sdk::AssignQuotaResponse response;
    request.admin = user_;
    request.user.user  = user;
    request.user.token = token;
    request.quota = quota;

    bool ret = resman_->AssignQuota(request, &response);
    if (ret) {
        printf("Assign quota Success\n");
    } else {
        printf("Assign quota failed for reason %s:%s\n",
                    StringStatus(response.error_code.status).c_str(), response.error_code.reason.c_str());
    }
    return ret;
} 

} // end namespace client
} // end namespace galaxy
} // end namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
