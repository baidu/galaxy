// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "binary_downloader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "common/logging.h"
namespace galaxy {
    
static int OPEN_FLAGS = O_CREAT | O_WRONLY;
static int OPEN_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO;

BinaryDownloader::BinaryDownloader()
    : stoped_(0) {
}

BinaryDownloader::~BinaryDownloader() {}

void BinaryDownloader::Stop() {
    stoped_ = 1;
}

int BinaryDownloader::Fetch(const std::string& uri, 
        const std::string& path) {

    int output_fd_ = open(path.c_str(), OPEN_FLAGS, OPEN_MODE);
    if (output_fd_ == -1) {
        LOG(WARNING, "open file failed %s err[%d: %s]",
                path.c_str(), errno, strerror(errno)); 
        return -1;
    }

    
    int ret = 0;
    LOG(INFO, "start to binary data");
    if (write(output_fd_, uri.c_str(), uri.size()) 
            == -1) {
        LOG(WARNING, "write file failed [%d: %s]", errno, strerror(errno)); 
        ret = -1;
    }

    close(output_fd_);
    return ret;
}

}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
