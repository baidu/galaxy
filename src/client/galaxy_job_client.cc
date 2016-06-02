// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <string.h>
#include "galaxy_job_action.h"

DEFINE_string(f, "", "specify config file");
DEFINE_string(i, "", "specify job id");
DEFINE_string(c, "", "specify cmd");
DEFINE_string(t, "", "specify task num");
DEFINE_string(d, "", "specify data_volums num");
DEFINE_string(p, "", "specify port num");
DEFINE_string(a, "", "specify packages num in data_package");
DEFINE_string(s, "", "specify service num");

DECLARE_string(flagfile);

const std::string kGalaxyUsage = "galaxy.\n"
                                 "Usage:\n"
                                 "      galaxy submit -f <jobconfig>\n"
                                 "      galaxy update -f <jobconfig> -i id\n"
                                 "      galaxy stop -i id\n"
                                 "      galaxy remove -i id\n"
                                 "      galaxy list\n"
                                 "      galaxy show -i id\n"
                                 "      galaxy exec -i id -c cmd\n"
                                 "      galaxy json [-t num_task -d num_data_volums -p num_port -a num_packages in data_package -s num_service]\n"
                                 "Optionss: \n"
                                 "      -f specify config file, job config file or label config file.\n"
                                 "      -c specify cmd.\n"
                                 "      -i specify job id.\n"
                                 "      -t specify specify task num, default 1.\n"
                                 "      -d spicify data_volums num, default 1\n"
                                 "      -p specify port num, default 1\n"
                                 "      -a specify specify packages num in data_package, default 1\n"
                                 "      -s specify specify service num, default 1\n";

int main(int argc, char** argv) {
    bool ok = true;
    FLAGS_flagfile = "./galaxy.flag";
    ::google::SetUsageMessage(kGalaxyUsage);
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    if (argc < 2) {
        fprintf(stderr, "%s", kGalaxyUsage.c_str());
        fprintf(stderr, "%d", argc);
        return -1;
    }

    ::baidu::galaxy::client::JobAction* jobAction = new 
                ::baidu::galaxy::client::JobAction();
    if (strcmp(argv[1], "submit") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        ok = jobAction->SubmitJob(FLAGS_f);
    } else if (strcmp(argv[1], "update") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        ok = jobAction->UpdateJob(FLAGS_f, FLAGS_i);
    } else if (strcmp(argv[1], "remove") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        ok = jobAction->RemoveJob(FLAGS_i);

    } else if (strcmp(argv[1], "list") == 0) {
        ok = jobAction->ListJobs();
    } else if (strcmp(argv[1], "stop") == 0) { 
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        ok = jobAction->StopJob(FLAGS_i);
    } else if (strcmp(argv[1], "show") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        ok = jobAction->ShowJob(FLAGS_i);

    } else if (strcmp(argv[1], "exec") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        if (FLAGS_c.empty()) {
            fprintf(stderr, "-c is needed\n");
            return -1;
        }
        ok = jobAction->ExecuteCmd(FLAGS_i, FLAGS_c);
    } else if (strcmp(argv[1], "json") == 0) { 
        int num_tasks = atoi(FLAGS_t.c_str());
        if (FLAGS_t.empty()) {
            num_tasks = 1;
        }

        int num_data_volums = atoi(FLAGS_d.c_str());
        if (FLAGS_d.empty()) {
            num_data_volums =1;
        }

        int num_ports = atoi(FLAGS_p.c_str());
        if (FLAGS_p.empty()) {
            num_ports = 1;
        }
        int num_packages = atoi(FLAGS_a.c_str());
        if (FLAGS_a.empty()) {
            num_packages = 1;
        }
        int num_services = atoi(FLAGS_s.c_str());
        if (FLAGS_s.empty()) {
            num_services = 1;
        }
        ok = ::baidu::galaxy::client::GenerateJson(num_tasks, 
                                                   num_data_volums,
                                                   num_ports, 
                                                   num_packages, 
                                                   num_services
                                                  ); 
    }else {
        fprintf(stderr, "%s", kGalaxyUsage.c_str());
        return -1;
    }
    if (ok) {
        return 0;
    } else {
        return -1;
    }
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
