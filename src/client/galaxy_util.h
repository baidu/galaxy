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

//单位转换
int UnitStringToByte(const std::string& input, int64_t* output);

//job json解析
int BuildJobFromConfig(const std::string& conf, ::baidu::galaxy::sdk::JobDescription* job);

//时间戳转换
std::string FormatDate(uint64_t datetime);

//获取主机名
bool GetHostname(std::string* hostname);

//读取endpoint
bool LoadAgentEndpointsFromFile(const std::string& file_name, std::vector<std::string>* agents);

} //end namespace client
} //end namespace galaxy
} //end namespace baidu

#endif  // BAIDU_GALAXY_CLIENT_UTIL_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
