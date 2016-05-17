// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "util/error_code.h"
#include "boost/shared_ptr.hpp"

#include <map>
#include <string>

namespace baidu {
namespace galaxy {
namespace volum {

baidu::galaxy::util::ErrorCode MountProc(const std::string& target);
baidu::galaxy::util::ErrorCode MountDir(const std::string& source, const std::string& target);
baidu::galaxy::util::ErrorCode MountTmpfs(const std::string& target, uint64_t size, bool readonly = false);

struct Mounter {
    std::string source;
    std::string target;
    std::string option;
    std::string filesystem;

    std::string ToString() const {
        std::string ret = source;
        ret += " ";
        ret += target;
        ret += " ";
        ret += filesystem;
        ret += " ";
        ret += option;
        return ret;
    }
};

baidu::galaxy::util::ErrorCode ListMounters(std::map<std::string, boost::shared_ptr<Mounter> >& mounters);

}
}
}
