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
    if (NULL == file_) {
        errno_ = errno;
    }
}
OutputStreamFile::~OutputStreamFile() {
    if (NULL != file_) {
        fclose(file_);
        file_ = NULL;
    }
}

bool OutputStreamFile::IsOpen() {
    return NULL != file_;
}

baidu::galaxy::util::ErrorCode OutputStreamFile::GetLastError() {
    return ERRORCODE(errno_, "%s", strerror(errno_));
}

baidu::galaxy::util::ErrorCode OutputStreamFile::Write(const void* data, size_t& len) {
    assert(IsOpen());
    assert(NULL != data);
    assert(len > 0);
    len = fwrite(data, 1, len, file_);

    if (len == 0) {
        errno_ = errno;
        return PERRORCODE(-1, errno_, "write failed");
    }

    return ERRORCODE_OK;
}

}
}
}
