// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * 
 * root/
 * |-- gc_dir
 * `-- work_dir
 *  |-- container1
 *  |   |-- app
 *  |   |-- bin
 *  |   |-- etc
 *  |   `-- home
 *  `-- container2
 */

#pragma once
#include <string>

namespace baidu {
namespace galaxy {
namespace util {

class PathTree {
public:
    PathTree(const std::string& root_path);
    ~PathTree();
    
    const std::string RootPath();
    const std::string GcDir();
    const std::string WorkDir();
    const std::string ContainerDir(const std::string& container);
    const std::string AppRootDir();
private:
};

} //namespace util
} //namespace galaxy
} //namespace baidu
