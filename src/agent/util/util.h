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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <string.h>
#include <string>

namespace baidu {
namespace galaxy {

namespace util {
    const std::string StrError(int errnum);
}

namespace file {
    bool create_directories(const boost::filesystem::path& path, boost::system::error_code& ec);
}


}
}
