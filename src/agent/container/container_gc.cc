// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "container_gc.h"
#include "util/input_stream_file.h"
#include "util/path_tree.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "timer.h"
#include "boost/bind.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

DECLARE_int64(gc_delay_time);
namespace baidu {
namespace galaxy {
namespace container {
ContainerGc::ContainerGc() :
    running_(true) {
}


ContainerGc::~ContainerGc() {
    running_ = false;
}

baidu::galaxy::util::ErrorCode ContainerGc::Reload() {
    boost::filesystem::path gc_path(baidu::galaxy::path::GcDir());
    boost::system::error_code ec;

    if (!boost::filesystem::exists(gc_path, ec)) {
        return ERRORCODE_OK;
    }

    boost::filesystem::directory_iterator end;
    for (boost::filesystem::directory_iterator iter(gc_path); iter != end; iter++) {
        if (!boost::filesystem::is_directory(iter->path(), ec)) {
            continue;
        } 
        struct stat s;
        ::stat(iter->path().string().c_str(), &s);
        gc_index_[iter->path().string()] = s.st_ctime + FLAGS_gc_delay_time;
        LOG(INFO) << "reload gc: " << iter->path().string() << ":" << s.st_ctime + FLAGS_gc_delay_time;
    }

    return ERRORCODE_OK;
}

baidu::galaxy::util::ErrorCode ContainerGc::Gc(const std::string& path) {
    int64_t destroy_time = baidu::common::timer::get_micros() / 1000000L;
    boost::mutex::scoped_lock lock(mutex_);
    gc_swap_index_[path] = destroy_time + FLAGS_gc_delay_time;
    LOG(INFO) << path << " will be gc in " << destroy_time + FLAGS_gc_delay_time;
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode ContainerGc::Setup() {
    running_ = true;

    if (!gc_thread_.Start(boost::bind(&ContainerGc::GcRoutine, this))) {
        return ERRORCODE(-1, "start gc thread failed");
    }

    return ERRORCODE_OK;
}

void ContainerGc::GcRoutine() {
    while (running_) {
        {
            boost::mutex::scoped_lock lock(mutex_);
            if (!gc_swap_index_.empty()) {
                std::map<std::string, int64_t>::iterator iter = gc_swap_index_.begin();

                while (iter != gc_swap_index_.end()) {
                    gc_index_[iter->first] = iter->second;
                    iter++;
                }
                gc_swap_index_.clear();
            }
        }

        std::map<std::string, int64_t>::iterator iter = gc_index_.begin();
        while (iter != gc_index_.end()) {
            int64_t now = baidu::common::timer::get_micros() / 1000000L;
            if (iter->second <= now) {
                baidu::galaxy::util::ErrorCode ec = DoGc(iter->first);

                if (ec.Code() != 0) {
                    LOG(WARNING) << iter->first << " gc failed: " << ec.Message();
                    iter++;
                    continue;
                }

                if (ec.Code() != 0) {
                    LOG(WARNING) << iter->first << " rm gc recode failed";
                }

                gc_index_.erase(iter++);
                continue;
            }

            iter++;
        }

        sleep(1);
    }
}

baidu::galaxy::util::ErrorCode ContainerGc::DoGc(const std::string& path) {

    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec)) {
        return ERRORCODE(0, "%s donot exist", path.c_str());
    }

    boost::filesystem::path p(path);
    p.append("container.property");
    
    if (!boost::filesystem::exists(p, ec)) {
        boost::filesystem::remove_all(path, ec);
        if (ec.value() != 0) {
            return ERRORCODE(-1, "remove %s failed: %s",
                        path.c_str(),
                        ec.message().c_str());
        } 
        return ERRORCODE_OK;
    }

    std::vector<std::string> gc_paths;
    baidu::galaxy::util::ErrorCode err = ListGcPath(p.string(), gc_paths);
    if (err.Code() != 0) {
        return ERRORCODE(-1, err.Message().c_str());
    }

    for (size_t i = 0; i < gc_paths.size(); i++) {
        err = Remove(gc_paths[i]);
        if (err.Code() != 0) {
            return ERRORCODE(-1, "remove %s failed: %s", gc_paths[i].c_str(), err.Message().c_str());
        }
        //LOG(INFO) << "succees in removing " << gc_paths[i] << std::endl;
    }

    boost::filesystem::remove_all(path, ec);
    if (ec.value() != 0) {
        return ERRORCODE(-1, "remove %s failed: ",
                    path.c_str(),
                    ec.message().c_str());
    }
    return ERRORCODE_OK;
}


baidu::galaxy::util::ErrorCode ContainerGc::ListGcPath(const std::string& property, 
            std::vector<std::string>& paths) {

    baidu::galaxy::file::InputStreamFile f(property);
    if (!f.IsOpen()) {
        baidu::galaxy::util::ErrorCode err = f.GetLastError();
        return ERRORCODE(-1, "open property failed: %s", err.Message().c_str());
    }

    while (!f.Eof()) {
        std::string line;
        f.ReadLine(line);
        if (boost::starts_with(line, "phy_gc_root_path")) {
            
            if (line[line.size()-1] == '\n') {
                line.erase(line.size()-1);
            }

            std::vector<std::string> v;
            boost::split(v, line, boost::is_any_of(":"));
            
            if (v.size() == 2) {
                boost::trim(v[1]);
                paths.push_back(v[1]);
            } else {
                LOG(WARNING) << "bad format: " << line << " " << v.size();
            }
        }
    }
    return ERRORCODE_OK;


}

baidu::galaxy::util::ErrorCode ContainerGc::Remove(const std::string& path) {
    boost::system::error_code ec;
    if (boost::filesystem::exists(path, ec)) {
        boost::filesystem::remove_all(path, ec);
        if (0 != ec.value()) {
            return ERRORCODE(-1, "remove %s failed: %s", 
                        path.c_str(),
                        ec.message().c_str());
        } 
    } 
    return ERRORCODE_OK;

}

}
}
}

