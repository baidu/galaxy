/***************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file src/agent/util/path_tree.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/04/20 19:17:53
 * @brief
 *
 **/

#include "path_tree.h"
#include "boost/filesystem/path.hpp"
#include "boost/lexical_cast/lexical_cast_old.hpp"


namespace baidu {
namespace galaxy {
namespace path {
static std::string root_path_;

void SetRootPath(const std::string& root_path) {
    assert(root_path_.empty());
    boost::filesystem::path path(root_path);
    root_path_ = path.string();
}

const std::string RootPath() {
    assert(!root_path_.empty());
    return root_path_;
}

const std::string GcDir() {
    assert(!root_path_.empty());
    boost::filesystem::path path(root_path_);
    path.append("gc_dir");
    return path.string();
}

const std::string ContainerGcDir(const std::string& container_id, int gc_index) {
    assert(gc_index >= 0);
    boost::filesystem::path gc(GcDir());
    gc.append(container_id + "." + boost::lexical_cast<std::string>(gc_index));
    return gc.string();
}

const std::string WorkDir() {
    assert(!root_path_.empty());
    boost::filesystem::path path(root_path_);
    path.append("work_dir");
    return path.string();
}

const std::string ContainerRootPath(const std::string& contaier_id) {
    assert(!root_path_.empty());
    boost::filesystem::path path(WorkDir());
    path.append(contaier_id);
    return path.string();
}

const std::string CgroupRootPath(const std::string& container_id, const std::string& cgroup_id) {
    assert(!root_path_.empty());
    boost::filesystem::path path(ContainerRootPath(container_id));
    path.append(cgroup_id);
    return path.string();
}
}
}
}
