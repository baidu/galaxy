// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy_sdk_util.h"
#include "string_util.h"

#ifndef GALAXY_SDK_UTIL_H
#define GALAXY_SDK_UTIL_H

namespace baidu {
namespace galaxy {
namespace sdk {

bool FillUser(const User& sdk_user, ::baidu::galaxy::proto::User* user) {
    if (sdk_user.user.empty()) {
        fprintf(stderr, "user must not be empty\n");
        return false;
    }
    if (sdk_user.token.empty()) {
        fprintf(stderr, "token must not be empty\n");
        return false;
    }
    user->set_user(sdk_user.user);
    user->set_token(sdk_user.token);
    return true;
}

bool FillVolumRequired(const VolumRequired& sdk_volum, ::baidu::galaxy::proto::VolumRequired* volum) {
    if (sdk_volum.size <= 0) {
        fprintf(stderr, "volum size must be greater than 0\n");
        return false;
    }
    volum->set_size(sdk_volum.size);

    if (sdk_volum.type == kEmptyDir) {
        volum->set_type(::baidu::galaxy::proto::kEmptyDir);
    } else if (sdk_volum.type == kHostDir) {
        volum->set_type(::baidu::galaxy::proto::kHostDir);
    } else {
        fprintf(stderr, "volum type must be kEmptyDir or kHostDir\n");
        return false;
    }

    if (sdk_volum.medium == kSsd) {
        volum->set_medium(baidu::galaxy::proto::kSsd);
    } else if (sdk_volum.medium == kDisk) {
        volum->set_medium(::baidu::galaxy::proto::kDisk);
    } else if (sdk_volum.medium == kBfs) {
        volum->set_medium(baidu::galaxy::proto::kBfs);
    } else if (sdk_volum.medium == kTmpfs) {
        volum->set_medium(::baidu::galaxy::proto::kTmpfs);
    } else {
        fprintf(stderr, "volum medium must be kDisk, kBfs, kTmpfs\n");
        return false;
    }

    volum->set_source_path(sdk_volum.source_path); //不需要赋值

    if (sdk_volum.dest_path.empty()) {
        fprintf(stderr, "volum dest_path must not be empty\n");
        return false;
    }
    volum->set_dest_path(sdk_volum.dest_path);
    volum->set_readonly(sdk_volum.readonly);
    volum->set_exclusive(sdk_volum.exclusive);
    //volum->set_use_symlink(sdk_volum.use_symlink);//暂时不支持，默认为false
    volum->set_use_symlink(false);
    return true;
}

bool FillCpuRequired(const CpuRequired& sdk_cpu, 
                        ::baidu::galaxy::proto::CpuRequired* cpu) {
    if (sdk_cpu.milli_core <= 0) {
        fprintf(stderr, "cpu millicores must be greater than 0\n");
        return false;
    }
    cpu->set_milli_core(sdk_cpu.milli_core);
    cpu->set_excess(sdk_cpu.excess);
    return true;
}

bool FillMemRequired(const MemoryRequired& sdk_mem, 
                        ::baidu::galaxy::proto::MemoryRequired* mem) {
    if (sdk_mem.size <= 0) {
        fprintf(stderr, "memory size must be greater than 0\n");
        return false;
    }
    mem->set_size(sdk_mem.size);
    mem->set_excess(sdk_mem.excess);
    return true;
}

bool FillTcpthrotRequired(const TcpthrotRequired& sdk_tcp,
                        ::baidu::galaxy::proto::TcpthrotRequired* tcp) {
    if (sdk_tcp.recv_bps_quota <= 0) {
        fprintf(stderr, "tcp recv_bps_quota must be greater than 0\n");
        return false;
    }
    tcp->set_recv_bps_quota(sdk_tcp.recv_bps_quota);
    tcp->set_recv_bps_excess(sdk_tcp.recv_bps_excess);

    if (sdk_tcp.send_bps_quota <= 0) {
        fprintf(stderr, "tcp send_bps_quota must be greater than 0\n");
        return false;
    }
    tcp->set_send_bps_quota(sdk_tcp.send_bps_quota);
    tcp->set_send_bps_excess(sdk_tcp.send_bps_excess);
    return true;
}

bool FillBlkioRequired(const BlkioRequired& sdk_blk,
                        ::baidu::galaxy::proto::BlkioRequired* blk) {
    if (sdk_blk.weight <= 0 || sdk_blk.weight >= 1000) {
        fprintf(stderr, "blkio weight must be in 0~1000\n");
        return false;
    }
    blk->set_weight(sdk_blk.weight);
    return true;
}

bool FillPortRequired(const PortRequired& sdk_port,
                        ::baidu::galaxy::proto::PortRequired* port) {
    if (sdk_port.port_name.empty()) {
        fprintf(stderr, "port_name must not be empty in port\n");
        return false;
    }
    port->set_port_name(sdk_port.port_name);
    if (sdk_port.port.empty()) {
        fprintf(stderr, "port must not be empty in port, it could be \"dynamic\"or specific port such as \"8080\" \n");
        return false;
    }
    port->set_port(sdk_port.port);

    port->set_real_port(sdk_port.real_port);//可以不用，有resman分配
    return true;
}

bool FillCgroup(const Cgroup& sdk_cgroup, 
                        ::baidu::galaxy::proto::Cgroup* cgroup) {
    
    if (!FillCpuRequired(sdk_cgroup.cpu, cgroup->mutable_cpu())) {
        return false;
    }

    if (!FillMemRequired(sdk_cgroup.memory, cgroup->mutable_memory())) {
        return false;
    }
    
    if (!FillTcpthrotRequired(sdk_cgroup.tcp_throt, cgroup->mutable_tcp_throt())) {
        return false;
    }
    
    if (!FillBlkioRequired(sdk_cgroup.blkio, cgroup->mutable_blkio())) {
        return false;
    }

    bool ok = true;
    for (size_t i = 0; i < sdk_cgroup.ports.size(); ++i) {
        ::baidu::galaxy::proto::PortRequired* port = cgroup->add_ports();
        if (!FillPortRequired(sdk_cgroup.ports[i], port)) {
            ok = false;
            break;
        }
    }
    return ok;
}

bool FillContainerDescription(const ContainerDescription& sdk_container, 
                                ::baidu::galaxy::proto::ContainerDescription* container) {

    if (sdk_container.priority == kJobMonitor) {
        container->set_priority(::baidu::galaxy::proto::kJobMonitor);
    } else if (sdk_container.priority == kJobService) {
        container->set_priority(::baidu::galaxy::proto::kJobService);
    } else if (sdk_container.priority == kJobBatch) {
        container->set_priority(::baidu::galaxy::proto::kJobBatch);
    } else if (sdk_container.priority == kJobBestEffort) {
        container->set_priority(::baidu::galaxy::proto::kJobBestEffort);
    } else {
        fprintf(stderr, "job type must be kJobService, kJobBatch, kJobBestEffort\n");
        return false;
    }

    if (sdk_container.run_user.empty()) {
        fprintf(stderr, "run_user must no be empty\n");
        return false;
    }
    container->set_run_user(sdk_container.run_user);
    if (sdk_container.version.empty()) {
        fprintf(stderr, "version must no be empty\n");
        return false;
    }
    container->set_version(sdk_container.version);

    if (sdk_container.tag.empty()) {
        fprintf(stderr, "tag must no be empty\n");
        return false;
    }
    container->set_tag(sdk_container.tag);

    container->set_cmd_line(sdk_container.cmd_line); //不用

    if (sdk_container.max_per_host <= 0) {
        fprintf(stderr, "max_per_host must be greater than 0\n");
        return false;
    }
    container->set_max_per_host(sdk_container.max_per_host);

    bool ok = true;
    for (size_t i = 0; i < sdk_container.pool_names.size(); ++i) {
        if (sdk_container.pool_names[i].empty()) {
            fprintf(stderr, "pool must no be empty\n");
            ok = false;
            break;
        }
        container->add_pool_names(sdk_container.pool_names[i]);
    }
    if (!ok) {
        return false;
    }

    if (!FillVolumRequired(sdk_container.workspace_volum, container->mutable_workspace_volum())) {
        return false;
    }

    for (size_t i = 0; i < sdk_container.data_volums.size(); ++i) {
        ::baidu::galaxy::proto::VolumRequired* volum = container->add_data_volums();
        if(!FillVolumRequired(sdk_container.data_volums[i], volum)) {
            ok = false;
            break;
        }
    }
    if (!ok) {
        return false;
    }

    for (uint32_t i = 0; i < sdk_container.cgroups.size(); ++i) {
        ::baidu::galaxy::proto::Cgroup* cgroup = container->add_cgroups();
        if(!FillCgroup(sdk_container.cgroups[i], cgroup)) {
            ok = false;
            break;
        }
        cgroup->set_id(::baidu::common::NumToString(i));
    }
    
    return ok;
}

bool FillPackage(const Package& sdk_package, ::baidu::galaxy::proto::Package* package) {
    if (sdk_package.source_path.empty()) {
        fprintf(stderr, "package source_path must no be empty\n");
        return false;
    }
    package->set_source_path(sdk_package.source_path);
    if (sdk_package.dest_path.empty()) {
        fprintf(stderr, "package dest_path must no be empty\n");
        return false;
    }
    package->set_dest_path(sdk_package.dest_path);
    if (sdk_package.version.empty()) {
        fprintf(stderr, "package version must no be empty\n");
        return false;
    }
    package->set_version(sdk_package.version);
    return true;
}

bool FillImagePackage(const ImagePackage& sdk_image, 
                        ::baidu::galaxy::proto::ImagePackage* image) {
    if (sdk_image.start_cmd.empty()) {
        fprintf(stderr, "package start_cmd must no be empty\n");
        return false;
    }
    image->set_start_cmd(sdk_image.start_cmd);
    image->set_stop_cmd(sdk_image.stop_cmd);
    if (!FillPackage(sdk_image.package, image->mutable_package())) {
        return false;
    }
    return true;
}

bool FilldataPackage(const DataPackage& sdk_data, ::baidu::galaxy::proto::DataPackage* data) {
    bool ok = true;
    data->set_reload_cmd(sdk_data.reload_cmd);
    for (size_t i = 0; i < sdk_data.packages.size(); ++i) {
        ::baidu::galaxy::proto::Package* package = data->add_packages();
        ok = FillPackage(sdk_data.packages[i], package);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool FillService(const Service& sdk_service, ::baidu::galaxy::proto::Service* service) {
    if (sdk_service.service_name.empty()) {
        fprintf(stderr, "service service_name must no be empty\n");
        return false;
    }
    service->set_service_name(sdk_service.service_name);
    
    if (sdk_service.port_name.empty()) {
        fprintf(stderr, "service port_name must no be empty\n");
        return false;
    }
    service->set_port_name(sdk_service.port_name);
    service->set_use_bns(sdk_service.use_bns);
    return true;
}

bool FillTaskDescription(const TaskDescription& sdk_task,
        ::baidu::galaxy::proto::TaskDescription* task) {
    
    bool ok = true;
    if (!FillCpuRequired(sdk_task.cpu, task->mutable_cpu())) {
        return false;
    }

    if (!FillMemRequired(sdk_task.memory, task->mutable_memory())) {
        return false;
    }

    for (size_t i = 0; i < sdk_task.ports.size(); ++i) {
        ::baidu::galaxy::proto::PortRequired* port = task->add_ports();
        ok = FillPortRequired(sdk_task.ports[i], port);
        if (!ok) {
            break;
        }
    }

    if (!ok) {
        return false;
    }

    if (!FillTcpthrotRequired(sdk_task.tcp_throt, task->mutable_tcp_throt())) {
        return false;
    }

    if (!FillBlkioRequired(sdk_task.blkio, task->mutable_blkio())) {
        return false;
    }

    if (!FillImagePackage(sdk_task.exe_package, task->mutable_exe_package())) {
        return false;
    }
    
    if (!FilldataPackage(sdk_task.data_package, task->mutable_data_package())) {
        return false;
    }

    for (size_t i = 0; i < sdk_task.services.size(); ++i) {
        ::baidu::galaxy::proto::Service* service = task->add_services();
        ok = FillService(sdk_task.services[i], service);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool FillPodDescription(const PodDescription& sdk_pod,
                            ::baidu::galaxy::proto::PodDescription* pod) {
    if (!FillVolumRequired(sdk_pod.workspace_volum, pod->mutable_workspace_volum())) {
        return false;
    }
    
    bool ok = true;
    for (size_t i = 0; i < sdk_pod.data_volums.size(); ++i) {
        ::baidu::galaxy::proto::VolumRequired* volum = pod->add_data_volums();
        ok = FillVolumRequired(sdk_pod.data_volums[i], volum);
        if (!ok) {
            break;
        }
    }
    if (!ok) {
        return false;
    }

    for (uint32_t i = 0; i < sdk_pod.tasks.size(); ++i) {
        ::baidu::galaxy::proto::TaskDescription* task = pod->add_tasks(); 
        ok = FillTaskDescription(sdk_pod.tasks[i], task);
        if (!ok) {
            break;
        }
        task->set_id(::baidu::common::NumToString(i));
    }
    return ok;
}

bool FillDeploy(const Deploy& sdk_deploy, ::baidu::galaxy::proto::Deploy* deploy) {
    if (sdk_deploy.replica <= 0 || sdk_deploy.replica >= 10000) {
        fprintf(stderr, "deploy replica must be greater than 0 and less than 10000\n");
        return false;
    }
    deploy->set_replica(sdk_deploy.replica);

    if (sdk_deploy.step <= 0 || sdk_deploy.step > sdk_deploy.replica) {
        fprintf(stderr, "deploy step must be greater than 0 and less than replica\n");
        return false;
    }
    deploy->set_step(sdk_deploy.step);

    if (sdk_deploy.interval < 0 || sdk_deploy.interval > 3600) {
        fprintf(stderr, "deploy interval must be greater than or equal to 0 and less than 3600\n");
        return false;
    }
    deploy->set_interval(sdk_deploy.interval);

    if (sdk_deploy.max_per_host <= 0 || sdk_deploy.max_per_host >= 30) {
        fprintf(stderr, "deploy max_per_host must be greater than 0 and less than 30\n");
        return false;
    }
    deploy->set_max_per_host(sdk_deploy.max_per_host);
    
    deploy->set_tag(sdk_deploy.tag);

    bool ok = true;
    for (size_t i = 0; i < sdk_deploy.pools.size(); ++i) {
        if (sdk_deploy.pools[i].empty()) {
            fprintf(stderr, "deploy pools must not be empty\n");
            ok = false;
            break;
        }
        deploy->add_pools(sdk_deploy.pools[i]);
    }
    return ok;
}

bool FillJobDescription(const JobDescription& sdk_job,
                        ::baidu::galaxy::proto::JobDescription* job) {
    if (sdk_job.name.empty()) {
        fprintf(stderr, "job name must not be empty\n");
        return false;
    }
    job->set_name(sdk_job.name);

    //job->set_version(sdk_job.version); //不再需要

    if (sdk_job.type == kJobMonitor) {
        job->set_priority(::baidu::galaxy::proto::kJobMonitor);
    } else if (sdk_job.type == kJobService) {
        job->set_priority(::baidu::galaxy::proto::kJobService);
    } else if (sdk_job.type == kJobBatch) {
        job->set_priority(::baidu::galaxy::proto::kJobBatch);
    } else if (sdk_job.type == kJobBestEffort) {
        job->set_priority(::baidu::galaxy::proto::kJobBestEffort);
    } else {
        fprintf(stderr, "job type must be kJobService, kJobBatch, kJobBestEffort\n");
        return false;
    }
    if (!FillDeploy(sdk_job.deploy, job->mutable_deploy())) {
        return false;
    }
    //job->set_run_user(sdk_job.run_user);
    job->set_run_user("galaxy");
    if (!FillPodDescription(sdk_job.pod, job->mutable_pod())) {
        return false;
    }
    return true;
}

bool FillGrant(const Grant& sdk_grant, ::baidu::galaxy::proto::Grant* grant) {
    if (sdk_grant.pool.empty()) {
        fprintf(stderr, "pool must not be empty\n");
        return false;
    } 
    grant->set_pool(sdk_grant.pool);

    if (sdk_grant.action == kActionAdd) {
        grant->set_action(::baidu::galaxy::proto::kActionAdd);
    } else if (sdk_grant.action == kActionRemove) {
        grant->set_action(::baidu::galaxy::proto::kActionRemove);
    } else if (sdk_grant.action == kActionSet) {
        grant->set_action(::baidu::galaxy::proto::kActionSet);
    } else if (sdk_grant.action == kActionClear) {
        grant->set_action(::baidu::galaxy::proto::kActionClear);
    } else {
        fprintf(stderr, "action must be kActionAdd, kActionRemove, kActionSet or kActionClear\n");
        return false;
    }

    bool ok = true;
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
        default:
            ok = false;
        }
        if (!ok) {
            break;
        }
    }
    return ok;
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
        task.tcp_throt.recv_bps_quota = pb_job.pod().tasks(i).tcp_throt().recv_bps_quota();
        task.tcp_throt.recv_bps_excess = pb_job.pod().tasks(i).tcp_throt().recv_bps_excess();
        task.tcp_throt.send_bps_quota = pb_job.pod().tasks(i).tcp_throt().send_bps_quota();
        task.tcp_throt.send_bps_excess = pb_job.pod().tasks(i).tcp_throt().send_bps_excess();
        task.blkio.weight = pb_job.pod().tasks(i).blkio().weight();
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
