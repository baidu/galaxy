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
DEFINE_int32(agent_heartbeat_period, 5000, "Agent heartbeat period");

// gce

