// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include <stdio.h>
#include <stdlib.h>

#include "agent/curl_downloader.h"

int FLAGS_agent_curl_recv_buffer_size = 1024*20;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "./curl_downloader_test uri path");
        return EXIT_FAILURE;
    }
    //char* uri = argv[1];
    //char* path = argv[2];
    galaxy::CurlDownloader downloader;
    downloader.Fetch(argv[1], argv[2]);
    return EXIT_SUCCESS;
}


/* vim: set ts=4 sw=4 sts=4 tw=100 */
