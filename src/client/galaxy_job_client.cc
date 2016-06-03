// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>
#include "galaxy_job_action.h"

DEFINE_string(f, "", "specify config file");
DEFINE_string(i, "", "specify job id");
DEFINE_string(c, "", "specify cmd");
DEFINE_int32(t, 1, "specify task num");
DEFINE_int32(d, 1, "specify data_volums num");
DEFINE_int32(p, 1, "specify port num");
DEFINE_int32(a, 1, "specify packages num in data_package");
DEFINE_int32(s, 1, "specify service num");
DEFINE_string(o, "", "operation");

DECLARE_string(flagfile);

const std::string kGalaxyUsage = "galaxy.\n"
                                 "Usage:\n"
                                 "      galaxy submit -f <jobconfig>\n"
                                 "      galaxy update -f <jobconfig> -i id [-o start(need -f -t breakpoint)|continue|rollback|default(need -f)]\n"
                                 "      galaxy stop -i id\n"
                                 "      galaxy remove -i id\n"
                                 "      galaxy list [-o cpu,mem,volums]\n"
                                 "      galaxy show -i id\n"
                                 "      galaxy exec -i id -c cmd\n"
                                 "      galaxy json [-t num_task -d num_data_volums -p num_port -a num_packages in data_package -s num_service]\n"
                                 "Optionss: \n"
                                 "      -f specify config file, job config file or label config file.\n"
                                 "      -c specify cmd.\n"
                                 "      -i specify job id.\n"
                                 "      -t specify specify task num or update breakpoint, default 1.\n"
                                 "      -d spicify data_volums num, default 1\n"
                                 "      -p specify port num, default 1\n"
                                 "      -a specify specify packages num in data_package, default 1\n"
                                 "      -s specify specify service num, default 1\n"
                                 "      -o specify operation.\n";

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
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        ok = jobAction->UpdateJob(FLAGS_f, FLAGS_i, FLAGS_o, FLAGS_t);
    } else if (strcmp(argv[1], "remove") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        ok = jobAction->RemoveJob(FLAGS_i);

    } else if (strcmp(argv[1], "list") == 0) {
        ok = jobAction->ListJobs(FLAGS_o);
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
        ok = ::baidu::galaxy::client::GenerateJson(FLAGS_t, 
                                                   FLAGS_d,
                                                   FLAGS_p, 
                                                   FLAGS_a, 
                                                   FLAGS_s
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
