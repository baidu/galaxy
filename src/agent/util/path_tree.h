// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * rootpath/
 * |-- gc_dir
 * `-- work_dir
 *    |-- container1   // bind dir, ContainerRootPath
 *    |   |-- bin
 *    |   |-- cgroup1  // cgroup default path
 *    |   |-- cgroup2
 *    |   |-- home
 *    |   |-- proc   // root fs
 *    |   `-- sbin
 *    `-- container2
 */

#pragma once
#include <string>

namespace baidu {
namespace galaxy {
namespace path {
void SetRootPath(const std::string& root_path);
const std::string RootPath();
const std::string GcDir();
const std::string WorkDir();

const std::string ContainerRootPath(const std::string& container_id);
const std::string CgroupRootPath(const std::string& container_id, const std::string& cgroup_id);
const std::string ContainerGcDir(const std::string& container_id, int gc_index);
} //namespace util
} //namespace galaxy
} //namespace path
