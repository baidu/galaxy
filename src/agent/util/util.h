/***************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file src/agent/util/util.h
 * @author haolifei(com@baidu.com)
 * @date 2016/05/04 13:44:49
 * @brief
 *
 **/

#pragma once
#include <string.h>

#include <string>

namespace baidu {
namespace galaxy {
namespace util {

const std::string StrError(int errnum) {
    char buf[1024] = {0};

    if (0 == strerror_r(errnum, buf, sizeof buf)) {
        return std::string(buf);
    }

    return "";
}
}
}
}
