// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com, sunjunyi01@baidu.com

#include <string>
#include <stdint.h>
#include <gflags/gflags.h>

DEFINE_string(master_port, "8101", "master rpc-server listen on this port");
DEFINE_string(agent_port, "8102", "agent rpc-server listen on this port");
DEFINE_int32(agent_http_port, 8089, "agent http-server listen on this port");
DEFINE_int32(agent_http_server_threads, 10, "agent http-server thread pool size");

DEFINE_string(master_addr, "localhost:" + FLAGS_master_port, "master rpc-server endpoint");
DEFINE_int32(task_retry_times, 3, "retry times of task ");
DEFINE_int32(task_deploy_timeout, 20, "task package deploy timeout");
DEFINE_int32(agent_keepalive_timeout, 20, "keepalive timeout of agent");

DEFINE_string(agent_work_dir, "/tmp", "agent work directory");
DEFINE_string(container, "cmd", "container type : cmd or cgroup");

DEFINE_string(cgroup_root, "/cgroups", "cgroup mount point");
DEFINE_int32(agent_curl_recv_buffer_size, 1024 * 10, "agent downloader recv buffer size");

DEFINE_double(cpu_num, 4, "cpu number");

DEFINE_int32(resource_collector_engine_interval, 1000, "rc collect engine interval");

DEFINE_int64(mem_gbytes, 32, "mem in giga bytes");
DEFINE_int64(mem_bytes, FLAGS_mem_gbytes * 1024 * 1024 * 1024, "mem in bytes");
// 5 hour
DEFINE_int32(agent_gc_timeout, 1000 * 60 * 60 * 5, "garbage collection timeout");

DEFINE_string(task_acct, "galaxy", "task user/role of system");

DEFINE_string(master_checkpoint_path, "./data/", "directory of master checkpoint data");
DEFINE_int32(master_max_len_sched_task_list, 30, "max length of scheduled tasks of a job");
DEFINE_int32(master_safe_mode_last, 30, "how many seconds the safe-mode goes on");


DEFINE_string(job_name,"","job name shown on galaxy");
DEFINE_string(task_raw,"","job package which could be a ftp address or a local file ");
DEFINE_string(cmd_line,"","the boot command of job");
DEFINE_int32(replicate_num,0,"the replication number of job");
DEFINE_double(cpu_soft_limit,0.0,"cpu soft limit which is lower than cpu limit");
DEFINE_double(cpu_limit,0.0,"cpu limit which a task can reach but not overtop");
DEFINE_int32(deploy_step_size,0,"how many tasks can be deployed in concurrent");
DEFINE_bool(one_task_per_host,false,"every node just run one task of job");
DEFINE_int64(task_id,-1,"the identify of task");
DEFINE_int64(job_id,-1,"the identify of job");
DEFINE_string(agent_addr,"","the address of a agent shown by listnode");


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
