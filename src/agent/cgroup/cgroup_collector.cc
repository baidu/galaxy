// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup_collector.h"
#include "boost/thread/mutex.hpp"
#include <assert.h>

namespace baidu {
    namespace galaxy {
        namespace cgroup {

            CgroupCollector::CgroupCollector(boost::shared_ptr<Cgroup> cgroup) :
                enabled_(false),
                cycle_(-1),
                cgroup_(cgroup) {
                assert(NULL != cgroup);
            }

            CgroupCollector::~CgroupCollector() {
                enabled_ = false;
            }

            baidu::galaxy::util::ErrorCode CgroupCollector::Collect() {
                return ERRORCODE_OK;
            }

            void CgroupCollector::Enable(bool enabled) {
                boost::mutex::scoped_lock lock(mutex_);
                enabled_ = enabled;
            }

            bool CgroupCollector::Enabled() {
                boost::mutex::scoped_lock lock(mutex_);
                return enabled_;
            }

            bool CgroupCollector::Equal(const Collector& c) {
                int64_t addr1 = (int64_t)this;
                int64_t addr2 = (int64_t)&c;
                return addr1 == addr2;
            }

            void CgroupCollector::SetCycle(int cycle) {
                boost::mutex::scoped_lock lock(mutex_);
                cycle_ = cycle;
            }
            
            int CgroupCollector::Cycle() {
                boost::mutex::scoped_lock lock(mutex_);
                return cycle_;
            }

            std::string CgroupCollector::Name() const {
                //boost::mutex::scoped_lock lock(mutex_);
                return name_;
            }

            void CgroupCollector::SetName(const std::string& name) {
                boost::mutex::scoped_lock lock(mutex_);
                name_ = name;

            }
        }
    }
}
