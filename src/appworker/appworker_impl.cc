// Copyright (C) 2016, Copyright (C) 

#include "appworker_impl.h"

#include <boost/bind.hpp>
#include <gflags/gflags.h>

DECLARE_int32(appworker_fetch_task_interval);
DECLARE_int32(appworker_background_thread_pool_size);
DECLARE_string(appmaster_nexus_path);

namespace baidu {
namespace galaxy {

AppWorkerImpl::AppWorkerImpl() :
    mutex_appworker_(),
    backgroud_thread_pool_(FLAGS_appworker_background_thread_pool_size),
    rpc_client_(NULL),
    appmaster_stub_(NULL) {
    LOG(INFO) << "threads: " << FLAGS_appworker_background_thread_pool_size;
    LOG(INFO) << "interval: " << FLAGS_appworker_fetch_task_interval;
    LOG(INFO) << "appmaster nexus path: " << FLAGS_appmaster_nexus_path;
    rpc_client_ = new RpcClient();

}

AppWorkerImpl::~AppWorkerImpl () {
    backgroud_thread_pool_.Stop(false);
    if (rpc_client_ != NULL) {
        delete rpc_client_;
        rpc_client_ = NULL;
    }
    LOG(INFO) << "appworker destroyed";
}

int AppWorkerImpl::Init() {
    LOG(INFO) << "AppWorkerImpl init.";
    backgroud_thread_pool_.DelayTask(FLAGS_appworker_fetch_task_interval,
            boost::bind(&AppWorkerImpl::FetchTask, this));

    return 0;
}

void AppWorkerImpl::FetchTask () {
    LOG(INFO) << "fetch task called";
    backgroud_thread_pool_.DelayTask(FLAGS_appworker_fetch_task_interval,
            boost::bind(&AppWorkerImpl::FetchTask, this));
}


}   // ending namespace galaxy
}   // ending namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
