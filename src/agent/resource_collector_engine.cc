// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yuanyi03@baidu.com

#include "agent/resource_collector_engine.h"

#include <pthread.h>
#include <boost/bind.hpp>
#include "common/logging.h"
#include <gflags/gflags.h>

DECLARE_int32(resource_collector_engine_interval);

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
                FLAGS_resource_collector_engine_interval);
    g_collector_engine->Start();
    ::atexit(DestroyResourceCollectorEngine);
    return;
}

ResourceCollectorEngine* GetResourceCollectorEngine() {
    pthread_once(&g_once, InitResourceCollectorEngine); 
    return g_collector_engine;
}

long ResourceCollectorEngine::AddCollector(ResourceCollector* collector, int collect_interval) {
    int32_t now_time = common::timer::get_micros() / 1000; 
    if (collector == NULL 
            || collect_interval < 0) {
        return -1; 
    }
    common::MutexLock lock(&collector_set_lock_);
    CollectCell cell;
    cell.collector_id = next_collector_id_ ++;
    if (collect_interval == 0) {
        collect_interval = collect_interval_;
    }
    cell.collect_interval = collect_interval;
    cell.next_collect_time = now_time + collect_interval;
    cell.collector_handler = collector;
    collector_set_[cell.collector_id] = cell;
    LOG(DEBUG, "add collector %ld next collect time[%d] interval[%d] now_time[%d]",
            cell.collector_id, cell.next_collect_time, cell.collect_interval, now_time);
    return cell.collector_id;
}

void ResourceCollectorEngine::DelCollector(long id) {
    if (id > next_collector_id_) {
        return; 
    }

    common::MutexLock lock(&collector_set_lock_);
    std::map<long, CollectCell>::iterator it; 
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
    int32_t now_time = common::timer::get_micros() / 1000;
    common::MutexLock lock(&collector_set_lock_);    
    std::map<long, CollectCell>::iterator it;
    for (it = collector_set_.begin(); 
            it != collector_set_.end(); ++it) {
        if (it->second.next_collect_time > now_time) {
            LOG(DEBUG, "collector %ld not meet next collect time. collect_time[%ld] now[%ld]",
                    it->first, it->second.next_collect_time, now_time);
            continue; 
        }
        LOG(DEBUG, "collector %ld meet next collect time. collect_time[%ld] now[%ld]",
                it->first, it->second.next_collect_time, now_time);
        it->second.next_collect_time += 
            it->second.collect_interval;
        ResourceCollector* collector = 
            it->second.collector_handler;
        if (!collector->CollectStatistics()) {
            LOG(WARNING, "collector %ld collect failed", 
                    it->first); 
        }
    }

    collector_thread_->DelayTask(collect_interval_, 
            boost::bind(&ResourceCollectorEngine::OnTimeout, 
                this));
    return;
}

}   // ending namespace galaxy

/* vim: set ts=4 sw=4 sts=4 tw=100 */
