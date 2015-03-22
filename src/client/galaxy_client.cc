// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "sdk/galaxy.h"

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "%s master_addr task_raw cmd_line replia_count\n", argv[0]);
        return -1;
    }
    FILE* fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "Open %s for read fail\n", argv[2]);
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
    galaxy->NewTask(argv[2], task_raw, argv[3], atoi(argv[4]));
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
