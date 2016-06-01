// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "user.h"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>

namespace baidu {
namespace galaxy {
namespace user {

baidu::galaxy::util::ErrorCode Su(const std::string& user_name)
{
    uid_t uid;
    gid_t gid;
    baidu::galaxy::util::ErrorCode ec = GetUidAndGid(user_name, &uid, &gid);

    if (0 != ec.Code()) {
        return ec;
    }

    if (::setgid(gid) != 0) {
        return PERRORCODE(-1, errno, "setgid %d failed", (int)gid);
    }

    if (::setuid(uid) != 0) {
        return PERRORCODE(-1, errno, "setuid %d failed", (int)uid);
    }



    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode GetUidAndGid(const std::string& user_name, uid_t* uid, gid_t* gid)
{
    // i can not get uid & gid by getpwnam_r after calling chroot, i donot know why
    std::ifstream in("/etc/passwd", std::ios::in);

    if (!in.is_open()) {
        return PERRORCODE(-1, errno, "open /etc/passwd failed");
    }

    std::string line;

    while (std::getline(in, line)) {
        if (boost::starts_with(line, user_name + ":")) {
            std::vector<std::string> v;
            boost::split(v, line, boost::is_any_of(":"));

            if (v.size() == 7) {
                *uid = atoi(v[2].c_str());
                *gid = atoi(v[3].c_str());
                return ERRORCODE_OK;
            }
        }
    }

    return ERRORCODE(-1, "user %s donot exist", user_name.c_str());
}


baidu::galaxy::util::ErrorCode Chown(const std::string& path, const std::string& user) {
    uid_t uid;
    gid_t gid;
    baidu::galaxy::util::ErrorCode ec = GetUidAndGid(user, &uid, &gid);

    if (0 != ec.Code()) {
        return ec;
    }
    if (0 != ::chown(path.c_str(), uid, gid)) {
        return PERRORCODE(-1, errno, "chown failed");
    }
    return ERRORCODE_OK;
}

}
}
}
