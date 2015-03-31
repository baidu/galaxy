// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#ifndef _CURL_DOWNLOADER_H
#define _CURL_DOWNLOADER_H

#include "downloader.h"

namespace galaxy {

class CurlDownloader : public Downloader {
public:
    CurlDownloader();

    virtual ~CurlDownloader();

    int Fetch(const std::string& uri, 
            const std::string& path);

    void Stop();

private:
    static size_t RecvTrunkData(char* ptr, size_t size, size_t nmemb, void* user_data);

    int recv_buffer_size_;
    int used_length_;
    char* recv_buffer_;
    int output_fd_;
    volatile int stoped_;
};

}   // ending namespace galaxy

#endif  //_CURL_DOWNLOADER_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
