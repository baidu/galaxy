// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgroup.h"
#include "subsystem_factory.h"
#include "subsystem.h"
#include "freezer_subsystem.h"
#include "protocol/galaxy.pb.h"
#include "subsystem.h"
#include <glog/logging.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

namespace baidu {
namespace galaxy {
namespace cgroup {

Cgroup::Cgroup(const boost::shared_ptr<SubsystemFactory> factory) :
    factory_(factory)
{
}

Cgroup::~Cgroup()
{
}

void Cgroup::SetContainerId(const std::string& container_id)
{
    container_id_ = container_id;
}

void Cgroup::SetDescrition(boost::shared_ptr<baidu::galaxy::proto::Cgroup> cgroup)
{
    cgroup_ = cgroup;
}

int Cgroup::Construct()
{
    assert(subsystem_.empty());
    std::vector<std::string> subsystems;
    factory_->GetSubsystems(subsystems);
    assert(subsystems.size() > 0);

    bool ok = true;

    for (size_t i = 0; i < subsystems.size(); i++) {
        boost::shared_ptr<Subsystem> ss = factory_->CreateSubsystem(subsystems[i]);
        assert(NULL != ss.get());
        ss->SetContainerId(container_id_);
        ss->SetCgroup(cgroup_);

        if (subsystems[i] == "freezer") {
            freezer_ = boost::dynamic_pointer_cast<FreezerSubsystem>(ss);
            assert(NULL != freezer_.get());
        } else {
            subsystem_.push_back(ss);
        }

        if (0 != ss->Construct()) {
            LOG(WARNING) << "construct subsystem " << ss->Name().c_str()
                         << " for cgroup " << ss->Path().c_str()
                         << " failed";
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

        return -1;
    }

    return 0;
}

// Fixme: freeze first, and than kill
baidu::galaxy::util::ErrorCode Cgroup::Destroy()
{
    if (subsystem_.empty() && NULL == freezer_.get()) {
        return ERRORCODE_OK;
    }

    int ret = 0;

    if (0 != freezer_->Freeze()) {
        return ERRORCODE(-1, "freeze failed");
    }

    std::vector<int> pids;
    freezer_->GetProcs(pids);
    for (size_t i = 0; i < pids.size(); i++) {
        ::kill(pids[i], SIGKILL);
    }

    if (0 != freezer_->Thaw()) {
        return ERRORCODE(-1, "thaw failed");
    }

    pids.clear();
    freezer_->GetProcs(pids);
    if (pids.size() > 0) {
        return ERRORCODE(-1, "kill pid failed");
    }

    ::usleep(1000);
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
        baidu::galaxy::util::ErrorCode ec = freezer_->Destroy();
        if (0 != ec.Code()) {
            return ERRORCODE(-1, "failed in destroying freezer:",
                    ec.Message().c_str());
        }
        freezer_.reset();
    }

    return ERRORCODE_OK;
}

boost::shared_ptr<google::protobuf::Message> Cgroup::Report()
{
    boost::shared_ptr<google::protobuf::Message> ret;
    return ret;
}

void Cgroup::ExportEnv(std::map<std::string, std::string>& env)
{
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

std::string Cgroup::Id()
{
    return cgroup_->id();
}

void Cgroup::Statistics(Metrix& matrix)
{
    for (size_t i = 0; i < subsystem_.size(); i++) {
        baidu::galaxy::util::ErrorCode ec = subsystem_[i]->Collect(matrix);
        if (ec.Code() != 0) {
            LOG(WARNING) << "collect metrix failed: " << ec.Message();
        }
    }
}

}
}
}


