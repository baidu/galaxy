// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "healthy_checker.h"

#include "glog/logging.h"
#include "boost/filesystem/path.hpp"
#include "cgroup/subsystem.h"
#include "boost/bind.hpp"

#include "cgroup/subsystem_factory.h"
#include "resource/resource_manager.h"
#include "volum/mounter.h"

namespace baidu {
namespace galaxy {
namespace health {
typedef enum ERR {
    CHECK_FAILURE = -1,
    CHECK_OK = 0,
    INTERVAL_ERROR = 1
};

HealthChecker::HealthChecker() :
    running_(false),
    healthy_(true) {
}
 
HealthChecker::~HealthChecker() {}

bool HealthChecker::Healthy() {
    return healthy_;
}

void HealthChecker::LoadVolum(const boost::shared_ptr<baidu::galaxy::resource::ResourceManager> res) {
    assert(NULL != res);
    assert(volums_.empty());
    std::vector<boost::shared_ptr<baidu::galaxy::proto::VolumResource> > resource;
    res->GetVolumResource(resource);

    for (size_t i = 0; i < resource.size(); i++) {
        volums_.push_back(resource[i]->device_path());
    }
}

void HealthChecker::LoadCgroup(const boost::shared_ptr<baidu::galaxy::cgroup::SubsystemFactory> fac) {
    assert(NULL != fac.get());
    assert(cgroups_.empty());
    fac->GetSubsystems(cgroups_);
}

// fixme: detach
void HealthChecker::Setup() {
    assert(!volums_.empty());
    assert(!cgroups_.empty());

    running_ = true;
    if (!thread_.Start(boost::bind(&HealthChecker::CheckRoutine, this))) {
        LOG(FATAL) << "setup failed";
        assert(false);
    }
}

void HealthChecker::CheckRoutine() {

    while (running_) {
        MMounter mounters;
        {
            boost::mutex::scoped_lock lock(mutex_);
            healthy_ = true;
            last_error_ = ERRORCODE_OK;
        }

        baidu::galaxy::util::ErrorCode ec = baidu::galaxy::volum::ListMounters(mounters);

        if (ec.Code() != 0) {
            LOG(WARNING) << "list mounter failed: " << ec.Message().c_str();
            ::sleep(10);
            continue;
        }

        ec = CheckVolumMounter(mounters);

        if (ec.Code() == CHECK_FAILURE) {
            boost::mutex::scoped_lock lock(mutex_);
            healthy_ = false;
            LOG(WARNING) << "server is no healthy: " << ec.Message();
            last_error_ = ec;
        }

        ec = CheckCgroupMounter(mounters);

        if (ec.Code() == CHECK_FAILURE) {
            boost::mutex::scoped_lock lock(mutex_);
            healthy_ = false;
            LOG(WARNING) << "server is no healthy: " << ec.Message();
            last_error_ = ec;
        }

        ec = CheckVolumReadable();

        if (ec.Code() == CHECK_FAILURE) {
            boost::mutex::scoped_lock lock(mutex_);
            healthy_ = false;
            LOG(WARNING) << "server is no healthy: " << ec.Message();
            last_error_ = ec;
        }

        ::sleep(10);
    }
}

baidu::galaxy::util::ErrorCode HealthChecker::CheckVolumMounter(const MMounter& mounters) {
    assert(!volums_.empty());

    for (size_t i = 0; i < volums_.size(); i++) {
        MMounter::const_iterator iter = mounters.find(volums_[i]);

        if (iter == mounters.end()) {
            return ERRORCODE(CHECK_FAILURE, "%s donot exist", volums_[i].c_str());
        }
    }

    return ERRORCODE(CHECK_OK, "");
}

baidu::galaxy::util::ErrorCode HealthChecker::CheckCgroupMounter(const MMounter& mounters) {
    assert(!cgroups_.empty());

    for (size_t i = 0; i < cgroups_.size(); i++) {
        std::string path = baidu::galaxy::cgroup::Subsystem::RootPath(cgroups_[i]);
        MMounter::const_iterator iter = mounters.find(path);

        if (iter == mounters.end()) {
            return ERRORCODE(CHECK_FAILURE, "%s donot exist", path.c_str());
        }

        if (iter->second->filesystem != "cgroup") {
            return ERRORCODE(CHECK_FAILURE, "filesystem is %s, cgroup is expected", iter->second->filesystem.c_str());
        }
    }

    return ERRORCODE(CHECK_OK, "");
}

// donot realize
baidu::galaxy::util::ErrorCode HealthChecker::CheckVolumReadable() {
    assert(!volums_.empty());

    for (size_t i = 0; i < volums_.size(); i++) {
    }

    return ERRORCODE(CHECK_OK, "");
}


}
}
}
