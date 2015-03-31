// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "downloader_manager.h"
#include <boost/bind.hpp>

using namespace galaxy;

int FLAGS_agent_curl_recv_buffer_size = 1024 * 20;

void Callback(int ret) {
    fprintf(stdout, "downloader ret %d", ret);
    return ;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "./downloader_manager_test uri path"); 
        return EXIT_FAILURE;
    }    
    char* uri = argv[1];
    char* path = argv[2];
    DownloaderManager* manager = DownloaderManager::GetInstance();

    manager->DownloadInThread(uri, path, boost::bind(Callback, _1));
    while (1) {}
    return EXIT_SUCCESS;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
