// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "master_util.h"

#include <sstream>
#include <sys/utsname.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gflags/gflags.h>
#include <logging.h>

#include "proto/galaxy.pb.h"
#include "proto/master.pb.h"

DECLARE_string(master_port);

namespace baidu {
namespace galaxy {

std::string MasterUtil::GenerateJobId(const JobDescriptor& job_desc) {
    return "job_" + job_desc.name() + "_" + UUID();
}

std::string MasterUtil::GeneratePodId(const JobDescriptor& job_desc) {
    return "pod_" + job_desc.name() + "_" + UUID();
}

std::string MasterUtil::UUID() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::stringstream sm_uuid;
    sm_uuid << uuid;
    std::string str_uuid= sm_uuid.str();
    return str_uuid;
}


void MasterUtil::AddResource(const Resource& from, Resource* to) {
    to->set_millicores(to->millicores() + from.millicores());
    to->set_memory(to->memory() + from.memory());
}

void MasterUtil::SubstractResource(const Resource& from, Resource* to) {
    assert(FitResource(from, *to));
    to->set_millicores(to->millicores() - from.millicores());
    to->set_memory(to->memory() - from.memory());
}

bool MasterUtil::FitResource(const Resource& from, const Resource& to) {
    if (to.millicores() < from.millicores()) {
        return false;
    }
    if (to.memory() < from.memory()) {
        return false;
    }
    // TODO: check port & disk & ssd
    return true;
}

std::string MasterUtil::SelfEndpoint() {
    std::string hostname = "";
    struct utsname buf;
    if (0 != uname(&buf)) {
        *buf.nodename = '\0';
    }
    hostname = buf.nodename;
    return hostname + ":" + FLAGS_master_port;
}

void MasterUtil::TraceJobDesc(const JobDescriptor& job_desc) {
    LOG(INFO, "job descriptor: \"%s\", "
              "replica:%d, "
              "deploy_step:%d, version:\"%s\"",
              job_desc.name().c_str(),
              job_desc.replica(),
              job_desc.deploy_step(),
              job_desc.version().c_str()
    );
    for (int i = 0; i < job_desc.pod().tasks_size(); i++) {
        const TaskDescriptor& task_desc = job_desc.pod().tasks(i);
        LOG(INFO, "job[%s]:task[%d] descriptor: "
            "start_cmd: \"%s\", stop_cmd: \"%s\", binary_size:%d",
            job_desc.name().c_str(),
            i, 
            task_desc.start_command().c_str(),
            task_desc.stop_command().c_str(),
            task_desc.binary().size()
        );
    }
}

}
}
