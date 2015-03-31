// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <string>
std::string FLAGS_master_port = "8101";
std::string FLAGS_agent_port = "8102";
std::string FLAGS_master_addr = "localhost:" + FLAGS_master_port;
int FLAGS_task_retry_times = 3;
int FLAGS_task_deloy_timeout = 60;

std::string FLAGS_agent_work_dir = "/tmp";
std::string FLAGS_container = "cmd";// cmd or cgroup


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
