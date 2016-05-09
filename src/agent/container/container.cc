

// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "container.h"

#include "cgroup/subsystem_factory.h"
#include "cgroup/cgroup.h"
#include "protocol/galaxy.pb.h"
#include "volum/volum_group.h"
#include "agent/util/user.h"
#include "agent/util/path_tree.h"
#include "agent/cgroup/cgroup.h"
#include "process.h"
#include "boost/filesystem/operations.hpp"
#include "boost/algorithm/string/case_conv.hpp"

#include <glog/logging.h>
#include <boost/lexical_cast/lexical_cast_old.hpp>
#include <boost/bind.hpp>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <iosfwd>
#include <iostream>

namespace baidu {
namespace galaxy {
namespace container {

Container::Container(const std::string& id, const baidu::galaxy::proto::ContainerDescription& desc) :
    desc_(desc),
    volum_group_(new baidu::galaxy::volum::VolumGroup()),
    process_(new Process()), 
    id_(id) {
}

Container::~Container() {
}

std::string Container::Id() const {
    return id_;
}

int Container::Construct() {
    assert(!id_.empty());
    // cgroup
    LOG(INFO) << "to create cgroup for container" << id_.c_str()
        << ", expect cgroup size is " << desc_.cgroups_size();

    for (int i = 0; i < desc_.cgroups_size(); i++) {
        boost::shared_ptr<baidu::galaxy::cgroup::Cgroup> cg(new baidu::galaxy::cgroup::Cgroup(
                        baidu::galaxy::cgroup::SubsystemFactory::GetInstance()));
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> desc(new baidu::galaxy::proto::Cgroup());
        desc->CopyFrom(desc_.cgroups(i));
        cg->SetContainerId(id_);
        cg->SetDescrition(desc);

        if (0 != cg->Construct()) {
            break;
        }

        cgroup_.push_back(cg);

        LOG(INFO) << "succed in creating cgroup(" << desc_.cgroups(i).id() 
            << ") for cotnainer " << id_; 
    }

    if (cgroup_.size() != (unsigned int)desc_.cgroups_size()) {
        LOG(WARNING) << "create cgroup for container " << id_.c_str() 
                     << " failed, expect size is " << desc_.cgroups_size() 
                     << " real size is " << cgroup_.size();

        for (size_t i = 0; i < cgroup_.size(); i++) {
            cgroup_[i]->Destroy();
        }
        return -1;
    }

    LOG(INFO) << "succed in creating cgroup for container " << id_.c_str();

    // clone
    LOG(INFO) << "to clone appwork process for container " << id_.c_str();
    std::string container_root_path = baidu::galaxy::path::ContainerRootPath(id_);
    std::stringstream ss;
    int now = (int)time(NULL);
    ss << "stderr." << now;
    process_->RedirectStderr(ss.str());
    
    ss.str("");
    ss << "stdout." << now;
    process_->RedirectStdout(ss.str());
    
    if(0 != process_->Clone(boost::bind(&Container::RunRoutine, this, _1), NULL, 0)) {
        LOG(INFO) << "failed to clone appwork process for container " << id_.c_str();
        return -1;
    }

    LOG(WARNING) << "sucessed in cloning appwork process (pid is " << process_->Pid()
                 << " for container " << id_.c_str();
    return 0;
}


int Container::RunRoutine(void*) {

    // construct volum in child process
    volum_group_->SetContainerId(id_);
    volum_group_->SetWorkspaceVolum(desc_.workspace_volum());

    for (int i = 0; i < desc_.data_volums_size(); i++) {
        volum_group_->AddDataVolum(desc_.data_volums(i));
    }

    // mount volum
    if (0 != volum_group_->Construct()) {
        LOG(WARNING) << "construct volum group failed";
        return -1;
    }
    LOG(INFO) << "succed in constructing volum group";
   
    // mount root fs
    if (0 != volum_group_->MountRootfs()) {
        std::cerr << "mount root fs failed" << std::endl;
        return -1;
    }
    LOG(INFO) << "succed in mounting root fs";

    // change root
    if(0 != ::chroot(baidu::galaxy::path::ContainerRootPath(Id()).c_str())) {
        LOG(WARNING) << "chroot failed: " << strerror(errno);
        return -1;
    }
    LOG(INFO) << "chroot successfully:" <<baidu::galaxy::path::ContainerRootPath(Id()).c_str();
    
    // change user or sh -l
    baidu::galaxy::util::ErrorCode ec = baidu::galaxy::user::Su(desc_.run_user());
    if(ec.Code() != 0) {
        LOG(WARNING) << "su user " << desc_.run_user() << " failed: " << ec.Message();
        return -1;
    }
    LOG(INFO) << "su user " << desc_.run_user() << " sucessfully";
    
    // export env
        
   
    // start appworker 
    LOG(INFO) << "start cmd: sh -c " << desc_.cmd_line();
    char* argv[] = {
        const_cast<char*>("sh"),
        const_cast<char*>("-c"),
        const_cast<char*>(desc_.cmd_line().c_str()),
        NULL};
    ::execv("/bin/sh", argv);

    LOG(WARNING) << "exec cmd " << desc_.cmd_line() << " failed: " << strerror(errno); 
    return -1;
}

void Container::ExportEnv() {
    std::map<std::string, std::string> env;
    for (size_t i = 0; i < cgroup_.size(); i++) {
        cgroup_[i]->ExportEnv(env);
    }
    
    volum_group_->ExportEnv(env);
    
    this->ExportEnv(env);
    
    std::map<std::string, std::string>::const_iterator iter = env.begin();
    while(iter != env.end()) {
        int ret = ::setenv(boost::to_upper_copy(iter->first).c_str(), boost::to_upper_copy(iter->second).c_str(), 1);
        if (0 != ret) {
            LOG(FATAL) << "set env failed for container " << id_;
        }
        iter++;
    }
}

void Container::ExportEnv(std::map<std::string, std::string>& env) {
    env["baidu_galaxy_container_id"] = id_;
    env["baidu_galaxy_container_cgroup_size"] = boost::lexical_cast<std::string>(cgroup_.size());
     
    for (size_t i = 0; i < cgroup_.size(); i++) {
        std::string key = "baidu_galaxy_container_cgroup_id" + boost::lexical_cast<std::string>(i);
        env[key] = cgroup_[i]->Id();
    }
}

int Container::Destroy() {
    // destroy cgroup
    for (size_t i = 0; i < cgroup_.size(); i++) {
        if (0 != cgroup_[i]->Destroy()) {
            return -1;
        }
    }
    
    return 0;
}

int Container::Tasks(std::vector<pid_t>& pids) {
    return -1;
}

int Container::Pids(std::vector<pid_t>& pids) {
    return -1;
}

boost::shared_ptr<google::protobuf::Message> Container::Report() {
    boost::shared_ptr<google::protobuf::Message> ret;
    return ret;
}

int Container::Attach(pid_t pid) {
    return -1;
}

int Container::Detach(pid_t pid) {
    return -1;
}

int Container::Detachall() {
    return -1;
}

} //namespace container
} //namespace galaxy
} //namespace baidu

