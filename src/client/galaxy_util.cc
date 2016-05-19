// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <string>

namespace baidu {
namespace galaxy {
namespace client {

std::string FormatDate(uint64_t datetime) {
    if (datetime < 100) {
        return "-";
    }

    char buf[100];
    time_t time = datetime / 1000000;
    struct tm *tmp;
    tmp = localtime(&time);
    strftime(buf, 100, "%F %X", tmp);
    std::string ret(buf);
    return ret;
}

bool GetHostname(std::string* hostname) {
    char buf[100];
    if (gethostname(buf, 100) != 0) {
        fprintf(stderr, "gethostname failed\n");
        return false;
    }
    *hostname = buf;
    return true;
}

} // end namespace client
} // end namespace galaxy
} // end namespace baidu
/* vim: set ts=4 sw=4 sts=4 tw=100 */
