// Copyright (c) 2014, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gflags/gflags.h>

// nexus
DEFINE_string(nexus_servers, "", "server list of nexus, e.g abc.com:1234,def.com:5342");
DEFINE_string(nexus_root_path, "/baidu/galaxy", "root path of galaxy cluster on nexus, e.g /ps/galaxy");

// sdk
DEFINE_string(master_host, "localhost", "Master service hostname");

// master
DEFINE_string(master_port, "7828", "Master service listen port");
DEFINE_int32(master_agent_timeout, 40000, "Agent timeout");
DEFINE_int32(master_agent_rpc_timeout, 10000, "Agent RPC timeout");
DEFINE_int32(master_query_period, 30000, "Query period");
DEFINE_string(master_lock_path, "/master_lock", "master lock name on nexus");
DEFINE_string(master_path, "/master", "master path on nexus");
DEFINE_string(jobs_store_path, "/jobs", "");

// scheduler

// agent
DEFINE_string(agent_port, "8080", "agent listen port");
DEFINE_string(master_endpoint, "127.0.0.1:9876", "master endpoint");
DEFINE_string(gce_endpoint, "127.0.0.1:9864", "gced endpoint");
DEFINE_int32(agent_background_threads_num, 2, "agent background threads");
DEFINE_int32(agent_heartbeat_interval, 1000, "agent haertbeat interval ms");
DEFINE_string(agent_ip, "127.0.0.1", "agent host ip");


// gce
DEFINE_string(gce_cgroup_root, "/cgroups/", "Cgroup root mount path");
DEFINE_string(gce_support_subsystems, "cpu,memory,cpuacct,frozen", "Cgroup default support subsystems");
DEFINE_int64(gce_initd_zombie_check_interval, 10000, "Initd Zombie Check Interval");
DEFINE_string(gce_initd_dump_file, "initd_checkpoint_file", "Initd Checkpoint File Name");
DEFINE_string(gce_initd_port, "8765", "gce initd listen port");
DEFINE_string(gce_gced_port, "8766", "gce initd listen port");
