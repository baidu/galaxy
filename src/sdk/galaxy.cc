// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "galaxy.h"

#include <stdio.h>
#include "proto/master.pb.h"
#include "proto/galaxy.pb.h"
#include "rpc/rpc_client.h"

namespace baidu {
namespace galaxy {

class GalaxyImpl : public Galaxy {
public:
    GalaxyImpl(const std::string& master_addr)
      : master_(NULL) {
        rpc_client_ = new RpcClient();
        rpc_client_->GetStub(master_addr, &master_);
    }
    virtual ~GalaxyImpl() {}
    bool SubmitJob(const JobDescription& job, std::string* job_id);
    bool UpdateJob(const std::string& jobid, const JobDescription& job);
    bool ListJobs(std::vector<JobInformation>* jobs);
    bool ListAgents(std::vector<NodeDescription>* nodes);
    bool TerminateJob(const std::string& job_id);
    bool LabelAgents(const std::string& label, 
                     const std::vector<std::string>& agents);
private:
    bool FillJobDescriptor(const JobDescription& sdk_job, JobDescriptor* job);
private:
    RpcClient* rpc_client_;
    Master_Stub* master_;
};

bool GalaxyImpl::LabelAgents(const std::string& label, 
                             const std::vector<std::string>& agents) {
    LabelAgentRequest request; 
    LabelAgentResponse response;
    request.mutable_labels()->set_label(label);
    for (size_t i = 0; i < agents.size(); i++) {
        request.mutable_labels()->add_agents_endpoint(agents[i]);     
    }
    bool ret = rpc_client_->SendRequest(master_, &Master_Stub::LabelAgents,
                                        &request, &response, 5, 1);
    if (!ret || 
            (response.has_status() 
                    && response.status() != kOk)) {
        return false;     
    }    
    return true;
}

bool GalaxyImpl::TerminateJob(const std::string& job_id) {
    TerminateJobRequest request;
    TerminateJobResponse response;
    request.set_jobid(job_id);
    rpc_client_->SendRequest(master_, &Master_Stub::TerminateJob,
                             &request,&response,5,1);
    if (response.status() == kOk) {
        return true;
    }
    return false;
}

bool GalaxyImpl::FillJobDescriptor(const JobDescription& sdk_job, 
                                   JobDescriptor* job) {
    job->set_name(sdk_job.job_name);
    // job meta
    JobType job_type;
    bool ok = JobType_Parse(sdk_job.type, &job_type);
    if (!ok) {
        return false;
    }
    job->set_type(job_type);
    JobPriority priority;
    ok = JobPriority_Parse(sdk_job.priority, &priority);
    if (!ok) {
        return false;
    }
    job->set_deploy_step(sdk_job.deploy_step);
    job->set_replica(sdk_job.replica);

    // pod meta
    PodDescriptor* pod_pb = job->mutable_pod();
    Resource* pod_res = pod_pb->mutable_requirement();
    // pod res
    pod_res->set_millicores(sdk_job.pod.requirement.millicores);
    pod_res->set_memory(sdk_job.pod.requirement.memory);

    for (size_t i = 0; i < sdk_job.pod.tasks.size(); i++) {
        TaskDescriptor* task = pod_pb->add_tasks();
        task->set_binary(sdk_job.pod.tasks[i].binary);
        task->set_start_command(sdk_job.pod.tasks[i].start_cmd);
        task->set_stop_command(sdk_job.pod.tasks[i].stop_cmd);
        SourceType source_type;
        ok = SourceType_Parse(sdk_job.pod.tasks[i].source_type, &source_type);
        if (!ok) {
            return false;
        }
        task->set_source_type(source_type);
        Resource* task_res = task->mutable_requirement();
        const ResDescription& task_res_desc = sdk_job.pod.tasks[i].requirement;
        task_res->set_millicores(task_res_desc.millicores);
        task_res->set_memory(task_res_desc.memory);
        for (size_t j = 0; j < task_res_desc.ports.size(); j++) {
            task_res->add_ports(task_res_desc.ports[j]);
        }
        for (size_t j = 0; j < task_res_desc.disks.size(); j++) {
            Volume* disk = task_res->add_disks();
            disk->set_quota(task_res_desc.disks[j].quota);
            disk->set_path(task_res_desc.disks[j].path);
        }
        for (size_t j = 0; j < task_res_desc.ssds.size(); j++) {
            Volume* ssd = task_res->add_ssds();
            ssd->set_quota(task_res_desc.ssds[j].quota);
            ssd->set_path(task_res_desc.ssds[j].path);
        }
    }
    if (!sdk_job.label.empty()) {
        job->mutable_pod()->add_labels(sdk_job.label);
    }
    return true;
}

bool GalaxyImpl::SubmitJob(const JobDescription& job, std::string* job_id){
    if (job_id == NULL) {
        return false;
    }
    SubmitJobRequest request;
    SubmitJobResponse response;
    bool ok = FillJobDescriptor(job, request.mutable_job());
    if (!ok) {
        return false;
    }
    rpc_client_->SendRequest(master_, &Master_Stub::SubmitJob,
                             &request,&response,5,1);
    if (response.status() != kOk) {
        return false;
    }
    *job_id = response.jobid();
    return true;
}

bool GalaxyImpl::UpdateJob(const std::string& jobid, const JobDescription& job) {
    UpdateJobRequest request;
    UpdateJobResponse response;
    request.set_jobid(jobid);
    bool ok = FillJobDescriptor(job, request.mutable_job());
    if (!ok) {
        return false;
    }
    rpc_client_->SendRequest(master_, &Master_Stub::UpdateJob,
                             &request, &response, 5, 1);
    if (response.status() != kOk) {
        return false;
    }
    return true;
}

bool GalaxyImpl::ListJobs(std::vector<JobInformation>* jobs) {
    ListJobsRequest request;
    ListJobsResponse response;
    bool ret = rpc_client_->SendRequest(master_, &Master_Stub::ListJobs,
                             &request,&response,5,1);
    if (!ret || response.status() != kOk) {
        return false;
    }
    int job_num = response.jobs_size();
    for(int i = 0; i< job_num;i++){
        const JobOverview& job = response.jobs(i);
        JobInformation job_info ;
        job_info.job_id = job.jobid();
        job_info.job_name = job.desc().name();
        job_info.replica = job.desc().replica();
        job_info.priority = job.desc().priority();
        job_info.running_num = job.running_num();
        job_info.pending_num = job.pending_num();
        job_info.deploying_num = job.deploying_num();
        job_info.cpu_used = job.resource_used().millicores();
        job_info.mem_used = job.resource_used().memory();
				job_info.is_batch = (job.desc().type() == kBatch);
        job_info.state = JobState_Name(job.state());
        jobs->push_back(job_info);
    }
    return true;
}

bool GalaxyImpl::ListAgents(std::vector<NodeDescription>* nodes) {
    ListAgentsRequest request;
    ListAgentsResponse response;
    rpc_client_->SendRequest(master_, &Master_Stub::ListAgents,
                             &request,&response, 5, 1);
    int node_num = response.agents_size();
    for (int i = 0; i < node_num; i++) {
        const AgentInfo& node = response.agents(i);
        NodeDescription node_desc;
        node_desc.addr = node.endpoint();
        node_desc.task_num = node.pods_size();
        node_desc.cpu_share = node.total().millicores();
        node_desc.mem_share = node.total().memory();
        node_desc.cpu_assigned = node.assigned().millicores();
        node_desc.mem_assigned = node.assigned().memory();
        node_desc.cpu_used = node.used().millicores();
        node_desc.mem_used = node.used().memory();
        std::string labels;
        for (int label_ind = 0; label_ind < node.tags_size(); label_ind++) {
            labels.append(node.tags(label_ind));     
            if (label_ind != node.tags_size() - 1) {
                labels.append(",");
            }
        }
        node_desc.labels = labels;
        node_desc.state = AgentState_Name(node.state());
        nodes->push_back(node_desc);
    }
    return true;

}

Galaxy* Galaxy::ConnectGalaxy(const std::string& master_addr) {
    return new GalaxyImpl(master_addr);
}

} // namespace galaxy
} // namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
