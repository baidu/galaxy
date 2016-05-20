// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_res_action.h"

DEFINE_string(f, "", "specify config file");
DEFINE_string(i, "", "specify container id");
DEFINE_string(p, "", "specify agent pool");
DEFINE_string(e, "", "specify agent endpoint");
DEFINE_string(t, "", "specify agent tag");

DECLARE_string(flagfile);

const std::string kGalaxyUsage = "galaxy_res_client.\n"
                                 "Usage:\n"
                                 "      galaxy_res_client create_container -f <jobconfig>\n"
                                 "      galaxy_res_client update_container -f <jobconfig> -i id\n"
                                 "      galaxy_res_client remove_container -i id\n"
                                 "      galaxy_res_client list_container\n"
                                 "      galaxy_res_client show_container -i id\n"
                                 "      galaxy_res_client add_agent -p pool -e endpoint\n"
                                 "      galaxy_res_client remove_agent -e endpoint\n"
                                 "      galaxy_res_client list_agents [-p pool -e tag]\n"
                                 "      galaxy_res_client enter_safemode\n"
                                 "      galaxy_res_client leave_safemode\n"
                                 "      galaxy_res_client online_agent -e endpoint\n"
                                 "      galaxy_res_client offline_agent -e endpoint\n"
                                 "      galaxy_res_client status\n"
                                 "Optionss: \n"
                                 "      -f specify config file, job config file or label config file.\n"
                                 "      -i specify container id.\n"
                                 "      -p specify agent pool.\n"
                                 "      -e specify agent tag.\n";

int main(int argc, char** argv) {
    FLAGS_flagfile = "./galaxy.flag";
    ::google::SetUsageMessage(kGalaxyUsage);
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    if (argc < 2) {
        fprintf(stderr, "%s", kGalaxyUsage.c_str());
        fprintf(stderr, "%d", argc);
        return -1;
    }

    ::baidu::galaxy::client::ResAction* resAction = new 
                ::baidu::galaxy::client::ResAction();
    if (strcmp(argv[1], "create_container") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        return resAction->CreateContainerGroup(FLAGS_f);
    } else if (strcmp(argv[1], "update_container") == 0) {
        if (FLAGS_f.empty()) {
            fprintf(stderr, "-f is needed\n");
            return -1;
        }
        
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }

        return resAction->UpdateContainerGroup(FLAGS_f, FLAGS_i);
    } else if (strcmp(argv[1], "remove_container") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        return resAction->RemoveContainerGroup(FLAGS_i);

    } else if (strcmp(argv[1], "list_container") == 0) {
        return resAction->ListContainerGroups();
    } else if (strcmp(argv[1], "show_container") == 0) {
        if (FLAGS_i.empty()) {
            fprintf(stderr, "-i is needed\n");
            return -1;
        }
        return resAction->ShowContainerGroup(FLAGS_i);

    } else if (strcmp(argv[1], "add_agent") == 0) {
        if (FLAGS_p.empty()) {
            fprintf(stderr, "-p is needed\n");
            return -1;
        }
        if (FLAGS_e.empty()) {
            fprintf(stderr, "-e is needed\n");
            return -1;
        }
        return resAction->AddAgent(FLAGS_p, FLAGS_e);
    } else if (strcmp(argv[1], "remove_agent") == 0) {
        if (FLAGS_e.empty()) {
            fprintf(stderr, "-e is needed\n");
            return -1;
        }
        return resAction->RemoveAgent(FLAGS_e);
    } else if (strcmp(argv[1], "list_agents") == 0) {
        if (!FLAGS_p.empty()) {
            return resAction->ListAgentsByPool(FLAGS_p);
        } else if (!FLAGS_t.empty()) {
            return resAction->ListAgentsByTag(FLAGS_t);
        } else {
            return resAction->ListAgents();
        }
    } else if (strcmp(argv[1], "enter_safemode") == 0) { 
        return resAction->EnterSafeMode();
    } else if (strcmp(argv[1], "leave_safemode") == 0) {
        return resAction->LeaveSafeMode();
    } else if (strcmp(argv[1], "online_agent") == 0) {
        if (FLAGS_e.empty()) {
            fprintf(stderr, "-e is needed\n");
            return -1;
        }

        return resAction->OnlineAgent(FLAGS_e);
    } else if (strcmp(argv[1], "offline_agent") == 0) {
        if (FLAGS_e.empty()) {
            fprintf(stderr, "-e is needed\n");
            return -1;
        }
        return resAction->OfflineAgent(FLAGS_e);
    } else if (strcmp(argv[1], "status") == 0) {
        return resAction->Status();
    } else {
        fprintf(stderr, "%s", kGalaxyUsage.c_str());
        return -1;
    }
    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
