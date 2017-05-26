// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>

namespace baidu {
namespace galaxy {

bool ExecuteShellCmd(const std::string cmd, std::string* ret_str) {
    char output_buffer[80];
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) {
        LOG(ERROR) << "fail to execute cmd: " << cmd;
        return false;
    }
    fgets(output_buffer, sizeof(output_buffer), fp);
    pclose(fp);
    if (ret_str) {
        *ret_str = std::string(output_buffer);
    }
    return true;
}

std::string GetLocalHostAddr() {
    std::string cmd =
        "/sbin/ifconfig | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}'";
    std::string addr;
    if (!ExecuteShellCmd(cmd, &addr)) {
        LOG(ERROR) << "fail to fetch local host addr";
    } else if (addr.length() > 1) {
        addr.erase(addr.length() - 1, 1);
    }
    return addr;
}

} //namespace galaxy
} //namespace baidu
