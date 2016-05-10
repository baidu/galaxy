// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "error_code.h"

#include <sys/types.h>
#include <unistd.h>

#include <string>

namespace baidu {
    namespace galaxy {
        namespace user {
            baidu::galaxy::util::ErrorCode GetUidAndGid(const std::string& user_name, uid_t* uid, gid_t* gid);
            baidu::galaxy::util::ErrorCode Su(const std::string& user_name);
        }
    }
}
