// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "container.h"
#include "process.h"

#include "cgroup/subsystem_factory.h"
#include "cgroup/cgroup.h"
#include "cgroup/cgroup_collector.h"
#include "protocol/galaxy.pb.h"
#include "volum/volum_group.h"
#include "util/user.h"
#include "util/path_tree.h"
#include "util/error_code.h"

#include "boost/filesystem/operations.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "collector/collector_engine.h"
#include "cgroup/cgroup_collector.h"

#include <glog/logging.h>
#include <boost/lexical_cast/lexical_cast_old.hpp>
#include <boost/bind.hpp>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <iosfwd>
#include <fstream>
#include <iostream>
#include <sstream>


DECLARE_string(agent_ip);
DECLARE_string(agent_port);
DECLARE_string(agent_hostname);
DECLARE_string(cmd_line);

namespace baidu {
namespace galaxy {
namespace container {

bool is_null(const std::string& x)
{
    return x.empty();
}

Container::Container(const ContainerId& id, const baidu::galaxy::proto::ContainerDescription& desc) :
    desc_(desc),
    volum_group_(new baidu::galaxy::volum::VolumGroup()),
    process_(new Process()),
    id_(id),
    status_(id.SubId())
{
}

Container::~Container()
{
}

ContainerId Container::Id() const
{
    return id_;
}

int Container::Construct()
{
    baidu::galaxy::util::ErrorCode ec = status_.EnterAllocating();

    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(WARNING) << ec.Message();
        return 0;
    }

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << "construct failed " << id_.CompactId() << ": " << ec.Message();
        return -1;
    }

    int ret = Construct_();

    if (0 != ret) {
        ec = status_.EnterError();
        LOG(WARNING) << "construct contaier " << id_.CompactId() << " failed";

        if (ec.Code() != baidu::galaxy::util::kErrorOk) {
            LOG(FATAL) << "contaner " << id_.CompactId() << ": " << ec.Message();
        }
    } else {
        ec = status_.EnterReady();
        created_time_ = baidu::common::timer::get_micros();
        if (ec.Code() != baidu::galaxy::util::kErrorOk) {
            LOG(FATAL) << "container " << id_.CompactId() << ": " << ec.Message();
        }

        // register collector
        for (size_t i = 0; i < cgroup_.size(); i++) {
            boost::shared_ptr<baidu::galaxy::collector::Collector> c(new baidu::galaxy::cgroup::CgroupCollector(cgroup_[i]));
            baidu::galaxy::collector::CollectorEngine::GetInstance()->Register(c);
            collectors_.push_back(c);
        }
    }

    if (0 == ret) {
        LOG(INFO) << "sucess in constructing container " << id_.CompactId();
    } else {
        LOG(INFO) << "failed to construct container " << id_.CompactId();
    }
    return ret;
}

int Container::Destroy()
{
    baidu::galaxy::util::ErrorCode ec = status_.EnterDestroying();

    if (ec.Code() == baidu::galaxy::util::kErrorRepeated) {
        LOG(WARNING) << "container  " << id_.CompactId() << " is in kContainerDestroying status: " << ec.Message();
        return 0;
    }

    if (ec.Code() != baidu::galaxy::util::kErrorOk) {
        LOG(WARNING) << "destroy container " << id_.CompactId() << " failed: " << ec.Message();
        return -1;
    }

    int ret = Destroy_();

    if (0 != ret) {
        ec = status_.EnterError();

        if (ec.Code() != baidu::galaxy::util::kErrorOk) {
            LOG(FATAL) << "destroy container " << id_.CompactId() << " failed: " << ec.Message();
        }
    } else {
        ec = status_.EnterTerminated();

        if (ec.Code() != baidu::galaxy::util::kErrorOk) {
            LOG(FATAL) << "destroy container " << id_.CompactId() << " failed: " << ec.Message();
        }
    }

    return ret;
}

int Container::Construct_()
{
    assert(!id_.Empty());
    // cgroup
    LOG(INFO) << "to create cgroup for container " << id_.CompactId()
              << ", expect cgroup size is " << desc_.cgroups_size();

    for (int i = 0; i < desc_.cgroups_size(); i++) {
        boost::shared_ptr<baidu::galaxy::cgroup::Cgroup> cg(new baidu::galaxy::cgroup::Cgroup(
                baidu::galaxy::cgroup::SubsystemFactory::GetInstance()));
        boost::shared_ptr<baidu::galaxy::proto::Cgroup> desc(new baidu::galaxy::proto::Cgroup());
        desc->CopyFrom(desc_.cgroups(i));
        cg->SetContainerId(id_.SubId());
        cg->SetDescrition(desc);

        baidu::galaxy::util::ErrorCode err = cg->Construct();
        if (0 != err.Code()) {
            LOG(WARNING) << "fail in constructing cgroup, cgroup id is " << cg->Id()
                         << ", container id is " << id_.CompactId();
            break;
        }

        cgroup_.push_back(cg);
        LOG(INFO) << "succeed in constructing cgroup(" << desc_.cgroups(i).id()
                  << ") for cotnainer " << id_.CompactId();
    }

    if (cgroup_.size() != (unsigned int)desc_.cgroups_size()) {
        LOG(WARNING) << "fail in constructing cgroup for container " << id_.CompactId()
                     << ", expect cgroup size is " << desc_.cgroups_size()
                     << " real size is " << cgroup_.size();

        for (size_t i = 0; i < cgroup_.size(); i++) {
            baidu::galaxy::util::ErrorCode err = cgroup_[i]->Destroy();
            if (err.Code() != 0) {
                LOG(WARNING) << id_.CompactId()
                    << " construc failed and destroy failed: " 
                    << err.Message();
            }
        }

        return -1;
    }

    LOG(INFO) << "succeed in creating cgroup for container " << id_.CompactId();
    volum_group_->SetContainerId(id_.SubId());
    volum_group_->SetWorkspaceVolum(desc_.workspace_volum());
    volum_group_->SetGcIndex(created_time_);

    for (int i = 0; i < desc_.data_volums_size(); i++) {
        volum_group_->AddDataVolum(desc_.data_volums(i));
    }

    baidu::galaxy::util::ErrorCode ec = volum_group_->Construct();
    if (0 != ec.Code()) {
        LOG(WARNING) << "failed in constructing volum group for container " << id_.CompactId()
                     << ", reason is: " << ec.Message();
        return -1;
    }

    LOG(INFO) << "succeed in constructing volum group for container " << id_.CompactId();
    // clone
    LOG(INFO) << "to clone appwork process for container " << id_.CompactId();
    std::string container_root_path = baidu::galaxy::path::ContainerRootPath(id_.SubId());

    int now = (int)time(NULL);
    std::stringstream ss;
    ss << container_root_path << "/stderr." << now;
    process_->RedirectStderr(ss.str());
    LOG(INFO) << "redirect stderr to " << ss.str() << " for container " << id_.CompactId();

    ss.str("");
    ss << container_root_path << "/stdout." << now;
    process_->RedirectStdout(ss.str());
    LOG(INFO) << "redirect stdout to " << ss.str() << " for container " << id_.CompactId();
    pid_t pid = process_->Clone(boost::bind(&Container::RunRoutine, this, _1), NULL, 0);

    if (pid <= 0) {
        LOG(INFO) << "fail in clonning appwork process for container " << id_.CompactId();
        return -1;
    }

    LOG(INFO) << "succeed in cloning appwork process (pid is " << process_->Pid()
              << " for container " << id_.CompactId();
    return 0;
}

int Container::Destroy_()
{
    // kill appwork
    pid_t pid = process_->Pid();
    if (pid > 0) {
        baidu::galaxy::util::ErrorCode ec = Process::Kill(pid);
        if (ec.Code() != 0) {
            LOG(WARNING) << "failed in killing appwork for container "
                         << id_.CompactId() << ": " << ec.Message();
            return -1;
        }
    }

    LOG(INFO) << "container " << id_.CompactId() << " suceed in killing appwork whose pid is " << pid;
    // destroy cgroup
    for (size_t i = 0; i < cgroup_.size(); i++) {
        baidu::galaxy::util::ErrorCode ec = cgroup_[i]->Destroy();
        if (0 != ec.Code()) {
            LOG(WARNING) << "container " << id_.CompactId()
                         << " failed in destroying cgroup: " << ec.Message();
            return -1;
        }
        LOG(INFO) << "container " << id_.CompactId() << " suceed in destroy cgroup";
    }

    // destroy volum
    baidu::galaxy::util::ErrorCode ec = volum_group_->Destroy();
    if (0 != ec.Code()) {
        LOG(WARNING) << "failed in destroying volum group in container " << id_.CompactId()
                     << " " << ec.Message();
        return -1;
    }
    LOG(INFO) << "container " << id_.CompactId() << " suceed in destroy volum";

    // mv to gc queue
    return 0;
}

int Container::RunRoutine(void*)
{
    // mount root fs
    if (0 != volum_group_->MountRootfs()) {
        std::cout << "mount root fs failed" << std::endl;
        return -1;
    }

    ::chdir(baidu::galaxy::path::ContainerRootPath(Id().SubId()).c_str());

    std::cout << "succed in mounting root fs\n";
    // change root
    if (0 != ::chroot(baidu::galaxy::path::ContainerRootPath(Id().SubId()).c_str())) {
        std::cout << "chroot failed: " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "chroot successfully:" << baidu::galaxy::path::ContainerRootPath(Id().SubId()) << std::endl;
    // change user or sh -l
    //baidu::galaxy::util::ErrorCode ec = baidu::galaxy::user::Su(desc_.run_user());
    //if (ec.Code() != 0) {
    //    std::cout << "su user " << desc_.run_user() << " failed: " << ec.Message() << std::endl;
    //    return -1;
    //}
    std::cout << "su user " << desc_.run_user() << " sucessfully" << std::endl;


    // export env
    // start appworker
    std::cout << "start cmd: /bin/sh -c " << desc_.cmd_line() << std::endl;
    std::string cmd_line = FLAGS_cmd_line;
    //char* argv[] = {"cat", NULL};
    char* argv[] = {
        const_cast<char*>("sh"),
        const_cast<char*>("-c"),
        //const_cast<char*>(desc_.cmd_line().c_str()),
        const_cast<char*>(cmd_line.c_str()),
        NULL
    };

    ExportEnv();
    ::execv("/bin/sh", argv);
    std::cout << "exec cmd " << cmd_line << " failed: " << strerror(errno) << std::endl;
    return -1;
}

void Container::ExportEnv()
{
    std::map<std::string, std::string> env;

    for (size_t i = 0; i < cgroup_.size(); i++) {
        cgroup_[i]->ExportEnv(env);
    }

    volum_group_->ExportEnv(env);
    this->ExportEnv(env);
    std::map<std::string, std::string>::const_iterator iter = env.begin();

    while (iter != env.end()) {
        int ret = ::setenv(boost::to_upper_copy(iter->first).c_str(), iter->second.c_str(), 1);

        if (0 != ret) {
            LOG(FATAL) << "set env failed for container " << id_.CompactId();
        }

        iter++;
    }
}

void Container::ExportEnv(std::map<std::string, std::string>& env)
{
    env["baidu_galaxy_containergroup_id"] = id_.GroupId();
    env["baidu_galaxy_container_id"] = id_.SubId();
    std::string ids;

    for (size_t i = 0; i < cgroup_.size(); i++) {
        if (!ids.empty()) {
            ids += ",";
        }

        ids += cgroup_[i]->Id();
    }

    env["baidu_galaxy_container_cgroup_ids"] = ids;
    env["baidu_galaxy_agent_hostname"] = FLAGS_agent_hostname;
    env["baidu_galaxy_agent_ip"] = FLAGS_agent_ip;
    env["baidu_galaxy_agent_port"] = FLAGS_agent_port;
}

baidu::galaxy::proto::ContainerStatus Container::Status()
{
    return status_.Status();
}

void Container::KeepAlive()
{
    int64_t now = baidu::common::timer::get_micros();
    if (now - created_time_ < 2000000L) {
        return;
    }

    if (status_.Status() != baidu::galaxy::proto::kContainerReady) {
        return;
    }

    if (!Alive()) {
        boost::filesystem::path exit_file(baidu::galaxy::path::ContainerRootPath(id_.SubId()));
        exit_file.append(".exit");
        boost::system::error_code ec;
        if (boost::filesystem::exists(exit_file, ec)) {
            baidu::galaxy::util::ErrorCode ec = status_.EnterFinished();
            if (ec.Code() != 0) {
                LOG(WARNING) << "container " << id_.CompactId() 
                    << " failed in entering finished status" << ec.Message();
            } else {
                LOG(INFO) << "container " << id_.CompactId() << " enter finished status";
            }

        } else {
            baidu::galaxy::util::ErrorCode ec = status_.EnterErrorFrom(baidu::galaxy::proto::kContainerReady);
            if (ec.Code() != 0) {
                LOG(WARNING) << "container " << id_.CompactId() 
                    << " failed in entering error status from kContainerReady:" << ec.Message();
            } else {
                LOG(INFO) << "container " << id_.CompactId() << " enter error status from kContainerReady";
            }

        }
    } else {
        LOG(INFO) << "i am alive ...";
    }
}

bool Container::Alive()
{
    int pid = (int)process_->Pid();
    if (pid <= 0) {
        return false;
    }

    std::stringstream path;
    path << "/proc/" << (int)pid << "/environ";
    FILE* file = fopen(path.str().c_str(), "rb");
    if (NULL == file) {
        LOG(WARNING) << "failed in openning file " << path.str() << ": " << strerror(errno);
        return false;
    }


    char buf[1024] = {0};
    std::string env_str;

    while (!feof(file)) {
        int size = fread(buf, 1, sizeof buf, file);
        env_str.append(buf, size);
    }
    fclose(file);

    for (size_t i = 0; i < env_str.size(); i++) {
        if (env_str[i] == '\0') {
            env_str[i] = '\n';
        }
    }

    std::vector<std::string> envs;
    boost::split(envs, env_str, boost::is_any_of("\n"));

    for (size_t i = 0; i < envs.size(); i++) {
        if (boost::starts_with(envs[i], "BAIDU_GALAXY_CONTAINER_ID=")) {
            std::vector<std::string> vid;
            boost::split(vid, envs[i], boost::is_any_of("="));

            if (vid.size() == 2) {
                if (id_.SubId() == vid[1]) {
                    return true;
                }
            }
        }
    }

    return false;
}

boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> Container::ContainerInfo(bool full_info)
{
    boost::shared_ptr<baidu::galaxy::proto::ContainerInfo> ret(new baidu::galaxy::proto::ContainerInfo());
    ret->set_id(id_.SubId());
    ret->set_group_id(id_.GroupId());
    ret->set_created_time(0);
    ret->set_status(status_.Status());
    ret->set_cpu_used(0);
    ret->set_memory_used(0);

    baidu::galaxy::proto::ContainerDescription* cd = ret->mutable_container_desc();
    if (full_info) {
        cd->CopyFrom(desc_);
    } else {
        cd->set_version(desc_.version());
    }
    return ret;
}


boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> Container::ContainerMeta() {
    boost::shared_ptr<baidu::galaxy::proto::ContainerMeta> ret(new baidu::galaxy::proto::ContainerMeta());
    ret->set_container_id(id_.SubId());
    ret->set_group_id(id_.GroupId());
    ret->set_gc_index(created_time_);
    ret->mutable_container()->CopyFrom(desc_);
    return ret;
}

const baidu::galaxy::proto::ContainerDescription& Container::Description()
{
    return desc_;
}

} //namespace container
} //namespace galaxy
} //namespace baidu

