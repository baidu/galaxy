// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BAIDU_GALAXY_SDK_UTIL_H
#define BAIDU_GALAXY_SDK_UTIL_H

#include "protocol/appmaster.pb.h"
#include "protocol/resman.pb.h"
#include "protocol/galaxy.pb.h"
#include "ins_sdk.h"
#include "galaxy_sdk.h"

namespace baidu {
namespace galaxy {
namespace sdk {

void FillUser(const User& sdk_user, ::baidu::galaxy::proto::User* user);
void FillVolumRequired(const VolumRequired& sdk_volum, ::baidu::galaxy::proto::VolumRequired* volum);
void FillCpuRequired(const CpuRequired& sdk_cpu, ::baidu::galaxy::proto::CpuRequired* cpu);
void FillMemRequired(const MemoryRequired& sdk_mem, ::baidu::galaxy::proto::MemoryRequired* mem);
void FillTcpthrotRequired(const TcpthrotRequired& sdk_tcp, ::baidu::galaxy::proto::TcpthrotRequired* tcp);
void FillBlkioRequired(const BlkioRequired& sdk_blk, ::baidu::galaxy::proto::BlkioRequired* blk);
void FillPortRequired(const PortRequired& sdk_port, ::baidu::galaxy::proto::PortRequired* port);
void FillCgroup(const Cgroup& sdk_cgroup, ::baidu::galaxy::proto::Cgroup* cgroup);
void FillContainerDescription(const ContainerDescription& sdk_container,
                                ::baidu::galaxy::proto::ContainerDescription* container);

void FillGrant(const Grant& sdk_grant, ::baidu::galaxy::proto::Grant* grant);

void FillService(const Service& sdk_service, ::baidu::galaxy::proto::Service* service);
void FillPackage(const Package& sdk_package, ::baidu::galaxy::proto::Package package);
void FilldataPackage(const DataPackage& sdk_data, ::baidu::galaxy::proto::DataPackage* data);
void FillImagePackage(const ImagePackage& sdk_image, ::baidu::galaxy::proto::ImagePackage* image);
void FillTaskDescription(const TaskDescription& sdk_task, ::baidu::galaxy::proto::TaskDescription* task);
void FillPodDescription(const PodDescription& sdk_pod, ::baidu::galaxy::proto::PodDescription* pod);
void FillDeploy(const Deploy& sdk_deploy, ::baidu::galaxy::proto::Deploy* deploy);
void FillJobDescription(const JobDescription& sdk_job,
                        ::baidu::galaxy::proto::JobDescription* job);

void PbJobDescription2SdkJobDescription(const ::baidu::galaxy::proto::JobDescription& pb_job, JobDescription* job);

bool StatusSwitch(const ::baidu::galaxy::proto::Status& pb_status, ::baidu::galaxy::sdk::Status* status);
bool VolumTypeSwitch(const baidu::galaxy::proto::VolumType& pb_type, ::baidu::galaxy::sdk::VolumType* type);
bool ContainerStatusSwitch(const ::baidu::galaxy::proto::ContainerStatus& pb_status, 
                                 ::baidu::galaxy::sdk::ContainerStatus* status); 
bool AgentStatusSwitch(const ::baidu::galaxy::proto::AgentStatus& pb_status, ::baidu::galaxy::sdk::AgentStatus* status);
bool VolumMediumSwitch(const ::baidu::galaxy::proto::VolumMedium& pb_medium, ::baidu::galaxy::sdk::VolumMedium* medium);
bool AuthoritySwitch(const ::baidu::galaxy::proto::Authority& pb_authority, ::baidu::galaxy::sdk::Authority* authority);

bool JobStatusSwitch(const ::baidu::galaxy::proto::JobStatus& pb_status, ::baidu::galaxy::sdk::JobStatus* status);
bool PodStatusSwitch(const ::baidu::galaxy::proto::PodStatus& pb_status, ::baidu::galaxy::sdk::PodStatus* status);

} // end namespace sdk
} // end namespace galaxy
} // end namespace baidu

#endif  // BAIDU_GALAXY_SDK_UTIL_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
