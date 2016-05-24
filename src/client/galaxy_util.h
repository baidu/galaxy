// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_CLIENT_UTIL_H
#define BAIDU_GALAXY_CLIENT_UTIL_H

#include <time.h>
#include "sdk/galaxy_sdk.h"
#include "string_util.h"
#include <map>

namespace baidu {
namespace galaxy {
namespace client {

//初始化字符串映射表
std::string StringAuthority(const ::baidu::galaxy::sdk::Authority& authority);
std::string StringAuthorityAction(const ::baidu::galaxy::sdk::AuthorityAction& action);
std::string StringVolumMedium(const ::baidu::galaxy::sdk::VolumMedium& medium);
std::string StringVolumType(const ::baidu::galaxy::sdk::VolumType& type);
std::string StringJobType(const ::baidu::galaxy::sdk::JobType& type);
std::string StringJobStatus(const ::baidu::galaxy::sdk::JobStatus& status);
std::string StringPodStatus(const ::baidu::galaxy::sdk::PodStatus& status);
std::string StringTaskStatus(const ::baidu::galaxy::sdk::TaskStatus& status);
std::string StringContainerStatus(const ::baidu::galaxy::sdk::ContainerStatus& status);
std::string StringContainerGroupStatus(const ::baidu::galaxy::sdk::ContainerGroupStatus& status);
std::string StringStatus(const ::baidu::galaxy::sdk::Status& status);
std::string StringAgentStatus(const ::baidu::galaxy::sdk::AgentStatus& status);
std::string StringResourceError(const ::baidu::galaxy::sdk::ResourceError& error);

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
