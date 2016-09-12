
#include "error_code.h"

// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "boost/noncopyable.hpp"
#include <stdio.h>
#include <string>
namespace baidu {
namespace galaxy {
namespace file {
class OutputStreamFile : public boost::noncopyable {
public:
    OutputStreamFile(const std::string& path, const std::string& mode);
    ~OutputStreamFile();
    bool IsOpen();
    baidu::galaxy::util::ErrorCode GetLastError();
    baidu::galaxy::util::ErrorCode Write(const void* data, size_t& len);

private:
    FILE* file_;
    int errno_;
};
}
}
}
