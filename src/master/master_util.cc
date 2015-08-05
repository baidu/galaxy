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
#include "proto/galaxy.pb.h"
#include "proto/master.pb.h"

DECLARE_string(master_port);

namespace baidu {
namespace galaxy {

static bool ComparePort(int32_t l, int32_t r) {
    return l > r;
}

static bool CompareVolume(const Volume& l, const Volume& r) {
    return l.quota() > r.quota();
}


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

bool MasterUtil::CompareResource(const Resource& r_old,
                                 const Resource& r_new) {
    if (r_old.millicores() != 
        r_new.millicores()) {
        return false;
    }

    if (r_old.memory() != 
        r_new.memory()) {
        return false;
    }

    if (r_old.ports_size() != r_new.ports_size()) {
        return false;
    }

    std::vector<int32_t> r_old_ports;
    std::vector<int32_t> r_new_ports;
    for (int i = 0; i < r_old.ports_size(); ++i) {
        r_old_ports.push_back(r_old.ports(i));
        r_new_ports.push_back(r_new.ports(i));
    }

    bool has_same_ports = MasterUtil::CompareVector<int32_t>(r_old_ports, r_new_ports, ComparePort);
    if (!has_same_ports) {
        return false;
    }

    if (r_old.disks_size() != r_new.disks_size()) {
        return false;
    }

    std::vector<Volume> r_old_volumes;
    std::vector<Volume> r_new_volumes;
    for (int i = 0; i < r_old.disks_size(); i++) {
        r_old_volumes.push_back(r_old.disks(i));
        r_new_volumes.push_back(r_new.disks(i));
    }

    bool has_same_disks = MasterUtil::CompareVector<Volume>(r_old_volumes,
      r_new_volumes,
      CompareVolume);
    if (!has_same_disks) {
        return false;
    }
    
    if (r_old.ssds_size() != r_new.ssds_size()) {
        return false;
    }
    r_old_volumes.clear();
    r_new_volumes.clear();
    for (int i = 0; i < r_old.ssds_size(); i++) {
        r_old_volumes.push_back(r_old.ssds(i));
        r_new_volumes.push_back(r_new.ssds(i));
    }
    bool has_same_ssds = MasterUtil::CompareVector<Volume>(r_old_volumes,
      r_new_volumes,
      CompareVolume);
    if (!has_same_ssds) {
        return false;
    }
    return true;
}


}
}
