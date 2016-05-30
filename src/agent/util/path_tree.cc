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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "boost/lexical_cast/lexical_cast_old.hpp"
#include "glog/logging.h"


namespace baidu {
namespace galaxy {
namespace path {
static std::string root_path_;

void SetRootPath(const std::string& root_path)
{
    assert(root_path_.empty());
    boost::filesystem::path path(root_path);
    root_path_ = path.string();

    boost::system::error_code ec;
    if (!boost::filesystem::exists(path, ec)
            && !boost::filesystem::create_directories(path, ec)) {
        LOG(FATAL) << "failed in creating root path: " << ec.message();
        exit(1);
    }

    std::string work_dir = WorkDir();
    if (!boost::filesystem::exists(work_dir)
            && !boost::filesystem::create_directories(work_dir, ec)) {
        LOG(FATAL) << "failed in creating workdir: " << ec.message();
        exit(1);
    }

    std::string gc_dir = WorkDir();
    if (!boost::filesystem::exists(gc_dir)
            && !boost::filesystem::create_directories(gc_dir, ec)) {
        LOG(FATAL) << "failed in creating workdir: " << ec.message();
        exit(1);
    }
}

const std::string RootPath()
{
    assert(!root_path_.empty());
    return root_path_;
}

const std::string GcDir()
{
    assert(!root_path_.empty());
    boost::filesystem::path path(root_path_);
    path.append("gc_dir");
    return path.string();
}

const std::string ContainerGcDir(const std::string& container_id, int gc_index)
{
    assert(gc_index >= 0);
    boost::filesystem::path gc(GcDir());
    gc.append(container_id + "." + boost::lexical_cast<std::string>(gc_index));
    return gc.string();
}

const std::string WorkDir()
{
    assert(!root_path_.empty());
    boost::filesystem::path path(root_path_);
    path.append("work_dir");
    return path.string();
}

const std::string ContainerRootPath(const std::string& container_id)
{
    assert(!root_path_.empty());
    boost::filesystem::path path(WorkDir());
    path.append(container_id);
    return path.string();
}

const std::string ContainerGcRootPath(const std::string& container_id, uint32_t index)
{
    assert(!root_path_.empty());
    boost::filesystem::path path(GcDir());
    path.append(container_id + "." + boost::lexical_cast<std::string>(index));
    return path.string();
}

const std::string CgroupRootPath(const std::string& container_id, const std::string& cgroup_id)
{
    assert(!root_path_.empty());
    boost::filesystem::path path(ContainerRootPath(container_id));
    path.append(cgroup_id);
    return path.string();
}
}
}
}
