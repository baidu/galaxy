// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <boost/bind.hpp>
#include "curl/curl.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <math.h>
#include "private_sdk.h"

namespace baidu {
namespace galaxy {

CurlThreadPool* CurlThreadPool::instance_ = NULL;

PrivateSubscribeSdk::PrivateSubscribeSdk (const std::string& service_name,
								int update_interval,
								int timeout_interval) {
	service_name_.assign(service_name);
	update_interval_ = update_interval;
	timeout_interval_ = timeout_interval;
	init_ = false;
	add_func_ = NULL;
	del_func_ = NULL;
	chg_func_ = NULL;	
}

void PrivateSubscribeSdk::RegistAddHandler(HandleFunc func) {
	add_func_ = func;
	return;
}

void PrivateSubscribeSdk::RegistDelHandler(HandleFunc func) {
	del_func_ = func;
	return;
}

void PrivateSubscribeSdk::RegistRefreshHandler(HandleFunc func) {
	chg_func_ = func;
	return;
}

void PrivateSubscribeSdk::GetServiceList(std::vector<std::string>* addr_list) {
	MutexLock lock(&mutex_);
	if(!init_) {
		return;
	}
	addr_list->assign(addr_list_.begin(), addr_list_.end());
	return;
}

void PrivateSubscribeSdk::ReloadService() {
    return;
}

void PrivateSubscribeSdk::Init() {
	if (init_) {
		return;
	}
	ReloadService();
	init_ = true;
	return;
}

PrivatePublicSdk::PrivatePublicSdk(std::string service_name,
						std::string token,
						std::string tag,
						std::string health_check,
						std::string health_script) {
	CurlThreadPool* pool = CurlThreadPool::GetInstance();
    curl_worker_ = pool->GetThreadPool();
	service_name_.assign(service_name);
	token_.assign(token);
	tag_.assign(tag);
	health_check_.assign(health_check);
	health_script_.assign(health_script);
	init_ = false;
    time_out_ = 3;
}

PrivatePublicSdk::~PrivatePublicSdk(){
}

void PrivatePublicSdk::Init() {
	if (init_) {
		return;
	}
    curl_global_init(CURL_GLOBAL_ALL);
    init_ = true;
	//CheckService();
	return;
}

void PrivatePublicSdk::AddServiceInstance(std::string podid, const ServiceInfo &service) {
    MutexLock lock(&mutex_);
    std::stringstream ss(service.port());
    int port;
    ss >> port;
    //uint32_t ip = Ip2Int(service.ip()); 
    size_t pod = podid.rfind("_");
    std::string pod_offset(podid, pod + 1, podid.size() - (pod + 1));
    std::stringstream ss_offset(pod_offset);
    int offset;
    std::string str = ss_offset.str();
    ss_offset >> offset;
    size_t site = service.hostname().rfind(".baidu.com");
    std::string host(service.hostname(), 0, site);
	return;
}

void PrivatePublicSdk::DelServiceInstance(const std::string podid) {
    MutexLock lock(&mutex_);
    size_t pod = podid.rfind("_");
    std::string pod_offset(podid, pod + 1, podid.size() - (pod + 1));
    std::stringstream ss;
    ss << pod_offset;
    int offset;
    ss >> offset;
    LOG(INFO) << __FUNCTION__ << " " << podid;
	return;
}

}
}
