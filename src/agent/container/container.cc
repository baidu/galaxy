

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
#include <iosfwd>
#include <boost/lexical_cast/lexical_cast_old.hpp>
#include <boost/bind.hpp>

#include <unistd.h>

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
    }

    if (cgroup_.size() != (unsigned int)desc_.cgroups_size()) {
        for (size_t i = 0; i < cgroup_.size(); i++) {
            cgroup_[i]->Destroy();
        }

        return -1;
    }

    // clone
    std::string container_root_path = baidu::galaxy::path::ContainerRootPath(id_);
    std::stringstream ss;
    int now = (int)time(NULL);
    ss << "stderr." << now;
    process_->RedirectStderr(ss.str());
    
    ss.str("stdout.");
    ss << now;
    process_->RedirectStdout(ss.str());
    
    if(0 != process_->Clone(boost::bind(&Container::RunRoutine, this, _1), NULL, 0)) {
        return -1;
    }
    return 0;
}


int Container::RunRoutine(void*) {

    // construct volum in child process
    volum_group_->SetContainerId(id_);
    volum_group_->SetWorkspaceVolum(desc_.workspace_volum());

    for (int i = 0; i < desc_.data_volums_size(); i++) {
        volum_group_->AddDataVolum(desc_.data_volums(i));
    }
    
    if (0 != Construct()) {
        return -1;
    }
    
    // mount root fs
    if (0 != baidu::galaxy::volum::VolumGroup::MountRootfs()) {
        return -1;
    }

    // mount workspace & datavolum
    if (0 != volum_group_->Mount(desc_.run_user())) {
        return -1;
    }

    // change root
    if(0 != ::chroot(baidu::galaxy::path::ContainerRootPath(Id()).c_str())) {
        return -1;
    }
    
    // change user or sh -l
    if(!baidu::galaxy::user::Su(desc_.run_user())) {
        return -1;
    }
    
    // start appworker 
    char* argv[] = {
        const_cast<char*>("sh"),
        const_cast<char*>("-c"),
        const_cast<char*>(desc_.cmd_line().c_str()),
        NULL};
    ::execv("/bin/sh", argv);
    return 0;
}

int Container::Destroy() {
    // destroy cgroup
    for (size_t i = 0; i < cgroup_.size(); i++) {
        if (0 != cgroup_[i]->Destroy()) {
            return -1;
        }
    }

    // destroy volum
    
    return 0;
}

int Container::Tasks(std::vector<pid_t>& pids) {
    return -1;
}

int Container::Pids(std::vector<pid_t>& pids) {
    return -1;
}

boost::shared_ptr<google::protobuf::Message> Container::Status() {
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

