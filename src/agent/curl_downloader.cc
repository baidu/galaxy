// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "curl_downloader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/logging.h"
extern "C" {
#include "curl/curl.h"
}

extern int FLAGS_agent_curl_recv_buffer_size;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

// destroy func process level
static void GlobalDestroy() {
    curl_global_cleanup();    
}

// init func process level
static void GlobalInit() {
    curl_global_init(CURL_GLOBAL_ALL);
    ::atexit(GlobalDestroy);
}

namespace galaxy {

static int OPEN_FLAGS = O_CREAT | O_WRONLY;
static int OPEN_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO;

CurlDownloader::CurlDownloader()
    : recv_buffer_size_(0),
      used_length_(0),
      recv_buffer_(NULL),
      output_fd_(-1),
      stoped_(0) { 
    if (FLAGS_agent_curl_recv_buffer_size <= 16 * 1024) {
        recv_buffer_size_ = 16 * 1024; 
    }
    else {
        recv_buffer_size_ = FLAGS_agent_curl_recv_buffer_size;
    }
    recv_buffer_ = new char[recv_buffer_size_];
    pthread_once(&once_control, GlobalInit);
}

CurlDownloader::~CurlDownloader() {
    if (recv_buffer_) {
        delete recv_buffer_; 
        recv_buffer_ = NULL;
    }
}

void CurlDownloader::Stop() {
    stoped_ = 1;
}

size_t CurlDownloader::RecvTrunkData(char* ptr, 
                                     size_t size, 
                                     size_t nmemb, 
                                     void* user_data) {
    CurlDownloader* downloader = 
        static_cast<CurlDownloader*>(user_data);
    assert(downloader);

    if (stoped_ == 1) {
        return 0; 
    }

    if (size * nmemb <= 0) {
        return size * nmemb; 
    }

    if (downloader->used_length_ == downloader->recv_buffer_size_
            || size * nmemb > static_cast<size_t>(downloader->recv_buffer_size_ - downloader->used_length_)) {
        // flush to disk 
        if (write(downloader->output_fd_, 
                    downloader->recv_buffer_, downloader->used_length_) == -1) {
            LOG(WARNING, "write file failed [%d: %s]", errno, strerror(errno)); 
            return 0;
        }
        LOG(INFO, "write file %d", downloader->used_length_);
        downloader->used_length_ = 0;
    } 
    
    memcpy(downloader->recv_buffer_ + downloader->used_length_, ptr, size * nmemb);
    downloader->used_length_ += size * nmemb;
    return size * nmemb;
} 

int CurlDownloader::Fetch(const std::string& uri, const std::string& path) {
    output_fd_ = open(path.c_str(), OPEN_FLAGS, OPEN_MODE);
    if (output_fd_ == -1) {
        LOG(WARNING, "open file failed %s err[%d: %s]", 
                path.c_str(), errno, strerror(errno));    
        return -1;
    }

    LOG(INFO, "start to curl data %s", uri.c_str()); 
    CURL* curl = curl_easy_init();
    int ret;
    do {
        ret = curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        if (ret != CURLE_OK) {
            LOG(WARNING, "libcurl setopt %s failed [%d: %s]", 
                    "CURLOPT_URL", ret, curl_easy_strerror((CURLcode)ret)); 
            break;
        }

        ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        if (ret != CURLE_OK) {
            LOG(WARNING, "libcurl setopt %s failed [%d: %s]",
                    "CURLOPT_WRITEDATA", ret, curl_easy_strerror((CURLcode)ret)); 
            break;
        }

        ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlDownloader::RecvTrunkData);
        if (ret != CURLE_OK) {
            LOG(WARNING, "libcurl setopt %s failed [%d: %s]",
                    "CURLOPT_WRITEFUNCTION", ret, curl_easy_strerror((CURLcode)ret)); 
            break;
        }

        ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        if (ret != CURLE_OK) {
            LOG(WARNING, "libcurl setopt %s failed [%d: %s]",
                    "CURLOPT_VERBOSE", ret, curl_easy_strerror((CURLcode)ret)); 
            break;
        }

        ret = curl_easy_perform(curl);
        if (ret != CURLE_OK) {
            LOG(WARNING, "libcurl perform failed [%d: %s]",    
                    ret, curl_easy_strerror((CURLcode)ret));
            break;
        }

        if (used_length_ != 0) {
            if (write(output_fd_, recv_buffer_, used_length_) 
                    == -1) {
                LOG(WARNING, "write file failed [%d: %s]", errno, strerror(errno)); 
                ret = -1;
                break;
            } 
            LOG(INFO, "write file %d", used_length_);
            used_length_ = 0;
        }
    } while(0);

    if (curl != NULL) {
        curl_easy_cleanup(curl); 
    }

    if (ret != CURLE_OK) {
        return -1; 
    }
    return 0;
}

}   // ending namespace galaxy



/* vim: set ts=4 sw=4 sts=4 tw=100 */
