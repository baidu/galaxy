// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "sdk/galaxy.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
void help() {
    fprintf(stderr, "./galaxy_client master_addr command(list/add/kill) args\n");
    fprintf(stderr, "./galaxy_client master_addr add task_raw cmd_line replicate_count\n");
    fprintf(stderr, "./galaxy_client master_addr list task_id\n");
    fprintf(stderr, "./galaxy_client master_addr kill task_id\n");
    return;
}

enum Command {
    LIST = 0,
    LISTJOB,
    ADD,
    KILL
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        help();
        return -1;
    }
    int COMMAND = 0;
    if (strcmp(argv[2], "add") == 0) {
        COMMAND = ADD;
        if (argc < 6) {
            help();
            return -1;
        }
    } else if (strcmp(argv[2], "list") == 0) {
        COMMAND = LIST;
    } else if (strcmp(argv[2], "listjob") == 0) {
        COMMAND = LISTJOB;
    } else if (strcmp(argv[2], "kill") == 0) {
        COMMAND = KILL;
        if (argc < 4) {
            help();
            return -1;
        }
    } else {
        help();
        return -1;
    }

    if (COMMAND == ADD) {
        FILE* fp = fopen(argv[3], "r");
        if (fp == NULL) {
            fprintf(stderr, "Open %s for read fail\n", argv[3]);
            return -2;
        }
        std::string task_raw;
        char buf[1024];
        int len = 0;
        while ((len = fread(buf, 1, 1024, fp)) > 0) {
            task_raw.append(buf, len);
        }
        fclose(fp);
        printf("Task binary len %lu\n", task_raw.size());
        galaxy::Galaxy* galaxy = galaxy::Galaxy::ConnectGalaxy(argv[1]);
        galaxy->NewTask(argv[3], task_raw, argv[4], atoi(argv[5]));
        return 0;
    }
    else if (COMMAND == LIST) {
        int64_t task_id = -1;
        if (argc == 4) {
            task_id = atoi(argv[3]);
        }
        galaxy::Galaxy* galaxy = galaxy::Galaxy::ConnectGalaxy(argv[1]);
        galaxy->ListTask(task_id);
        return 0;
    }
    else if (COMMAND == LISTJOB) {
        galaxy::Galaxy* galaxy = galaxy::Galaxy::ConnectGalaxy(argv[1]);
        galaxy->ListJob();
        return 0;
    }
    else if (COMMAND == KILL) {
        int64_t task_id = atoi(argv[3]);
        galaxy::Galaxy* galaxy = galaxy::Galaxy::ConnectGalaxy(argv[1]);
        galaxy->TerminateTask(task_id);
        return 0;
    }
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
