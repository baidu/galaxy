// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sdk/galaxy.h"

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <gflags/gflags.h>

DECLARE_string(master_host);
DECLARE_string(master_port);
DECLARE_string(flagfile);

const std::string kGalaxyUsage = "\n./galaxy_client submit <job_name> <job_package> <replica> <cpu> <mem> <start_cmd>\n" 
                                 "./galaxy_client list <jobid>\n"
                                 "./galaxy_client kill <jobid>";

int AddJob(int argc, char* argv[]) {
    baidu::galaxy::Galaxy* galaxy = baidu::galaxy::Galaxy::ConnectGalaxy(FLAGS_master_host + ":" + FLAGS_master_port);
    baidu::galaxy::JobDescription job;
    job.job_name = argv[0];
    job.binary = argv[1];
    job.replica = atoi(argv[2]);
    job.cpu_required = atoi(argv[3]);
    job.mem_required = atoi(argv[4]);
    job.deploy_step = 0;
    std::string jobid = galaxy->SubmitJob(job);
    if (!jobid.empty()) {
        fprintf(stderr, "Submit job fail\n");
    }
    printf("Submit job %s\n", jobid.c_str());
    return 0;
}

int main(int argc, char* argv[]) {
    FLAGS_flagfile = "./galaxy.flag";
    ::google::SetUsageMessage(kGalaxyUsage);
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    if(argc < 2){
        fprintf(stderr,"Usage:%s\n", kGalaxyUsage.c_str());
        return -1;
    }
    if (strcmp(argv[1], "submit") == 0) {
        return AddJob(argc -2, argv + 2);
    } else {
        fprintf(stderr,"Usage:%s\n", kGalaxyUsage.c_str());
        return -1;
    }
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
