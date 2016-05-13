// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include "client/galaxy_util.h"

int main(int argc, char** argv) {
    baidu::galaxy::sdk::JobDescription job;
    int ok = baidu::galaxy::client::BuildJobFromConfig("./app.json", &job);
    
    std::cout << job.name << std::endl;
    std::cout << job.type << std::endl;
    std::cout << job.version << std::endl;
    std::cout << job.deploy.replica << std::endl;
    std::cout << job.deploy.step << std::endl;
    std::cout << job.deploy.interval << std::endl;
    std::cout << job.deploy.max_per_host << std::endl;

    std::cout << job.pod_desc.workspace_volum.size << std::endl;
    std::cout << job.pod_desc.workspace_volum.type << std::endl;
    std::cout << job.pod_desc.workspace_volum.medium << std::endl;
    std::cout << job.pod_desc.workspace_volum.source_path << std::endl;
    std::cout << job.pod_desc.workspace_volum.dest_path << std::endl;
    std::cout << job.pod_desc.workspace_volum.readonly << std::endl;
    std::cout << job.pod_desc.workspace_volum.exclusive << std::endl;
    std::cout << job.pod_desc.workspace_volum.use_symlink << std::endl;

    for (size_t i = 0; i< job.pod_desc.data_volums.size(); ++i) {
        std::cout << "\ndata_volums No." << i << std::endl;
        std::cout << job.pod_desc.data_volums[i].size << std::endl;
        std::cout << job.pod_desc.data_volums[i].type << std::endl;
        std::cout << job.pod_desc.data_volums[i].medium << std::endl;
        std::cout << job.pod_desc.data_volums[i].source_path << std::endl;
        std::cout << job.pod_desc.data_volums[i].dest_path << std::endl;
        std::cout << job.pod_desc.data_volums[i].readonly << std::endl;
        std::cout << job.pod_desc.data_volums[i].exclusive << std::endl;
        std::cout << job.pod_desc.data_volums[i].use_symlink << std::endl;

    }

    std::cout << "task num is " << job.pod_desc.tasks.size() << std::endl;
    for (size_t i = 0; i < job.pod_desc.tasks.size(); ++i) {
        std::cout << "\ntasks No." << i << std::endl;
        std::cout << job.pod_desc.tasks[i].id << std::endl;
        std::cout << job.pod_desc.tasks[i].cpu.milli_core << std::endl;
        std::cout << job.pod_desc.tasks[i].cpu.excess << std::endl;
        std::cout << job.pod_desc.tasks[i].memory.size << std::endl;
        std::cout << job.pod_desc.tasks[i].memory.excess << std::endl;

        for (size_t j = 0; j < job.pod_desc.tasks[i].ports.size(); ++j) {
            std::cout << job.pod_desc.tasks[i].ports[j].port_name << std::endl;
            std::cout << job.pod_desc.tasks[i].ports[j].port << std::endl;
        }

        std::cout << job.pod_desc.tasks[i].exe_package.start_cmd << std::endl;
        std::cout << job.pod_desc.tasks[i].exe_package.stop_cmd << std::endl;
        std::cout << job.pod_desc.tasks[i].exe_package.package.source_path << std::endl;
        std::cout << job.pod_desc.tasks[i].exe_package.package.dest_path << std::endl;
        std::cout << job.pod_desc.tasks[i].exe_package.package.version << std::endl;

        std::cout << job.pod_desc.tasks[i].data_package.reload_cmd << std::endl;
        for (size_t j = 0; j < job.pod_desc.tasks[i].data_package.packages.size(); ++j) {
            std::cout << job.pod_desc.tasks[i].data_package.packages[j].source_path << std::endl;
            std::cout << job.pod_desc.tasks[i].data_package.packages[j].dest_path << std::endl;
            std::cout << job.pod_desc.tasks[i].data_package.packages[j].version << std::endl;
        }
        
        for (size_t j = 0; j < job.pod_desc.tasks[i].services.size(); ++j) {
            std::cout << job.pod_desc.tasks[i].services[j].service_name << std::endl;
            std::cout << job.pod_desc.tasks[i].services[j].port_name  << std::endl;
            std::cout << job.pod_desc.tasks[i].services[j].use_bns << std::endl;
        }

    }

    return ok;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
