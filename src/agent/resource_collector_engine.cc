// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/resource_collector_engine.h"

#include <pthread.h>
#include <boost/bind.hpp>
#include "common/logging.h"

extern int FLAGS_resource_collector_engine_threads;

namespace galaxy {

static ResourceCollectorEngine* g_collector_engine = NULL;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static void DestroyResourceCollectorEngine() {
    delete g_collector_engine;
    g_collector_engine = NULL;
    return;
}

static void InitResourceCollectorEngine() {
    g_collector_engine = 
        new ResourceCollectorEngine(
                FLAGS_resource_collector_engine_threads);
    ::atexit(DestroyResourceCollectorEngine);
    return;
}

ResourceCollectorEngine* GetResourceCollectorEngine() {
    pthread_once(&g_once, InitResourceCollectorEngine); 
    return g_collector_engine;
}

long ResourceCollectorEngine::AddCollector(ResourceCollector* collector) {
    if (collector == NULL) {
        return -1; 
    }
    common::MutexLock lock(&collector_set_lock_);
    long collector_id = next_collector_id_ ++;
    collector_set_[collector_id] = collector;
    return collector_id;
}

void ResourceCollectorEngine::DelCollector(long id) {
    if (id > next_collector_id_) {
        return; 
    }

    common::MutexLock lock(&collector_set_lock_);
    std::map<long, ResourceCollector*>::iterator it; 
    it = collector_set_.find(id);
    if (it == collector_set_.end()) {
        return; 
    }
    collector_set_.erase(it);
    return;
}

void ResourceCollectorEngine::Start() {
    collector_thread_->DelayTask(collect_interval_, 
            boost::bind(&ResourceCollectorEngine::OnTimeout, this));
    return;
}

void ResourceCollectorEngine::OnTimeout() {
    // TODO lock scope may be a little bigger
    // But ResourceCollector* may be release by user
    LOG(DEBUG, "start to do collect");
    common::MutexLock lock(&collector_set_lock_);    
    std::map<long, ResourceCollector*>::iterator it;
    for (it = collector_set_.begin(); 
            it != collector_set_.end(); ++it) {
        ResourceCollector* collector = it->second;
        if (!collector->CollectStatistics()) {
            LOG(WARNING, "collector %ld collect failed", it->first); 
        }
    }

    collector_thread_->DelayTask(collect_interval_, 
            boost::bind(&ResourceCollectorEngine::OnTimeout, this));
    return;
}

}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
