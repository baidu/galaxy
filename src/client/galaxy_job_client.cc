// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_job_action.h"

DEFINE_string(f, "", "specify config file");
DEFINE_string(i, "", "specify job id");
DEFINE_string(c, "", "specify cmd");

DECLARE_string(flagfile);

const std::string kGalaxyUsage = "galaxy.\n"
                                 "Usage:\n"
                                 "      galaxy submit -f <jobconfig>\n"
                                 "      galaxy update -f <jobconfig> -i id\n"
                                 "      galaxy remove -i id\n"
                                 "      galaxy stop -i id\n"
                                 "      galaxy remove -i id\n"
                                 "      galaxy list\n"
                                 "      galaxy show -i id\n"
                                 "      galaxy exec -i id -c cmd\n"
                                 "Optionss: \n"
                                 "      -f specify config file, job config file or label config file.\n"
                                 "      -c specify cmd.\n"
                                 "      -i specify job id.\n";

int main(int argc, char** argv) {
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
    if (strcmp(argv[1], "create") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        return jobAction->SubmitJob(FLAGS_f);
    } else if (strcmp(argv[1], "update") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        return jobAction->UpdateJob(FLAGS_f, FLAGS_i);
    } else if (strcmp(argv[1], "remove") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        return jobAction->RemoveJob(FLAGS_i);

    } else if (strcmp(argv[1], "list") == 0) {
        return jobAction->ListJobs();
    } else if (strcmp(argv[1], "show") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        return jobAction->ShowJob(FLAGS_i);

    } else if (strcmp(argv[1], "exec") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        if (FLAGS_c.empty()) {
            fprintf(stderr, "-c is needed\n");
            return -1;
        }
        return jobAction->ExecuteCmd(FLAGS_i, FLAGS_c);
    } else {
        fprintf(stderr, "%s", kGalaxyUsage.c_str());
        return -1;
    }
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
