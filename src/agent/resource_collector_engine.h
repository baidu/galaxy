// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#ifndef _RESOURCE_COLLECTOR_ENGINE_H
#define _RESOURCE_COLLECTOR_ENGINE_H

#include <map>
#include "common/thread_pool.h"
#include "common/mutex.h"
#include "agent/resource_collector.h"

namespace galaxy {

ResourceCollectorEngine* GetResourceCollectorEngine();

class ResourceCollectorEngine {
public:
    explicit ResourceCollectorEngine(int interval) 
        : collector_thread_(NULL),
          collect_interval_(interval),
          collector_set_(),
          next_collector_id_(0L),
          collector_set_lock_() {
        collector_thread_ = new ThreadPool(1); 
    }   

    virtual ~ResourceCollectorEngine() {
        delete collector_thread_; 
    }

    long AddCollector(ResourceCollector* collector);
    void DelCollector(long collector_id);

    void Start();
protected:
    void OnTimeout();

    common::ThreadPool* collector_thread_;
    int collect_interval_;
    std::map<long, ResourceCollector*> collector_set_;
    // TODO change id generator
    long next_collector_id_;
    common::Mutex collector_set_lock_;
};

}   // ending namespace galaxy

#endif  //_RESOURCE_COLLECTOR_ENGINE_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
