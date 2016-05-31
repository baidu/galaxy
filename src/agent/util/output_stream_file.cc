// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "output_stream_file.h"
#include <errno.h>
namespace baidu {
namespace galaxy {
namespace file {

OutputStreamFile::OutputStreamFile(const std::string& path, const std::string& mode) :
    file_(NULL),
    errno_(0) {
    file_ = fopen(path.c_str(), mode.c_str());
    errno_ = errno;
}
OutputStreamFile::~OutputStreamFile() {
    if (NULL != file_) {
        fclose(file_);
        errno_ = errno;
        file_ = NULL;
    }
}

bool OutputStreamFile::IsOpen() {
    return NULL != file_;
}

void OutputStreamFile::GetLastError(baidu::galaxy::util::ErrorCode& ec) {
    ec = ERRORCODE(errno_, "%s", strerror(errno_));
}

baidu::galaxy::util::ErrorCode OutputStreamFile::Write(const void* data, size_t& len) {
    assert(IsOpen());
    assert(NULL != data);
    assert(len > 0);
    len = fwrite(data, 1, len, file_);
    errno_ = errno;

    if (len == 0) {
        return PERRORCODE(-1, errno_, "write failed");
    }

    return ERRORCODE_OK;
}
}
}
}
