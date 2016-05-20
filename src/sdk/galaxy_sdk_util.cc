// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ins_sdk.h"
#include "galaxy_sdk_util.h"

#ifndef GALAXY_SDK_UTIL_H
#define GALAXY_SDK_UTIL_H

namespace baidu {
namespace galaxy {
namespace sdk {

bool StatusSwitch(const ::baidu::galaxy::proto::Status& pb_status, Status* status) {
    switch(pb_status) {
    case ::baidu::galaxy::proto::kOk:
        *status = kOk;
        break;
    case ::baidu::galaxy::proto::kError:
        *status = kError;
        break;
    case ::baidu::galaxy::proto::kTerminate:
        *status = kTerminate;
        break;
    case ::baidu::galaxy::proto::kAddAgentFail:
        *status = kAddAgentFail;
        break;
    case ::baidu::galaxy::proto::kDeny:
        *status = kDeny;
        break;
    case ::baidu::galaxy::proto::kJobNotFound:
        *status = kOk;
        break;
    case ::baidu::galaxy::proto::kCreateContainerGroupFail:
        *status = kCreateContainerGroupFail;
        break;
    case ::baidu::galaxy::proto::kRemoveContainerGroupFail:
        *status = kRemoveContainerGroupFail;
        break;
    case ::baidu::galaxy::proto::kUpdateContainerGroupFail:
        *status = kUpdateContainerGroupFail;
        break;
    case ::baidu::galaxy::proto::kRemoveAgentFail:
        *status = kRemoveAgentFail;
        break;
    case ::baidu::galaxy::proto::kCreateTagFail:
        *status = kCreateTagFail;
        break;
    case ::baidu::galaxy::proto::kAddAgentToPoolFail:
        *status = kAddAgentToPoolFail;
        break;
    default:
        return false;
    }
    return true;
}

bool AgentStatusSwitch(const ::baidu::galaxy::proto::AgentStatus& pb_status, baidu::galaxy::sdk::AgentStatus* status) {
    switch(pb_status) {
    case ::baidu::galaxy::proto::kAgentUnkown:
        *status = kAgentUnkown;
        break;
    case ::baidu::galaxy::proto::kAgentAlive:
        *status = kAgentAlive;
        break;
    case ::baidu::galaxy::proto::kAgentDead:
        *status = kAgentDead;
        break;
    case ::baidu::galaxy::proto::kAgentOffline:
        *status = kAgentOffline;
        break;
    default:
        return false;
    }
    return true;
}

bool VolumTypeSwitch(const ::baidu::galaxy::proto::VolumType& pb_type, baidu::galaxy::sdk::VolumType* type) {
    switch(pb_type) {
    case ::baidu::galaxy::proto::kEmptyDir:
        *type = kEmptyDir;
        break;
    case ::baidu::galaxy::proto::kHostDir:
        *type = kHostDir;
        break;
    default:
        return false;
    }
    return true;
}

bool VolumMediumSwitch(const ::baidu::galaxy::proto::VolumMedium& pb_medium, baidu::galaxy::sdk::VolumMedium* medium) {
    switch(pb_medium) {
    case ::baidu::galaxy::proto::kSsd:
        *medium = kSsd;
        break;
    case ::baidu::galaxy::proto::kDisk:
        *medium = kDisk;
        break;
    case ::baidu::galaxy::proto::kBfs:
        *medium = kBfs;
        break;
    case ::baidu::galaxy::proto::kTmpfs:
        *medium = kTmpfs;
        break;
    default:
        return false;
    }
    return true;
}

bool ContainerStatusSwitch(const ::baidu::galaxy::proto::ContainerStatus& pb_status, 
                                baidu::galaxy::sdk::ContainerStatus* status) {
    switch(pb_status) {
    case ::baidu::galaxy::proto::kContainerPending:
        *status = kContainerPending;
        break;
    case ::baidu::galaxy::proto::kContainerAllocating:
        *status = kContainerAllocating;
        break;
    case ::baidu::galaxy::proto::kContainerReady:
        *status = kContainerReady;
        break;
    case ::baidu::galaxy::proto::kContainerFinish:
        *status = kContainerFinish;
        break;
    case ::baidu::galaxy::proto::kContainerError:
        *status = kContainerError;
        break;
    case ::baidu::galaxy::proto::kContainerDestroying:
       *status = kContainerDestroying;
        break;
    case ::baidu::galaxy::proto::kContainerTerminated:
        *status = kContainerTerminated;
        break;
    default:
        return false;
    }
    return true;
}

bool AuthoritySwitch(const ::baidu::galaxy::proto::Authority& pb_authority, ::baidu::galaxy::sdk::Authority* authority) {
    switch(pb_authority) {
    case ::baidu::galaxy::proto::kAuthorityCreateContainer:
        *authority = kAuthorityCreateContainer;
        break;
    case ::baidu::galaxy::proto::kAuthorityRemoveContainer:
        *authority = kAuthorityRemoveContainer;
        break;
    case ::baidu::galaxy::proto::kAuthorityUpdateContainer:
        *authority = kAuthorityUpdateContainer;
        break;
    case ::baidu::galaxy::proto::kAuthorityListContainer:
        *authority = kAuthorityListContainer;
        break;
    case ::baidu::galaxy::proto::kAuthoritySubmitJob:
        *authority = kAuthoritySubmitJob;
        break;
    case ::baidu::galaxy::proto::kAuthorityUpdateJob:
        *authority = kAuthorityUpdateJob;
        break;
    case ::baidu::galaxy::proto::kAuthorityListJobs:
        *authority = kAuthorityListJobs;
        break;
    default:
        return false;
    }
    return true;
}

void FillUser(const User& sdk_user, ::baidu::galaxy::proto::User* user) {
    user->set_user(sdk_user.user);
    user->set_token(sdk_user.token);
}

void FillVolumRequired(const VolumRequired& sdk_volum, ::baidu::galaxy::proto::VolumRequired* volum) {
    volum->set_size(sdk_volum.size);

    if (sdk_volum.type == kEmptyDir) {
        volum->set_type(::baidu::galaxy::proto::kEmptyDir);
    } else if (sdk_volum.type == kHostDir) {
        volum->set_type(::baidu::galaxy::proto::kHostDir);
    }

    //::baidu::galaxy::proto::VolumMedium medium;
    if (sdk_volum.medium == kSsd) {
        volum->set_medium(baidu::galaxy::proto::kSsd);
    } else if (sdk_volum.medium == kDisk) {
        volum->set_medium(::baidu::galaxy::proto::kDisk);
    } else if (sdk_volum.medium == kBfs) {
        volum->set_medium(baidu::galaxy::proto::kBfs);
    } else if (sdk_volum.medium == kTmpfs) {
        volum->set_medium(::baidu::galaxy::proto::kTmpfs);
    }

    volum->set_source_path(sdk_volum.source_path);
    volum->set_dest_path(sdk_volum.dest_path);
    volum->set_readonly(sdk_volum.readonly);
    volum->set_exclusive(sdk_volum.exclusive);
    volum->set_use_symlink(sdk_volum.use_symlink);

}

void FillCpuRequired(const CpuRequired& sdk_cpu, 
                        ::baidu::galaxy::proto::CpuRequired* cpu) {
    cpu->set_milli_core(sdk_cpu.milli_core);
    cpu->set_excess(sdk_cpu.excess);
}
void FillMemRequired(const MemoryRequired& sdk_mem, 
                        ::baidu::galaxy::proto::MemoryRequired* mem) {
    mem->set_size(sdk_mem.size);
    mem->set_excess(sdk_mem.excess);
}
void FillTcpthrotRequired(const TcpthrotRequired& sdk_tcp,
                        ::baidu::galaxy::proto::TcpthrotRequired* tcp) {
    tcp->set_recv_bps_quota(sdk_tcp.recv_bps_quota);
    tcp->set_recv_bps_excess(sdk_tcp.recv_bps_excess);
    tcp->set_send_bps_quota(sdk_tcp.send_bps_quota);
    tcp->set_send_bps_excess(sdk_tcp.send_bps_excess);
}
void FillBlkioRequired(const BlkioRequired& sdk_blk,
                        ::baidu::galaxy::proto::BlkioRequired* blk) {
    blk->set_weight(sdk_blk.weight);
}

void FillPortRequired(const PortRequired& sdk_port,
                        ::baidu::galaxy::proto::PortRequired* port) {
    port->set_port_name(sdk_port.port_name);
    port->set_port(sdk_port.port);
    port->set_real_port(sdk_port.real_port);
}

void FillCgroup(const Cgroup& sdk_cgroup, 
                        ::baidu::galaxy::proto::Cgroup* cgroup) {
    cgroup->set_id(sdk_cgroup.id);
    FillCpuRequired(sdk_cgroup.cpu, cgroup->mutable_cpu());
    FillMemRequired(sdk_cgroup.memory, cgroup->mutable_memory());
    FillTcpthrotRequired(sdk_cgroup.tcp_throt, cgroup->mutable_tcp_throt());
    FillBlkioRequired(sdk_cgroup.blkio, cgroup->mutable_blkio());
    for (size_t i = 0; i < sdk_cgroup.ports.size(); ++i) {
        ::baidu::galaxy::proto::PortRequired* port = cgroup->add_ports();
        FillPortRequired(sdk_cgroup.ports[i], port);
    }
}

void FillContainerDescription(const ContainerDescription& sdk_container, 
                                ::baidu::galaxy::proto::ContainerDescription* container) {

    container->set_priority(sdk_container.priority);
    container->set_run_user(sdk_container.run_user);
    container->set_version(sdk_container.version);
    container->set_tag(sdk_container.tag);
    container->set_cmd_line(sdk_container.cmd_line);
    container->set_max_per_host(sdk_container.max_per_host);
    for (size_t i = 0; i < sdk_container.pool_names.size(); ++i) {
        container->add_pool_names(sdk_container.pool_names[i]);
    }
    FillVolumRequired(sdk_container.workspace_volum, container->mutable_workspace_volum());
    for (size_t i = 0; i < sdk_container.data_volums.size(); ++i) {
        ::baidu::galaxy::proto::VolumRequired* volum = container->add_data_volums();
        FillVolumRequired(sdk_container.data_volums[i], volum);
    }

    for (size_t i = 0; i < sdk_container.cgroups.size(); ++i) {
        ::baidu::galaxy::proto::Cgroup* cgroup = container->add_cgroups();
        FillCgroup(sdk_container.cgroups[i], cgroup);
    }
}

void FillGrant(const Grant& sdk_grant, ::baidu::galaxy::proto::Grant* grant) {
    
    grant->set_pool(sdk_grant.pool);

    if (sdk_grant.action == kActionAdd) {
        grant->set_action(::baidu::galaxy::proto::kActionAdd);
    } else if (sdk_grant.action == kActionRemove) {
        grant->set_action(::baidu::galaxy::proto::kActionRemove);
    } else if (sdk_grant.action == kActionSet) {
        grant->set_action(::baidu::galaxy::proto::kActionSet);
    } else if (sdk_grant.action == kActionClear) {
        grant->set_action(::baidu::galaxy::proto::kActionClear);
    }

    for (size_t i = 0; i < sdk_grant.authority.size(); ++i) {
        switch (sdk_grant.authority[i]) {
        case kAuthorityCreateContainer:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityCreateContainer);
            break;
        case kAuthorityRemoveContainer:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityRemoveContainer);
            break;
        case kAuthorityUpdateContainer:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityUpdateContainer);
            break;
        case kAuthorityListContainer:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityListContainer);
            break;
        case kAuthoritySubmitJob:
            grant->add_authority(::baidu::galaxy::proto::kAuthoritySubmitJob);
            break;
        case kAuthorityRemoveJob:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityRemoveJob);
            break;
        case kAuthorityUpdateJob:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityUpdateJob);
            break;
        case kAuthorityListJobs:
            grant->add_authority(::baidu::galaxy::proto::kAuthorityListJobs);
            break;
        }
    }
}

} // end namespace sdk
} // end namespace galaxy
} // end namespace baidu

#endif  // GALAXY_SDK_UTIL_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
