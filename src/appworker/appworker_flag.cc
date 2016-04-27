// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gflags/gflags.h>

// nexus
DEFINE_string(nexus_servers, "", "server list of nexus, e.g abc.com:1234,def.com:5342");
DEFINE_string(nexus_root_path, "/baidu/galaxy", "root path of galaxy cluster on nexus, e.g /ps/galaxy");

DEFINE_string(appmaster_nexus_path, "/appmaster", "appmaster path on nexus");
DEFINE_string(appmaster_endpoint, "127.0.0.1:8221", "appmaster endpoint");

DEFINE_int32(appworker_fetch_task_rpc_timeout, 10000, "appworker fetch task timeout");
DEFINE_int32(appworker_background_thread_pool_size, 10, "appworker background trehad pool size");
DEFINE_string(appworker_container_id, "0", "appworker container id");

DEFINE_int32(task_manager_background_thread_pool_size, 10, "task manager background thread pool size");
DEFINE_int32(task_manager_killer_thread_pool_size, 10, "task manager killer thread pool size");


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */

