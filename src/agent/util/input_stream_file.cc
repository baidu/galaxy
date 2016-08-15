// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input_stream_file.h"
#include <string.h>
#include <errno.h>

namespace baidu {
namespace galaxy {
namespace file {
InputStreamFile::InputStreamFile(const std::string& path) :
    path_(path),
    errno_(0),
    file_(NULL) {

    file_ = fopen(path.c_str(), "r");
    if (NULL == file_) {
        errno_ = errno;
    }
}

InputStreamFile::~InputStreamFile() {
    if (NULL != file_) {
        fclose(file_);
    }
}

bool InputStreamFile::IsOpen() {
    return NULL != file_;
}

bool InputStreamFile::Eof() {
    assert(NULL != file_);
    return 0 != feof(file_);
}

baidu::galaxy::util::ErrorCode InputStreamFile::GetLastError() {
    return ERRORCODE(errno_, "%s", strerror(errno_));
}

baidu::galaxy::util::ErrorCode InputStreamFile::ReadLine(std::string& line) {
    assert(IsOpen());
    size_t size;
    char* buf = NULL;
    ssize_t rsize = getline(&buf, &size, file_);

    if (rsize == -1 && !feof(file_)) {
        return PERRORCODE(-1, errno_, "get line failed");
    } else if (feof(file_)) {
        return ERRORCODE_OK;
    }

    line.assign(buf, rsize);
    free(buf);
    buf = NULL;
    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode InputStreamFile::Read(void* buf, size_t& size) {
    assert(IsOpen());
    assert(NULL != buf);
    assert(size > 0);
    size = fread(buf, 1, size, file_);
    errno_ = errno;

    if (size == 0 && ferror(file_)) {
        return PERRORCODE(-1, errno_, "read failed");
    }

    return ERRORCODE_OK;
}
}
}
}
