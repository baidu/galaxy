// Copyright (c) 2014, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gflags/gflags.h>

// sdk
DEFINE_string(master_host, "localhost", "Master service hostname");

// master
DEFINE_string(master_port, "7828", "Master service listen port");

// scheduler

// agent
DEFINE_int32(agent_timeout, 40000, "Agent timeout");
DEFINE_int32(agent_heartbeat_period, 5000, "Agent heartbeat period");
DEFINE_int32(agent_rpc_timeout, 10000, "Agent RPC timeout");

// gce
