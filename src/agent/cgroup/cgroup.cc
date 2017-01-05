// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup.h"
#include "subsystem_factory.h"
#include "subsystem.h"
#include "freezer_subsystem.h"
#include "protocol/galaxy.pb.h"
#include "protocol/agent.pb.h"
#include "subsystem.h"
#include "collector/collector_engine.h"
#include "cgroup_collector.h"
#include <glog/logging.h>

#include <unistd.h>

namespace baidu {
namespace galaxy {
namespace cgroup {


Cgroup::Cgroup(const boost::shared_ptr<SubsystemFactory> factory) :
    factory_(factory) {
}

Cgroup::~Cgroup() {
}

void Cgroup::SetContainerId(const std::string& container_id) {
    container_id_ = container_id;
}

void Cgroup::SetDescrition(boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup) {
    cgroup_ = cgroup;
}

baidu::galaxy::util::ErrorCode Cgroup::Construct() {
    assert(subsystem_.empty());
    std::vector<std::string> subsystems;
    factory_->GetSubsystems(subsystems);
    assert(subsystems.size() > 0);
    bool ok = true;
    bool use_galaxy_killer = false;
    if (cgroup_->memory().has_use_galaxy_killer() 
                && cgroup_->memory().use_galaxy_killer()) {
        use_galaxy_killer = true;
    }

    for (size_t i = 0; i < subsystems.size(); i++) {
        boost::shared_ptr<Subsystem> ss = factory_->CreateSubsystem(subsystems[i], use_galaxy_killer);
        assert(NULL != ss.get());
        ss->SetContainerId(container_id_);
        ss->SetCgroup(cgroup_);

        if (subsystems[i] == "freezer") {
            freezer_ = boost::dynamic_pointer_cast<FreezerSubsystem>(ss);
            assert(NULL != freezer_.get());
        } else {
            if (subsystems[i] == "cpuacct") {
                cpu_acct_ = ss;
            } else if ("memory" == subsystems[i]) {
                memory_ = ss;
            }

            subsystem_.push_back(ss);
        }

        baidu::galaxy::util::ErrorCode err = ss->Construct();

        if (0 != err.Code()) {
            LOG(WARNING) << "construct subsystem " << ss->Name().c_str()
                         << " for cgroup " << ss->Path().c_str()
                         << " failed: "
                         << err.Message();
            ok = false;
            break;
        }

        LOG(INFO) << "create subsystem " << ss->Name()
                  << "(path: " << ss->Path()
                  << ") successfully for container " << container_id_;
    }

    if (!ok) {
        // destroy
        for (size_t i = 0; i < subsystem_.size(); i++) {
            subsystem_[i]->Destroy();
        }

        subsystem_.clear();

        if (NULL != freezer_.get()) {
            freezer_->Destroy();
            freezer_.reset();
        }

        return ERRORCODE(-1, "");
    }

    collector_.reset(new CgroupCollector());
    collector_->SetCpuacctPath(cpu_acct_->Path() + "/cpuacct.stat");
    collector_->SetMemoryPath(memory_->Path() + "/memory.usage_in_bytes");
    collector_->SetCycle(5);
    collector_->SetName(container_id_ + "_cgroup");
    collector_->Enable(true);
    baidu::galaxy::collector::CollectorEngine::GetInstance()->Register(collector_, true);
    VLOG(10) << "register cgroup collector: " << container_id_ << " " << (int64_t)this;
    return ERRORCODE_OK;
}

// Fixme: freeze first, and than kill
baidu::galaxy::util::ErrorCode Cgroup::Destroy() {
    boost::mutex::scoped_lock lock(mutex_);

    if (collector_.get() != NULL) {
        VLOG(10) << "disable cgroup collector: " << container_id_ << " " << (int64_t)this;
        collector_->Enable(false);
    }

    if (subsystem_.empty() && NULL == freezer_.get()) {
        return ERRORCODE_OK;
    }

    if (freezer_.get() != NULL) {
        baidu::galaxy::util::ErrorCode err = freezer_->Freeze();

        if (0 != err.Code()) {
            return ERRORCODE(-1, "freeze failed: %s",
                       err.Message().c_str());
        }

        for (size_t i = 0; i < subsystem_.size(); i++) {
            subsystem_[i]->Kill();
        }

        err = freezer_->Thaw();

        if (0 != err.Code()) {
            return ERRORCODE(-1,
                    "thaw failed: %s",
                    err.Message().c_str());
        }
    }

    for (size_t i = 0; i < subsystem_.size(); i++) {
        baidu::galaxy::util::ErrorCode ec = subsystem_[i]->Destroy();

        if (0 != ec.Code()) {
            return ERRORCODE(-1,
                    "failed in destroying %s: %s",
                    subsystem_[i]->Name().c_str(),
                    ec.Message().c_str());
        }
    }

    subsystem_.clear();

    if (NULL !=  freezer_.get()) {
        freezer_->Freeze();
        freezer_->Kill();
        freezer_->Thaw();
        baidu::galaxy::util::ErrorCode ec = freezer_->Destroy();

        if (0 != ec.Code()) {
            return ERRORCODE(-1, "failed in destroying freezer:",
                    ec.Message().c_str());
        }

        freezer_.reset();
    }

    return ERRORCODE_OK;
}

void Cgroup::ExportEnv(std::map<std::string, std::string>& env) {
    std::vector<std::string> subsystems;
    factory_->GetSubsystems(subsystems);
    std::string value;

    for (size_t i = 0; i < subsystems.size(); i++) {
        if (!value.empty()) {
            value += ",";
        }

        value += subsystems[i];
    }

    env["baidu_galaxy_cgroup_subsystems"] = value;

    for (size_t i = 0; i < subsystem_.size(); i++) {
        std::stringstream ss;
        ss << "baidu_galaxy_container_" << cgroup_->id() << "_" << subsystem_[i]->Name() << "_path";
        env[ss.str()] = subsystem_[i]->Path();
    }

    {
        std::stringstream ss;
        ss << "baidu_galaxy_container_" << cgroup_->id() << "_" << freezer_->Name() << "_path";
        env[ss.str()] = freezer_->Path();
    }

    // export port
    std::string port_names;

    for (int i = 0; i < cgroup_->ports_size(); i++) {
        std::stringstream ss;
        const baidu::galaxy::proto::PortRequired& pr = cgroup_->ports(i);

        if (!port_names.empty()) {
            port_names += ",";
        }

        port_names += pr.port_name();
        ss << "baidu_galaxy_container_" << cgroup_->id() << "_port_" << pr.port_name();
        env[ss.str()] = pr.real_port();
    }

    std::stringstream ss;
    ss << "baidu_galaxy_container_" << cgroup_->id() << "_portlist";
    env[ss.str()] = port_names;
}

std::string Cgroup::Id() {
    return cgroup_->id();
}


baidu::galaxy::util::ErrorCode Cgroup::Collect(boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> metrix) {
    assert(NULL != metrix);
    boost::mutex::scoped_lock lock(mutex_);

    for (size_t i = 0; i < subsystem_.size(); i++) {
        baidu::galaxy::util::ErrorCode ec = subsystem_[i]->Collect(metrix);

        if (ec.Code() != 0) {
            return ERRORCODE(-1, "collect %s failed: %s",
                    subsystem_[i]->Name().c_str(),
                    ec.Message().c_str());
        }
    }

    return ERRORCODE_OK;
}


boost::shared_ptr<baidu::galaxy::proto::CgroupMetrix> Cgroup::Statistics() {
    return collector_->Statistics();
}

}
}
}


