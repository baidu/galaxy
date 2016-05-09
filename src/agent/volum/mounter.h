// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"

namespace baidu {
    namespace galaxy {
        namespace volum {
            
            baidu::galaxy::util::ErrorCode MountProc(const std::string& target);
            baidu::galaxy::util::ErrorCode MountDir(const std::string& source, const std::string& target);
            baidu::galaxy::util::ErrorCode MountTmpfs(const std::string& target, uint64_t size, bool readonly = false);
        }
    }
}
