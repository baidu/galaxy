// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gflags/gflags.h>

// nexus
DEFINE_string(nexus_servers, "127.0.0.1:8868", "server list of nexus, e.g abc.com:1234,def.com:5342");
DEFINE_string(nexus_root_path, "/baidu/galaxy", "root path of galaxy cluster on nexus, e.g /ps/galaxy");

// appmaster
DEFINE_string(appmaster_nexus_path, "/appmaster", "appmaster path on nexus");
DEFINE_string(appmaster_endpoint, "", "appmaster endpoint");

// appworker
DEFINE_int32(appworker_fetch_task_timeout, 10000, "appworker fetch task timeout");
DEFINE_int32(appworker_fetch_task_interval, 1000, "appworker fetch task interval");
DEFINE_int32(appworker_update_appmaster_stub_interval, 10000, "appworker udate appmaster_stub interval, default 1 min");
DEFINE_int32(appworker_background_thread_pool_size, 10, "appworker background trehad pool size");
DEFINE_string(appworker_job_id, "", "appworker job id");
DEFINE_string(appworker_pod_id, "", "appworker pod id");
DEFINE_string(appworker_endpoint, "", "appworker endpoint");
DEFINE_string(appworker_container_id, "", "appworker container id");

// pod_manager
DEFINE_int32(pod_manager_check_pod_interval, 500, "pod manager check pod interval");

// task_manager
DEFINE_int32(task_manager_background_thread_pool_size, 1, "task manager background thread pool size");
DEFINE_int32(task_manager_killer_thread_pool_size, 10, "task manager killer thread pool size");
DEFINE_int32(task_manager_loop_wait_interval, 1000, "task manager loop wait child pid interval");

// task_collector
DEFINE_int32(task_collector_collect_interval, 500, "task collector collect interval");
