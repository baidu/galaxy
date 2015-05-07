// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include <string>
#include <stdint.h>

std::string FLAGS_master_port = "8101";
std::string FLAGS_agent_port = "8102";
std::string FLAGS_agent_http_port = "8103";
int FLAGS_agent_http_server_threads = 10;
std::string FLAGS_master_addr = "localhost:" + FLAGS_master_port;
int FLAGS_task_retry_times = 3;
int FLAGS_task_deploy_timeout = 20;
int FLAGS_agent_keepalive_timeout = 20;
int FLAGS_master_max_len_sched_task_list = 30;

std::string FLAGS_agent_work_dir = "/tmp";
std::string FLAGS_container = "cmd";// cmd or cgroup
std::string FLAGS_cgroup_root = "/cgroups";
int FLAGS_agent_curl_recv_buffer_size = 1024 * 10;

double FLAGS_cpu_num = 4;

int FLAGS_resource_collector_engine_interval = 1000;

int64_t FLAGS_mem_gbytes = 32;
int64_t FLAGS_mem_bytes = FLAGS_mem_gbytes * 1024 * 1024 * 1024;
// 5 hour
int FLAGS_agent_gc_timeout = 1000 * 60 * 60 * 5;

std::string FLAGS_task_acct = "galaxy";
std::string FLAGS_master_checkpoint_path = "./data/";
int64_t FLAGS_master_safe_mode_last = 30; // unit second
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
