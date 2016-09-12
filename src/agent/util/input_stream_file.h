// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "error_code.h"
#include "boost/noncopyable.hpp"
#include <stdio.h>

namespace baidu {
namespace galaxy {
namespace file {

class InputStreamFile : public boost::noncopyable {
public:
    explicit InputStreamFile(const std::string& path);
    ~InputStreamFile();

    bool IsOpen();
    bool Eof();
    baidu::galaxy::util::ErrorCode GetLastError();
    baidu::galaxy::util::ErrorCode ReadLine(std::string& line);
    baidu::galaxy::util::ErrorCode Read(void* buf, size_t& size);

private:
    std::string path_;
    int errno_; // last_erro
    FILE* file_;
};
}
}
}
