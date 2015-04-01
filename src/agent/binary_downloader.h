// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#ifndef _BINARY_DOWNLOADER_H
#define _BINARY_DOWNLOADER_H

#include "agent/downloader.h"

namespace galaxy {

class BinaryDownloader : public Downloader {
public:
    BinaryDownloader();

    virtual ~BinaryDownloader();

    virtual int Fetch(const std::string& uri,
                      const std::string& path); 

    virtual void Stop();
private:
    int stoped_;
};

}   // ending namespace galaxy

#endif  //_BINARY_DOWNLOADER_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
