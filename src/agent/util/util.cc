/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file util/util.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/07/27 14:04:31
 * @brief 
 *  
 **/

#include "util.h"


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


namespace file {
    bool create_directories(const boost::filesystem::path& path, boost::system::error_code& ec) {
        bool ret = true;
        if (!boost::filesystem::create_directories(path, ec) && ec.value() != 0) {
            ret = false;
        }
        return ret;
    }
}


}
}





















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
