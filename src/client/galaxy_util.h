// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_CLIENT_UTIL_H
#define BAIDU_GALAXY_CLIENT_UTIL_H

#include <time.h>
#include "sdk/galaxy_sdk.h"
#include "string_util.h"

namespace baidu {
namespace galaxy {
namespace client {

//job json解析
int BuildJobFromConfig(const std::string& conf, ::baidu::galaxy::sdk::JobDescription* job);

//时间戳转换
std::string FormatDate(int64_t datetime);

//获取主机名
bool GetHostname(std::string* hostname);

} //end namespace client
} //end namespace galaxy
} //end namespace baidu

#endif  // BAIDU_GALAXY_CLIENT_UTIL_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
