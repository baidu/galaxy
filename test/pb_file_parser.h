// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com
#ifndef _PB_FILE_PARSER_H
#define _PB_FILE_PARSER_H

#include <string>
#include "google/protobuf/message.h"

namespace galaxy {
namespace test {

bool ParseFromFile(const std::string& file, google::protobuf::Message* pb);

bool DumpToFile(const std::string& file, const google::protobuf::Message& pb);

}   // ending namespace test
}   // ending namespace galaxy

#endif  //_PB_FILE_PARSER_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
