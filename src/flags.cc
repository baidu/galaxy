// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <string>
std::string FLAGS_master_port = "8101";
std::string FLAGS_agent_port = "8102";
std::string FLAGS_master_addr = "localhost:" + FLAGS_master_port;

#ifdef AGENT_WORK_DIR
const char* work_dir = AGENT_WORK_DIR;
std::string FLAGS_agent_work_dir(work_dir);
#else 
std::string FLAGS_agent_work_dir;
#endif


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
