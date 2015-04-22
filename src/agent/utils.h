// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#ifndef _AGENT_UTILS_H
#define _AGENT_UTILS_H

#include <string>
#include <vector>

#include "google/protobuf/message.h"

namespace galaxy {

bool GetDirFilesByPrefix(
        const std::string& dir, 
        const std::string& prefix, 
        std::vector<std::string>* files);

// %Y%m%d%H%M%S
void GetStrFTime(std::string* time_str);

}   // ending namespace galaxy

#endif  //_AGENT_UTILS_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
