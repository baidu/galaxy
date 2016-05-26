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

void FillPackage(const Package& sdk_package, ::baidu::galaxy::proto::Package* package) {
    package->set_source_path(sdk_package.source_path);
    package->set_dest_path(sdk_package.dest_path);
    package->set_version(sdk_package.version);
}

void FillImagePackage(const ImagePackage& sdk_image, 
                        ::baidu::galaxy::proto::ImagePackage* image) {
    image->set_start_cmd(sdk_image.start_cmd);
    image->set_stop_cmd(sdk_image.stop_cmd);
    FillPackage(sdk_image.package, image->mutable_package());
}

void FilldataPackage(const DataPackage& sdk_data, ::baidu::galaxy::proto::DataPackage* data) {
    data->set_reload_cmd(sdk_data.reload_cmd);
    for (size_t i = 0; i < sdk_data.packages.size(); ++i) {
        ::baidu::galaxy::proto::Package* package = data->add_packages();
        FillPackage(sdk_data.packages[i], package);
    }
}

void FillService(const Service& sdk_service, ::baidu::galaxy::proto::Service* service) {
    service->set_service_name(sdk_service.service_name);
    service->set_port_name(sdk_service.port_name);
    service->set_use_bns(sdk_service.use_bns);
}

void FillTaskDescription(const TaskDescription& sdk_task,
        ::baidu::galaxy::proto::TaskDescription* task) {
    task->set_id(sdk_task.id);
    FillCpuRequired(sdk_task.cpu, task->mutable_cpu());
    FillMemRequired(sdk_task.memory, task->mutable_memory());
    for (size_t i = 0; i < sdk_task.ports.size(); ++i) {
        ::baidu::galaxy::proto::PortRequired* port = task->add_ports();
        FillPortRequired(sdk_task.ports[i], port);
    }
    FillImagePackage(sdk_task.exe_package, task->mutable_exe_package());
    FilldataPackage(sdk_task.data_package, task->mutable_data_package());
    for (size_t i = 0; i < sdk_task.services.size(); ++i) {
        ::baidu::galaxy::proto::Service* service = task->add_services();
        FillService(sdk_task.services[i], service);
    }
}

void FillPodDescription(const PodDescription& sdk_pod,
                            ::baidu::galaxy::proto::PodDescription* pod) {
    FillVolumRequired(sdk_pod.workspace_volum, pod->mutable_workspace_volum());
    for (size_t i = 0; i < sdk_pod.data_volums.size(); ++i) {
        ::baidu::galaxy::proto::VolumRequired* volum = pod->add_data_volums();
        FillVolumRequired(sdk_pod.data_volums[i], volum);
    }
    for (size_t i = 0; i < sdk_pod.tasks.size(); ++i) {
        ::baidu::galaxy::proto::TaskDescription* task = pod->add_tasks(); 
        FillTaskDescription(sdk_pod.tasks[i], task);
    }
}

void FillDeploy(const Deploy& sdk_deploy, ::baidu::galaxy::proto::Deploy* deploy) {
    deploy->set_replica(sdk_deploy.replica);
    deploy->set_step(sdk_deploy.step);
    deploy->set_interval(sdk_deploy.interval);
    deploy->set_max_per_host(sdk_deploy.max_per_host);
    deploy->set_tag(sdk_deploy.tag);
    for (size_t i = 0; i < sdk_deploy.pools.size(); ++i) {
        fprintf(stderr, "port is %s\n", sdk_deploy.pools[i].c_str());
        deploy->add_pools(sdk_deploy.pools[i]);
    }
}

void FillJobDescription(const JobDescription& sdk_job,
                        ::baidu::galaxy::proto::JobDescription* job) {
    job->set_name(sdk_job.name);
    job->set_version(sdk_job.version);
    if (sdk_job.type == kJobMonitor) {
        job->set_priority(::baidu::galaxy::proto::kJobMonitor);
    } else if (sdk_job.type == kJobService) {
        job->set_priority(::baidu::galaxy::proto::kJobService);
    } else if (sdk_job.type == kJobBatch) {
        job->set_priority(::baidu::galaxy::proto::kJobBatch);
    } else if (sdk_job.type == kJobBestEffort) {
        job->set_priority(::baidu::galaxy::proto::kJobBestEffort);
    }
    FillDeploy(sdk_job.deploy, job->mutable_deploy());
    job->set_run_user(sdk_job.run_user);
    FillPodDescription(sdk_job.pod, job->mutable_pod());

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

void PbJobDescription2SdkJobDescription(const ::baidu::galaxy::proto::JobDescription& pb_job, JobDescription* job) {
    job->name = pb_job.name();
    if (pb_job.priority() == ::baidu::galaxy::proto::kJobMonitor) {
        job->type = kJobMonitor;
    } else if (pb_job.priority() == ::baidu::galaxy::proto::kJobService) {
        job->type = kJobService;
    } else if (pb_job.priority() == ::baidu::galaxy::proto::kJobBatch) {
        job->type = kJobBatch;
    } else if (pb_job.priority() == ::baidu::galaxy::proto::kJobBestEffort) {
        job->type = kJobBestEffort;
    }
    job->version = pb_job.version();
    job->run_user = pb_job.run_user();
    job->deploy.replica = pb_job.deploy().replica();
    job->deploy.step = pb_job.deploy().step();
    job->deploy.interval = pb_job.deploy().interval();
    job->deploy.max_per_host = pb_job.deploy().max_per_host();
    job->deploy.tag = pb_job.deploy().tag();
    for (int i = 0; i < pb_job.deploy().pools().size(); ++i) {
        job->deploy.pools.push_back(pb_job.deploy().pools(i));
    }
    job->pod.workspace_volum.size = pb_job.pod().workspace_volum().size();
    job->pod.workspace_volum.type = (VolumType)pb_job.pod().workspace_volum().type();
    job->pod.workspace_volum.medium = (VolumMedium)pb_job.pod().workspace_volum().medium();
    job->pod.workspace_volum.source_path = pb_job.pod().workspace_volum().source_path();
    job->pod.workspace_volum.dest_path = pb_job.pod().workspace_volum().dest_path();
    job->pod.workspace_volum.readonly = pb_job.pod().workspace_volum().readonly();
    job->pod.workspace_volum.exclusive = pb_job.pod().workspace_volum().exclusive();
    job->pod.workspace_volum.use_symlink = pb_job.pod().workspace_volum().use_symlink();

    for (int i = 0; i < pb_job.pod().data_volums().size(); ++i) {
        VolumRequired volum;
        volum.size = pb_job.pod().data_volums(i).size();
        volum.type = (VolumType)pb_job.pod().data_volums(i).type();
        volum.medium = (VolumMedium)pb_job.pod().data_volums(i).medium();
        volum.source_path = pb_job.pod().data_volums(i).source_path();
        volum.dest_path = pb_job.pod().data_volums(i).dest_path();
        volum.readonly = pb_job.pod().data_volums(i).readonly();
        volum.exclusive = pb_job.pod().data_volums(i).exclusive();
        volum.use_symlink = pb_job.pod().data_volums(i).use_symlink();
        job->pod.data_volums.push_back(volum);
    }

    for (int i = 0; i < pb_job.pod().tasks().size(); ++i) {
        TaskDescription task;
        task.id = pb_job.pod().tasks(i).id();
        task.cpu.milli_core = pb_job.pod().tasks(i).cpu().milli_core();
        task.cpu.excess = pb_job.pod().tasks(i).cpu().excess();
        task.memory.size = pb_job.pod().tasks(i).memory().size();
        task.memory.excess = pb_job.pod().tasks(i).memory().excess();
        for (int j = 0; j < pb_job.pod().tasks(i).ports().size(); ++j) {
            PortRequired port;
            port.port_name = pb_job.pod().tasks(i).ports(j).port_name();
            port.port = pb_job.pod().tasks(i).ports(j).port();
            port.real_port = pb_job.pod().tasks(i).ports(j).real_port();
            task.ports.push_back(port);
        }
        task.exe_package.start_cmd = pb_job.pod().tasks(i).exe_package().start_cmd();
        task.exe_package.stop_cmd = pb_job.pod().tasks(i).exe_package().stop_cmd();
        task.exe_package.package.source_path = pb_job.pod().tasks(i).exe_package().package().source_path();
        task.exe_package.package.dest_path = pb_job.pod().tasks(i).exe_package().package().dest_path();
        task.exe_package.package.version = pb_job.pod().tasks(i).exe_package().package().version();

        task.data_package.reload_cmd = pb_job.pod().tasks(i).data_package().reload_cmd();
        for (int j = 0; j < pb_job.pod().tasks(i).data_package().packages().size(); ++j) {
            Package package;
            package.source_path = pb_job.pod().tasks(i).data_package().packages(j).source_path();
            package.dest_path = pb_job.pod().tasks(i).data_package().packages(j).dest_path();
            package.version = pb_job.pod().tasks(i).data_package().packages(j).version();
            task.data_package.packages.push_back(package);
        }
        for (int j = 0; j < pb_job.pod().tasks(i).services().size(); ++j) {
            Service service;
            service.service_name = pb_job.pod().tasks(i).services(j).service_name(); 
            service.port_name = pb_job.pod().tasks(i).services(j).port_name(); 
            service.use_bns = pb_job.pod().tasks(i).services(j).use_bns(); 
            task.services.push_back(service);
        }
        job->pod.tasks.push_back(task);
    }
}

} // end namespace sdk
} // end namespace galaxy
} // end namespace baidu

#endif  // GALAXY_SDK_UTIL_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
