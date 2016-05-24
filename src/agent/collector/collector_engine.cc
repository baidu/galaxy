// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "collector_engine.h"
#include "timer.h"
#include "thread_pool.h"
#include "thread.h"

#include <glog/logging.h>
#include <boost/bind.hpp>

#include <assert.h>
#include <unistd.h>

namespace baidu {
namespace galaxy {
namespace collector {

CollectorEngine::CollectorEngine() :
    running_(false)
{
}

CollectorEngine::~CollectorEngine()
{
}

boost::shared_ptr<CollectorEngine> CollectorEngine::instance_(new CollectorEngine());

boost::shared_ptr<CollectorEngine> CollectorEngine::GetInstance()
{
    assert(NULL != instance_.get());
    return instance_;
}

baidu::galaxy::util::ErrorCode CollectorEngine::Register(boost::shared_ptr<Collector> collector)
{
    assert(NULL != collector.get());
    boost::mutex::scoped_lock lock(mutex_);

    for (size_t i = 0; i < collectors_.size(); i++) {
        if (collectors_[i]->GetCollector()->Equal(*collector)) {
            return ERRORCODE(-1, "collector %s alread registered", collector->Name().c_str());
        }
    }

    boost::shared_ptr<CollectorEngine::RuntimeCollector> rc(new CollectorEngine::RuntimeCollector(collector));
    collectors_.push_back(rc);
    return ERRORCODE_OK;
}

void CollectorEngine::Unregister(boost::shared_ptr<Collector> collector)
{
    assert(NULL != collector.get());
    boost::mutex::scoped_lock lock(mutex_);
    std::vector<boost::shared_ptr<RuntimeCollector> >::iterator iter = collectors_.begin();

    while (iter != collectors_.end()) {
        if ((*iter)->GetCollector()->Equal(*collector)) {
            (*iter)->GetCollector()->Enable(false);
            collectors_.erase(iter);
        }
    }
}

int CollectorEngine::Setup()
{
    assert(!running_);

    int ret = -1;
    if (main_collect_thread_.Start(boost::bind(&CollectorEngine::CollectMainThreadRoutine, this))) {
        ret = 0;
    }
    return ret;
}

void CollectorEngine::TearDown()
{
    running_ = false;
    main_collect_thread_.Join();
}

void CollectorEngine::CollectMainThreadRoutine()
{
    running_ = true;
    while (running_) {
        int64_t now = baidu::common::timer::get_micros();
        {
            boost::mutex::scoped_lock lock(mutex_);

            for (size_t i = 0; i < collectors_.size(); i++) {
                if (collectors_[i]->Expired(now)) {
                    if (collectors_[i]->IsRunning()) {
                        LOG(WARNING) << "last collection is not commplete: "
                                     << collectors_[i]->GetCollector()->Name();
                        continue;
                    }

                    if (!collectors_[i]->GetCollector()->Enabled()) {
                        LOG(WARNING) << "collector is not enabled: "
                                     << collectors_[i]->GetCollector()->Name();
                        continue;
                    }

                    collectors_[i]->SetRunning(true);
                    collector_pool_.AddTask(boost::bind(&CollectorEngine::CollectRoutine, this, collectors_[i]));
                    collectors_[i]->UpdateNextRuntime();
                }
            }
        }
        ::sleep(1);
    }
}

void CollectorEngine::CollectRoutine(boost::shared_ptr<CollectorEngine::RuntimeCollector> rc)
{
    rc->GetCollector()->Collect();
    rc->SetRunning(false);
}

}
}
}
