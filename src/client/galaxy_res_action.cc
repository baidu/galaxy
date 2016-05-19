// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tprinter.h>
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
    //request.max_per_host = job.deploy.max_per_host;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assign(job.pod.data_volums.begin(), job.pod.data_volums.end());
    request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = "test";
    request.desc.pool_names.push_back("test");
    
    for (size_t i = 0; i < job.pod.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        time_t timestamp;
        time(&timestamp);
        cgroup.id = job.name + baidu::common::NumToString(timestamp);
        cgroup.cpu = job.pod.tasks[i].cpu;
        cgroup.memory = job.pod.tasks[i].memory;
        request.desc.cgroups.push_back(cgroup);
    }

    bool ret = resman_->CreateContainerGroup(request, &response);
    if (ret) {
        printf("Create container group %s\n", response.id.c_str());
    } else {
        printf("Create container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
    //request.max_per_host = job.deploy.max_per_host;
    //request.name = job.name;
    //request.desc.priority = job.type;
    request.desc.run_user = user_.user;
    request.desc.version = job.version;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assign(job.pod.data_volums.begin(), job.pod.data_volums.end());
    request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = "test";
    request.desc.pool_names.push_back("test");
    
    for (size_t i = 0; i < job.pod.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        cgroup.id = i;
        cgroup.cpu = job.pod.tasks[i].cpu;
        cgroup.memory = job.pod.tasks[i].memory;
        request.desc.cgroups.push_back(cgroup);
    }

    bool ret = resman_->UpdateContainerGroup(request, &response);
    if (ret) {
        printf("Update container group %s\n", id.c_str());
    } else {
        printf("Update container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Remove container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
        ::baidu::common::TPrinter tp(9);
        tp.AddRow(9, "", "id", "replica", "stat(p/d)", "cpu(t/a/u)", "mem(t/a/u)", "volums(med/t/a/u)", "create", "update");
        for (uint32_t i = 0; i < response.containers.size(); ++i) {
            std::vector<std::string> vs;
            vs.push_back(baidu::common::NumToString(i + 1));
            vs.push_back(response.containers[i].id);
            vs.push_back(::baidu::common::NumToString(response.containers[i].replica));
            vs.push_back(::baidu::common::NumToString(response.containers[i].pending) + "/" +
                         ::baidu::common::NumToString(response.containers[i].destroying)
                        );
            vs.push_back(::baidu::common::NumToString(response.containers[i].cpu.total) + "/" +
                         ::baidu::common::NumToString(response.containers[i].cpu.assigned) + "/" +
                         ::baidu::common::NumToString(response.containers[i].cpu.used)
                        );
            vs.push_back(::baidu::common::NumToString(response.containers[i].memory.total) + "/" +
                         ::baidu::common::NumToString(response.containers[i].memory.assigned) + "/" +
                         ::baidu::common::NumToString(response.containers[i].memory.used)
                        );

            std::string volums;
            for (size_t j = 0; j < response.containers[i].volums.size(); ++j) {
                volums += ::baidu::common::NumToString(response.containers[i].volums[j].medium) + "/";
                volums += ::baidu::common::NumToString(response.containers[i].volums[j].volum.total) + "/"; 
                volums += ::baidu::common::NumToString(response.containers[i].volums[j].volum.assigned) + "/"; 
                volums += ::baidu::common::NumToString(response.containers[i].volums[j].volum.used) + "/"; 
                volums += response.containers[i].volums[j].device_path + "\n";
            }
            vs.push_back(volums);
            vs.push_back(FormatDate(response.containers[i].submit_time));
            vs.push_back(FormatDate(response.containers[i].update_time));
            
            tp.AddRow(vs);
        }
        printf("%s\n", tp.ToString().c_str());
    } else {
        printf("List container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
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
        fprintf(stdout, "run_user: %s\n", response.desc.run_user.c_str());
        fprintf(stdout, "version: %s\n", response.desc.version.c_str());
        fprintf(stdout, "priority: %d\n", response.desc.priority);
        fprintf(stdout, "cmd_line: %s\n", response.desc.cmd_line.c_str());
        fprintf(stdout, "max_per_host: %d\n", response.desc.max_per_host);
        fprintf(stdout, "tag: %s\n", response.desc.tag.c_str());
        std::string pools;
        for (size_t i = 0; i < response.desc.pool_names.size(); ++i) {
            pools += response.desc.pool_names[i] + ", ";
        }
        fprintf(stdout, "pool_names: %s\n", pools.c_str());
        fprintf(stdout, "workspace_volum size: %d\n", response.desc.workspace_volum.size);
        fprintf(stdout, "workspace_volum type: %d\n", response.desc.workspace_volum.type);
        fprintf(stdout, "workspace_volum medium: %d\n", response.desc.workspace_volum.medium);
        fprintf(stdout, "workspace_volum source_path: %s\n", response.desc.workspace_volum.source_path.c_str());
        fprintf(stdout, "workspace_volum dest_path: %s\n", response.desc.workspace_volum.dest_path.c_str());
        fprintf(stdout, "workspace_volum readonly: %d\n", response.desc.workspace_volum.readonly);
        fprintf(stdout, "workspace_volum exclusive: %d\n", response.desc.workspace_volum.exclusive);
        fprintf(stdout, "workspace_volum use_symlink: %d\n", response.desc.workspace_volum.use_symlink);

        fprintf(stdout, "data_volums[]:\n");
        for (size_t i = 0; i < response.desc.data_volums.size(); ++i) {
            fprintf(stdout, "   data_volums[%u]\n", i);
            fprintf(stdout, "   data_volum size: %d\n", response.desc.data_volums[i].size);
            fprintf(stdout, "   data_volum type: %d\n", response.desc.data_volums[i].type);
            fprintf(stdout, "   data_volum medium: %d\n", response.desc.data_volums[i].medium);
            fprintf(stdout, "   data_volum source_path: %s\n", response.desc.data_volums[i].source_path.c_str());
            fprintf(stdout, "   data_volum dest_path: %s\n", response.desc.data_volums[i].dest_path.c_str());
            fprintf(stdout, "   data_volum readonly: %d\n", response.desc.data_volums[i].readonly);
            fprintf(stdout, "   data_volum exclusive: %d\n", response.desc.data_volums[i].exclusive);
            fprintf(stdout, "   data_volum use_symlink: %d\n", response.desc.data_volums[i].use_symlink);
            fprintf(stdout, "\n");
        }
        
        fprintf(stdout, "cgroups[]:\n");
        for (size_t i = 0; i < response.desc.cgroups.size(); ++i) {
            fprintf(stdout, "   cgroups[%u]\n", i);
            fprintf(stdout, "   cgroup id: %s\n", response.desc.cgroups[i].id.c_str());
            fprintf(stdout, "   cgroup cpu millcores: %d\n", response.desc.cgroups[i].cpu.milli_core);
            fprintf(stdout, "   cgroup cpu excess: %d\n", response.desc.cgroups[i].cpu.excess);
            fprintf(stdout, "   cgroup memory size: %d\n", response.desc.cgroups[i].memory.size);
            fprintf(stdout, "   cgroup memory excess: %d\n", response.desc.cgroups[i].memory.excess);

            fprintf(stdout, "   tcp_throt recv_bps_quota %d:\n", response.desc.cgroups[i].tcp_throt.recv_bps_quota);
            fprintf(stdout, "   tcp_throt recv_bps_excess %d:\n", response.desc.cgroups[i].tcp_throt.recv_bps_excess);
            fprintf(stdout, "   tcp_throt send_bps_quota %d:\n", response.desc.cgroups[i].tcp_throt.send_bps_quota);
            fprintf(stdout, "   tcp_throt send_bps_excess %d:\n", response.desc.cgroups[i].tcp_throt.send_bps_excess);

            fprintf(stdout, "   blkio weight %d:\n", response.desc.cgroups[i].blkio.weight);

            fprintf(stdout, "\n");
        }

                
        fprintf(stdout, "containers[]:\n");
        for (size_t i = 0; i < response.containers.size(); ++i) {
            fprintf(stdout, "   containers[%u]\n", i);
            fprintf(stdout, "   container status: %d\n", response.containers[i].status);
            fprintf(stdout, "   container endpoint: %d\n", response.containers[i].endpoint.c_str());
            fprintf(stdout, "   container cpu total: %d\n", response.containers[i].cpu.total);
            fprintf(stdout, "   container cpu assigned: %d\n", response.containers[i].cpu.assigned);
            fprintf(stdout, "   container cpu used: %d\n", response.containers[i].cpu.used);
            fprintf(stdout, "   container mem total: %d\n", response.containers[i].memory.total);
            fprintf(stdout, "   container mem assigned: %d\n", response.containers[i].memory.assigned);
            fprintf(stdout, "   container mem used: %d\n", response.containers[i].memory.used);

            for (size_t j = 0; j < response.containers[i].volums.size(); ++j) {
                fprintf(stdout, "   container volums[%u]\n", j);
                fprintf(stdout, "       container volum medium : %d\n", response.containers[i].volums[j].medium);
                fprintf(stdout, "       container volum device_path : %s\n", response.containers[i].volums[j].device_path.c_str());
                fprintf(stdout, "       container volum total: %d\n", response.containers[i].volums[j].volum.total);
                fprintf(stdout, "       container volum assigned: %d\n", response.containers[i].volums[j].volum.assigned);
                fprintf(stdout, "       container volum used: %d\n", response.containers[i].volums[j].volum.used);
            }

            fprintf(stdout, "\n");
        }


    } else {
        printf("Show container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Add agent failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("remove agent failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
    }

    return ret;

}
bool ResAction::ListAgents(const std::string& pool) {
    if (pool.empty()) {
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListAgentsRequest request;
    ::baidu::galaxy::sdk::ListAgentsResponse response;
    request.user = user_;
    request.pool = pool;

    bool ret = resman_->ListAgents(request, &response);
    if (ret) {
        ::baidu::common::TPrinter tp(11);
        tp.AddRow(11, "", "endpoint", "status", "pool", "tags", "cpu(t/a/u)", "mem(t/a/u)", "vol_type", "vol(t/a/u)", "vol_path","total_containers");
        for (uint32_t i = 0; i < response.agents.size(); ++i) {
            std::vector<std::string> vs;
            vs.push_back(baidu::common::NumToString(i + 1));
            vs.push_back(response.agents[i].endpoint);
            vs.push_back(baidu::common::NumToString(response.agents[i].status));
            vs.push_back(response.agents[i].pool);
            std::string tags;
            for (size_t j = 0; j < response.agents[i].tags.size(); ++j) {
                tags += response.agents[i].tags[j] + ", ";
            }

            vs.push_back(tags);

            vs.push_back(baidu::common::NumToString(response.agents[i].cpu.total) + "/" +
                         baidu::common::NumToString(response.agents[i].cpu.assigned) + "/" +
                        baidu::common::NumToString(response.agents[i].cpu.used)
                        );
            vs.push_back(baidu::common::NumToString(response.agents[i].memory.total) + "/" +
                         baidu::common::NumToString(response.agents[i].memory.assigned) + "/" +
                        baidu::common::NumToString(response.agents[i].memory.used)
                        );
            std::string volums;
            for (size_t j = 0; j < response.agents[i].volums.size(); ++j) {
                volums += ::baidu::common::NumToString(response.agents[i].volums[j].medium) + ":";
                volums += ::baidu::common::NumToString(response.agents[i].volums[j].volum.total) + ":";
                volums += ::baidu::common::NumToString(response.agents[i].volums[j].volum.assigned) + ":";
                volums += ::baidu::common::NumToString(response.agents[i].volums[j].volum.used) + ":"; 
                volums += response.agents[i].volums[j].device_path;
            }
            vs.push_back(volums);
            vs.push_back(baidu::common::NumToString(response.agents[i].total_containers));
            tp.AddRow(vs);
        }
        printf("%s\n", tp.ToString().c_str());

    } else {
        printf("List agents failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Enter safemode failed for reason %d:%s\n",
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Leave safemode failed for reason %d:%s\n",
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Online agent failed for reason %d:%s\n",
                    response.error_code.status, response.error_code.reason.c_str());
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
        printf("Offline agent failed for reason %d:%s\n",
                    response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu

/* vim: set ts=4 sw=4 sts=4 tw=100 */
