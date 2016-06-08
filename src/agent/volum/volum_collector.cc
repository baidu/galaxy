// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "volum_collector.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
namespace baidu {
namespace galaxy {
namespace volum {
VolumCollector::VolumCollector(const std::string& phy_path) :
    enable_(false),
    cycle_(18000),
    name_(phy_path),
    phy_path_(phy_path),
    size_(0) {
}

VolumCollector::~VolumCollector() {

}

baidu::galaxy::util::ErrorCode VolumCollector::Collect() {
    boost::filesystem::path path(phy_path_);
    boost::system::error_code ec;

    if (!boost::filesystem::exists(path, ec)) {
        return ERRORCODE(-1, "%s donot exist: %s",
                phy_path_.c_str(),
                ec.message().c_str());
    }


    int64_t size = 0;
    int64_t count = 0;
    Du(path, size, count);

    boost::mutex::scoped_lock lock(mutex_);
    size_ = size;
    return ERRORCODE_OK;
}

void VolumCollector::Du(const boost::filesystem::path& p, int64_t& size, int64_t& count) {

    boost::system::error_code ec;
    if (boost::filesystem::is_regular(p, ec)) {
        size =  boost::filesystem::file_size(p, ec);
        count = 1;
        return;
    }

    boost::filesystem::directory_iterator end;
    for (boost::filesystem::directory_iterator iter(p); iter != end; iter++) {
        if (boost::filesystem::is_directory(iter->path(), ec)) {
            Du(iter->path(), size, count);
        } else if (boost::filesystem::is_symlink(iter->path(), ec)) {
            //do nothing just ingor
        }
        else if (boost::filesystem::is_regular(iter->path(), ec)) {
            uint64_t s = boost::filesystem::file_size(iter->path(), ec);
            if (static_cast<uintmax_t>(-1) != s) {
                size += s;
                count++;

            }
        }
    }
}


void VolumCollector::Enable(bool enable) {
    enable_ = enable;
}

bool VolumCollector::Enabled() {
    return enable_;
}

bool VolumCollector::Equal(const Collector* c) {
    assert(NULL != c);
    return phy_path_ == c->Name();
}


std::string VolumCollector::Path() const {
    return phy_path_;
}

int VolumCollector::Cycle() {
    return cycle_;
}

std::string VolumCollector::Name() const {
    return name_;
}

void VolumCollector::SetCycle(const int cycle) {
    assert(cycle > 0);
    cycle_ = cycle;
}


int64_t VolumCollector::Size() {
    boost::mutex::scoped_lock lock(mutex_);
    return size_;
}

}
}
}
