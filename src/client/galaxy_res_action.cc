// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gflags/gflags.h>
//#include "ins_sdk.h"
#include <tprinter.h>
#include "galaxy_res_action.h"

namespace baidu {
namespace galaxy {
namespace client {

ResAction::ResAction(const std::string& name, const std::string& token, const std::string& nexus_key) { 
    resman_ = new ::baidu::galaxy::sdk::ResourceManager(nexus_key);
    user_. user =  user;
    user_. token = token;
}

Action::~Action() {
    if (NULL != resman_) {
        delete resman_;
    }

}

bool Action::Init() {
    //用户名和token验证
    //

    //
    if (!resman_->GetStub()) {
        return false;
    }
    return true;
}

bool CreateContainerGroup(const std::string& json_file) {
    if (json_file.empty()) {
        fprintf(stderr, "json_file and jobid are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    
    baidu::galaxy::sdk::JobDescription job;
    int ok = baidu::galaxy::client::BuildJobFromConfig(json_file, &job);
    if (ok != 0) {
        return false;
    }

    ::baidu::galaxy::sdk::CreateContainerGroupRequest request;
    ::baidu::galaxy::sdk::CreateContainerGroupResponse responce;
    request.user = user_;
    request.replica = job.deploy.replica;
    request.max_per_host = job.deploy.max_per_host;
    request.name = job.name;
    request.desc.priority = job.type;
    request.desc.run_user = user_.user;
    request.desc.version = job.version;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assgin(job.pod.datavolums.begin(), job.pod.datavolums.end());
    request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = "test";
    request.desc.pool_names.push_back("test");
    
    for (size_t i = 0; i < job.pod_desc.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        cgroup.id = j;
        cgroup.cpu = job.pod_desc.tasks[i].cpu;
        cgroup.memory = job.pod_desc.tasks[i].memory;
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

bool UpdateContainerGroup(const std::string& json_file, const std::string& id) {
    if (json_file.empty() || id.empty()) {
        fprintf(stderr, "json_file and id are needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    
    baidu::galaxy::sdk::JobDescription job;
    int ok = baidu::galaxy::client::BuildJobFromConfig(json_file, &job);
    if (ok != 0) {
        return false;
    }

    ::baidu::galaxy::sdk::UpdateContainerGroupRequest request;
    ::baidu::galaxy::sdk::UpdateContainerGroupResponse responce;
    request.user = user_;
    request.replica = job.deploy.replica;
    request.id = id;
    request.interval = interval;
    //request.max_per_host = job.deploy.max_per_host;
    //request.name = job.name;
    //request.desc.priority = job.type;
    request.desc.run_user = user_.user;
    request.desc.version = job.version;
    request.desc.workspace_volum = job.pod.workspace_volum;
    request.desc.data_volums.assgin(job.pod.datavolums.begin(), job.pod.datavolums.end());
    request.desc.cmd_line = "sh appworker.sh";
    request.desc.tag = "test";
    request.desc.pool_names.push_back("test");
    
    for (size_t i = 0; i < job.pod_desc.tasks.size(); ++i) {
        ::baidu::galaxy::sdk::Cgroup cgroup;
        cgroup.id = j;
        cgroup.cpu = job.pod_desc.tasks[i].cpu;
        cgroup.memory = job.pod_desc.tasks[i].memory;
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

bool RemoveContainerGroup(const std::string& id) {
    if (id.empty()) {
        fprintf(stderr, "id is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::RemoveContainerGroupRequest request;
    ::baidu::galaxy::sdk::RemoveContainerGroupResponse responce;
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

bool ListContainerGroups() {
    if(!this->Init()) {
        return false;
    }

    ::baidu::galaxy::sdk::ListContainerGroupsRequest request;
    ::baidu::galaxy::sdk::ListContainerGroupsResponse responce;
    request.user = user_;

    bool ret = resman_->ListContainerGroups(request, &response);
    if (ret) {
        baidu::common::TPrinter tp(12);
        tp.AddRow(11, "", "id", "replica", "stat(p/d)", "cpu(t/a/u)", "mem(t/a/u)", "vol_type", "vol(t/a/u)", "vol_path","create", "update");
        for (size_t i = 0; i < response.containers.size(); ++i) {
            std::vector<std::string> vs;
            vs.push_back(baidu::common::NumToString(i + 1));
            vs.push_back(baidu::common::NumToString(response.containers[i].id));
            vs.push_back(baidu::common::NumToString(response.containers[i].replica));
            vs.push_back(baidu::common::NumToString(response.containers[i].pengding));
            vs.push_back(baidu::common::NumToString(response.containers[i].destroying));
            vs.push_back(baidu::common::NumToString(response.containers[i].cpu.total) + "/" +
                         baidu::common::NumToString(response.containers[i].cpu.assigned) + "/" +
                        baidu::common::NumToString(response.containers[i].cpu.used)
                        );
            vs.push_back(baidu::common::NumToString(response.containers[i].memroy.total) + "/" +
                         baidu::common::NumToString(response.containers[i].memroy.assigned) + "/" +
                        baidu::common::NumToString(response.containers[i].memroy.used)
                        );
            vs.push_back(baidu::common::NumToString(response.containers[i].volum.medium));
            vs.push_back(baidu::common::NumToString(response.containers[i].volum.volum.total) + "/" +
                         baidu::common::NumToString(response.containers[i].volum.volum.assigned) + "/" +
                         baidu::common::NumToString(response.containers[i].volum.volum.used)
                        );
            vs.push_back(response.containers[i].volum.device_path);
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

bool ShowContainerGroup(const std::string& id) {
    if (id.empty()) {
        fprintf(stderr, "id is needed\n");
        return false;
    }
    
    if(!this->Init()) {
        return false;
    }
    ::baidu::galaxy::sdk::ShowContainerGroupRequest request;
    ::baidu::galaxy::sdk::ShowContainerGroupResponse responce;
    request.user = user_;
    request.id = id;

    bool ret = resman_->ShowContainerGroup(request, &response);
    if (ret) {
        fprintf(stderr, "run_user: %s\n", response.desc.run_user.c_str());
        fprintf(stderr, "version: %s\n", response.desc.version.c_str());
        fprintf(stderr, "priority: %d\n", response.desc.priority);
        fprintf(stderr, "cmd_line: %s\n", response.desc.cmd_line.c_str());
        fprintf(stderr, "max_per_host: %d\n", response.desc.max_per_host);
        fprintf(stderr, "tag: %s\n", response.desc.tag.c_str());
        std::string pools;
        for (size_t i = 0; i < response.desc.pool_names.size(); ++i) {
            pools += response.desc.pool_names[i] + ", ";
        }
        fprintf(stderr, "pool_names: %s\n", pools.c_str());
        fprintf(stderr, "workspace_volum size: %d\n", response.desc.workspace_volum.size);
        fprintf(stderr, "workspace_volum type: %d\n", response.desc.workspace_volum.type);
        fprintf(stderr, "workspace_volum medium: %d\n", response.desc.workspace_volum.medium);
        fprintf(stderr, "workspace_volum source_path: %s\n", response.desc.workspace_volum.source_path.c_str());
        fprintf(stderr, "workspace_volum dest_path: %s\n", response.desc.workspace_volum.dest_path.c_str());
        fprintf(stderr, "workspace_volum readonly: %d\n", response.desc.workspace_volum.readonly);
        fprintf(stderr, "workspace_volum exclusive: %d\n", response.desc.workspace_volum.exclusive);
        fprintf(stderr, "workspace_volum use_symlink: %d\n", response.desc.workspace_volum.use_symlink);

        fprintf(stderr, "data_volums[]:\n");
        for (size_t i = 0; i < response.desc.data_volums.size(); ++i) {
            fprintf(stderr, "data_volums[%u]\n", i);
            fprintf(stderr, "data_volum size: %d\n", response.desc.response.desc.data_volums[i].size);
            fprintf(stderr, "data_volum type: %d\n", response.desc.response.desc.data_volums[i].type);
            fprintf(stderr, "data_volum medium: %d\n", response.desc.response.desc.data_volums[i].medium);
            fprintf(stderr, "data_volum source_path: %s\n", response.desc.response.desc.data_volums[i].source_path.c_str());
            fprintf(stderr, "data_volum dest_path: %s\n", response.desc.response.desc.data_volums[i].dest_path.c_str());
            fprintf(stderr, "data_volum readonly: %d\n", response.desc.response.desc.data_volums[i].readonly);
            fprintf(stderr, "data_volum exclusive: %d\n", response.desc.response.desc.data_volums[i].exclusive);
            fprintf(stderr, "data_volum use_symlink: %d\n", response.desc.response.desc.data_volums[i].use_symlink);
            fprintf(stderr, "\n");
        }
        
        fprintf(stderr, "cgroups[]:\n");
        for (size_t i = 0; i < response.desc.cgroups.size(); ++i) {
            fprintf(stderr, "cgroups[%u]\n", i);
            fprintf(stderr, "cgroup id: %s\n", response.desc.workspace_volum.id.c_str());
            fprintf(stderr, "cgroup cpu millcores: %d\n", response.desc.cgroups[i].cpu.milli_core);
            fprintf(stderr, "cgroup cpu excess: %d\n", response.desc.cgroups[i].cpu.excess);
            fprintf(stderr, "cgroup memory size: %d\n", response.desc.cgroups[i].memory.size);
            fprintf(stderr, "cgroup memory excess: %d\n", response.desc.cgroups[i].memory.excess);

            fprintf(stderr, "\n");
        }

        fprintf(stderr, "tcp_throt recv_bps_quota %d:\n", response.desc.cgroups.tcp_throt.recv_bps_quota);
        fprintf(stderr, "tcp_throt recv_bps_excess %d:\n", response.desc.cgroups.tcp_throt.recv_bps_excess);
        fprintf(stderr, "tcp_throt send_bps_quota %d:\n", response.desc.cgroups.tcp_throt.send_bps_quota);
        fprintf(stderr, "tcp_throt send_bps_excess %d:\n", response.desc.cgroups.tcp_throt.send_bps_excess);

        fprintf(stderr, "tcp_throt weight %d:\n", response.desc.cgroups.blkio.weight);
        
        fprintf(stderr, "containers[]:\n");
        for (size_t i = 0; i < response.desc.containers.size(); ++i) {
            fprintf(stderr, "containers[%u]\n", i);
            fprintf(stderr, "container status: %d\n", response.desc.containers[i].status);
            fprintf(stderr, "container endpoint: %d\n", response.desc.containers[i].endpoint.c_str());
            fprintf(stderr, "container cpu total: %d\n", response.desc.containers[i].cpu.total);
            fprintf(stderr, "container cpu assigned: %d\n", response.desc.containers[i].cpu.assigned);
            fprintf(stderr, "container cpu used: %d\n", response.desc.containers[i].cpu.used);
            fprintf(stderr, "container mem total: %d\n", response.desc.containers[i].mem.total);
            fprintf(stderr, "container mem assigned: %d\n", response.desc.containers[i].mem.assigned);
            fprintf(stderr, "container mem used: %d\n", response.desc.containers[i].mem.used);

            fprintf(stderr, "container volum medium: %d\n", response.desc.containers[i].volum.medium);
            fprintf(stderr, "container volum device_path: %s\n", response.desc.containers[i].volum.device_path.c_str());
            fprintf(stderr, "container volum total: %d\n", response.desc.containers[i].volum.volum.total);
            fprintf(stderr, "container volum assigned: %d\n", response.desc.containers[i].volum.volum.assigned);
            fprintf(stderr, "container volum used: %d\n", response.desc.containers[i].volum.volum.used);

            fprintf(stderr, "\n");
        }


    } else {
        printf("Show container group failed for reason %d:%s\n", 
                    response.error_code.status, response.error_code.reason.c_str());
    }
    return ret;

}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu


/* vim: set ts=4 sw=4 sts=4 tw=100 */
