// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "subsystem_factory.h"
#include "cpu_subsystem.h"
#include "memory_subsystem.h"
#include "galaxy_memory_subsystem.h"
#include "freezer_subsystem.h"
#include "netcls_subsystem.h"
#include "cpuacct_subsystem.h"
#include "tcp_throt_subsystem.h"
#include "blkio_subsystem.h"

#include <assert.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

boost::shared_ptr<Subsystem> memory_;
boost::shared_ptr<Subsystem> galaxy_memory_;


SubsystemFactory::SubsystemFactory() :
    memory_(new baidu::galaxy::cgroup::MemorySubsystem()),
    galaxy_memory_(new baidu::galaxy::cgroup::GalaxyMemorySubsystem()) {
}

boost::shared_ptr<SubsystemFactory> SubsystemFactory::s_instance_(new SubsystemFactory());


boost::shared_ptr<SubsystemFactory> SubsystemFactory::GetInstance() {
    assert(NULL != s_instance_.get());
    return s_instance_;
}

SubsystemFactory* SubsystemFactory::Register(Subsystem* malloc_subsystem) {
    assert(NULL != malloc_subsystem);
    boost::shared_ptr<Subsystem> sub(malloc_subsystem);
    cgroups_seed_[sub->Name()] = sub;
    return this;
}

void SubsystemFactory::Setup() {
    this->Register(new baidu::galaxy::cgroup::CpuSubsystem())
    ->Register(new baidu::galaxy::cgroup::FreezerSubsystem())
    ->Register(new baidu::galaxy::cgroup::TcpThrotSubsystem())
    ->Register(new baidu::galaxy::cgroup::CpuacctSubsystem())
    ->Register(new baidu::galaxy::cgroup::NetclsSubsystem());
}

 
boost::shared_ptr<Subsystem> SubsystemFactory::CreateSubsystem(const std::string& name, bool use_galaxy) {
    boost::shared_ptr<Subsystem> ret;
    std::map<std::string, boost::shared_ptr<Subsystem> >::iterator iter = cgroups_seed_.find(name);

    if (cgroups_seed_.end() != iter) {
        ret = iter->second->Clone();
    } else if (name == "memory") {
        if (use_galaxy) {
            ret = galaxy_memory_->Clone();
        } else {
            ret = memory_->Clone();
        }
    }

    return ret;
}


void SubsystemFactory::GetSubsystems(std::vector<std::string>& subsystems) {
    subsystems.clear();
    std::map<std::string, boost::shared_ptr<Subsystem> >::iterator iter = cgroups_seed_.begin();

    while (iter != cgroups_seed_.end()) {
        subsystems.push_back(iter->first);
        iter++;
    }
    subsystems.push_back("memory");
}

} //namespace cgroup
} //namespace galaxy
} //namespace baidu
