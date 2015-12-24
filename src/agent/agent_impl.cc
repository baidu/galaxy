// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "agent/agent_impl.h"

#include "gflags/gflags.h"
#include <fstream>
#include "boost/bind.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/split.hpp"
#include "proto/master.pb.h"
#include "logging.h"
#include "agent/agent_internal_infos.h"
#include "agent/utils.h"

DECLARE_string(master_host);
DECLARE_string(master_port);
DECLARE_int32(agent_background_threads_num);
DECLARE_int32(agent_heartbeat_interval);
DECLARE_string(agent_ip);
DECLARE_string(agent_port);
DECLARE_string(agent_work_dir);

DECLARE_int32(agent_millicores_share);
DECLARE_int64(agent_mem_share);

DECLARE_string(agent_persistence_path);
DECLARE_int32(stat_check_period);
DECLARE_double(max_cpu_usage);
DECLARE_double(max_mem_usage);
DECLARE_double(max_disk_r_bps);
DECLARE_double(max_disk_w_bps);
DECLARE_double(max_disk_r_rate);
DECLARE_double(max_disk_w_rate);
DECLARE_double(max_disk_util);
DECLARE_double(max_net_in_bps);
DECLARE_double(max_net_out_bps);
DECLARE_double(max_net_in_pps);
DECLARE_double(max_net_out_pps);
DECLARE_double(max_intr_rate);
DECLARE_double(max_soft_intr_rate);
DECLARE_int32(agent_recover_threshold);

namespace baidu {
namespace galaxy {
    
static const uint64_t MIN_COLLECT_TIME = 4;

AgentImpl::AgentImpl() : 
    master_endpoint_(),
    lock_(),
    background_threads_(FLAGS_agent_background_threads_num),
    rpc_client_(NULL),
    endpoint_(),
    master_(NULL),
    resource_capacity_(),
    master_watcher_(NULL),
    mutex_master_endpoint_(),
    state_(kAlive),
    recover_threshold_(0) {
    rpc_client_ = new RpcClient();    
    endpoint_ = FLAGS_agent_ip;
    endpoint_.append(":");
    endpoint_.append(FLAGS_agent_port);
    master_watcher_ = new MasterWatcher();
}

AgentImpl::~AgentImpl() {
    background_threads_.Stop(false);
    if (rpc_client_ != NULL) {
        delete rpc_client_; 
        rpc_client_ = NULL;
    }
    delete master_watcher_;
}

void AgentImpl::Query(::google::protobuf::RpcController* /*cntl*/,
                      const ::baidu::galaxy::QueryRequest* /*req*/,
                      ::baidu::galaxy::QueryResponse* resp,
                      ::google::protobuf::Closure* done) {

    MutexLock scope_lock(&lock_);
    resp->set_status(kOk);
    AgentInfo agent_info; 
    agent_info.set_endpoint(endpoint_);
    agent_info.mutable_total()->set_millicores(
            FLAGS_agent_millicores_share);
    agent_info.mutable_total()->set_memory(
            FLAGS_agent_mem_share);
    agent_info.mutable_assigned()->set_millicores(
            FLAGS_agent_millicores_share - resource_capacity_.millicores); 
    agent_info.mutable_assigned()->set_memory(
            FLAGS_agent_mem_share - resource_capacity_.memory);

    boost::unordered_set<int32_t>::iterator port_it = resource_capacity_.used_port.begin();
    for (; port_it != resource_capacity_.used_port.end(); ++port_it) {
        agent_info.mutable_assigned()->add_ports(*port_it);
    } 
    
    int32_t millicores = 0;
    int64_t memory_used = 0;
    std::vector<PodInfo> pods;
    pod_manager_.ShowPods(&pods);
    LOG(INFO, "query pods size %u", pods.size());
    std::vector<PodInfo>::iterator it = pods.begin();
    for (; it != pods.end(); ++it) {
        PodStatus* pod_status = agent_info.add_pods();         
        pod_status->CopyFrom(it->pod_status);
        millicores += pod_status->resource_used().millicores();
        memory_used += pod_status->resource_used().memory();
        LOG(DEBUG, "query pod %s job %s state %s", 
                pod_status->podid().c_str(), 
                pod_status->jobid().c_str(),
                PodState_Name(pod_status->state()).c_str());
    }
    agent_info.mutable_used()->set_millicores(millicores);
    agent_info.mutable_used()->set_memory(memory_used);
    resp->mutable_agent()->CopyFrom(agent_info);
    done->Run();
    return;
}

void AgentImpl::CreatePodInfo(
                    const ::baidu::galaxy::RunPodRequest* req,
                    PodInfo* pod_info) {
    if (pod_info == NULL) {
        return; 
    }
    pod_info->pod_id = req->podid();
    pod_info->job_id = req->jobid();
    pod_info->pod_desc.CopyFrom(req->pod());
    pod_info->pod_status.set_podid(req->podid());
    pod_info->pod_status.set_jobid(req->jobid());
    pod_info->pod_status.set_state(kPodPending);
    pod_info->initd_port = -1;
    pod_info->initd_pid = -1;
    
    for (int i = 0; i < pod_info->pod_desc.tasks_size(); i++) {
        TaskInfo task_info;
        task_info.task_id = GenerateTaskId(pod_info->pod_id);
        task_info.pod_id = pod_info->pod_id;
        task_info.job_id = pod_info->job_id;
        task_info.desc.CopyFrom(pod_info->pod_desc.tasks(i));
        task_info.desc.add_env("POD_VERSION=" + pod_info->pod_desc.version());
        task_info.status.set_state(kTaskPending);
        task_info.initd_endpoint = "";
        task_info.stage = kTaskStagePENDING;
        task_info.fail_retry_times = 0;
        task_info.max_retry_times = 10;
        pod_info->tasks[task_info.task_id] = task_info;
    }
    if (req->has_job_name()) {
        pod_info->job_name = req->job_name();
    }
}

void AgentImpl::RunPod(::google::protobuf::RpcController* /*cntl*/,
                       const ::baidu::galaxy::RunPodRequest* req,
                       ::baidu::galaxy::RunPodResponse* resp,
                       ::google::protobuf::Closure* done) {
    do {
        MutexLock scope_lock(&lock_);
        if (state_ == kOffline) {
            resp->set_status(kAgentOffline);
            lock_.Unlock();
            OfflineAgentRequest request;
            OfflineAgentResponse response;
            MutexLock lock(&mutex_master_endpoint_);
            request.set_endpoint(endpoint_);
            if (!rpc_client_->SendRequest(master_,
                                          &Master_Stub::OfflineAgent,
                                          &request,
                                          &response,
                                          5, 1)) {
                LOG(WARNING, "send offline request fail");
            }
            break;
        }
        if (!req->has_podid()
                || !req->has_pod()) {
            resp->set_status(kInputError);  
            break;
        }
        std::map<std::string, PodDescriptor>::iterator it = 
            pods_descs_.find(req->podid());
        if (it != pods_descs_.end()) {
            resp->set_status(kOk); 
            break;
        }
        if (AllocResource(req->pod().requirement()) != 0) {
            LOG(WARNING, "pod %s alloc resource failed",
                    req->podid().c_str()); 
            resp->set_status(kQuota);
            break;
        } 
        // NOTE alloc should before pods_desc set
        pods_descs_[req->podid()] = req->pod();

        PodInfo info;
        CreatePodInfo(req, &info);
        // if add failed, clean by master?
        pod_manager_.AddPod(info);
        if (0 != pod_manager_.ShowPod(req->podid(), &info) 
                || !persistence_handler_.SavePodInfo(info)) {
            LOG(WARNING, "pod %s persistence failed",
                         req->podid().c_str());
            // NOTE if persistence failed, need delete pod
            pod_manager_.DeletePod(req->podid());
            resp->set_status(kUnknown);
            break;
        }
        LOG(INFO, "run pod %s", req->podid().c_str());
        resp->set_status(kOk); 
    } while (0);
    done->Run();
    return;
}

void AgentImpl::KillPod(::google::protobuf::RpcController* /*cntl*/,
                        const ::baidu::galaxy::KillPodRequest* req,
                        ::baidu::galaxy::KillPodResponse* resp,
                        ::google::protobuf::Closure* done) {
    if (!req->has_podid()) {
        LOG(WARNING, "master kill request has no podid"); 
        resp->set_status(kInputError);
        done->Run();
        return;
    }
    MutexLock scope_lock(&lock_);
    std::map<std::string, PodDescriptor>::iterator it = 
        pods_descs_.find(req->podid());
    if (it == pods_descs_.end()) {
        LOG(WARNING, "pod %s not exists", req->podid().c_str());
        resp->set_status(kOk); 
        done->Run();
        return;
    }

    std::string pod_id = req->podid();
    resp->set_status(kOk); 
    done->Run();

    pod_manager_.DeletePod(pod_id);
    LOG(INFO, "pod %s add to delete", pod_id.c_str());
    return;
}

void AgentImpl::KillAllPods() {
    lock_.AssertHeld();
    for (std::map<std::string, PodDescriptor>::iterator it = pods_descs_.begin();
            it != pods_descs_.end(); it++) {
        std::string pod_id = it->first;
        pod_manager_.DeletePod(pod_id);
        LOG(WARNING, "fork to kill %s", pod_id.c_str());
    }
    return;
}

void AgentImpl::KeepHeartBeat() {
    MutexLock lock(&mutex_master_endpoint_);
    if (!PingMaster()) {
        LOG(WARNING, "ping master %s failed", 
                     master_endpoint_.c_str());
    }
    background_threads_.DelayTask(FLAGS_agent_heartbeat_interval,
                                  boost::bind(&AgentImpl::KeepHeartBeat, this));
    return;
}

bool AgentImpl::RestorePods() {
    MutexLock scope_lock(&lock_);
    std::vector<PodInfo> pods;
    if (!persistence_handler_.ScanPodInfo(&pods)) {
        LOG(WARNING, "scan pods failed");
        return false;
    }

    std::vector<PodInfo>::iterator it = pods.begin();
    for (; it != pods.end(); ++it) {
        PodInfo& pod = *it; 
        if (AllocResource(pod.pod_desc.requirement()) != 0) {
            LOG(WARNING, "alloc for pod %s failed require %ld %ld",
                    pod.pod_id.c_str(),
                    pod.pod_desc.requirement().millicores(),
                    pod.pod_desc.requirement().memory());
            return false;
        }
        pods_descs_[pod.pod_id] = pod.pod_desc;

        if (pod_manager_.ReloadPod(pod) != 0) {
            LOG(WARNING, "reload pod %s failed",
                    pod.pod_id.c_str()); 
            return false;
        }
    }
    return true;
}

bool AgentImpl::Init() {

    resource_capacity_.millicores = FLAGS_agent_millicores_share;
    resource_capacity_.memory = FLAGS_agent_mem_share; 
    if (!file::Mkdir(FLAGS_agent_work_dir)) {
        LOG(WARNING, "mkdir workdir %s falied", 
                            FLAGS_agent_work_dir.c_str()); 
        return false;
    }

    if (!persistence_handler_.Init(FLAGS_agent_persistence_path)) {
        LOG(WARNING, "init persistence handler failed");
        return false;  
    }

    if (pod_manager_.Init() != 0) {
        LOG(WARNING, "init pod manager failed");
        return false; 
    }

    if (!RestorePods()) {
        LOG(WARNING, "restore pods failed"); 
        return false;
    }

    if (!RegistToMaster()) {
        return false; 
    }
    background_threads_.DelayTask(
            FLAGS_agent_heartbeat_interval, boost::bind(&AgentImpl::KeepHeartBeat, this));

    background_threads_.DelayTask(
                500, 
                boost::bind(&AgentImpl::LoopCheckPods, this)); 
    
    background_threads_.AddTask(boost::bind(&AgentImpl::CollectSysStat, this));
    return true;
}

bool AgentImpl::PingMaster() {
    mutex_master_endpoint_.AssertHeld();
    HeartBeatRequest request;
    HeartBeatResponse response;
    request.set_endpoint(endpoint_);
    return rpc_client_->SendRequest(master_,
                                    &Master_Stub::HeartBeat,
                                    &request,
                                    &response,
                                    5, 1);    
}

void AgentImpl::LoopCheckPods() {
    MutexLock scope_lock(&lock_);
    std::map<std::string, PodDescriptor>::iterator it = 
        pods_descs_.begin();
    std::vector<std::string> to_del_pod;
    for (; it != pods_descs_.end(); ++it) {
        if (pod_manager_.CheckPod(it->first) != 0) {
            if (!persistence_handler_.DeletePodInfo(it->first)) {
                LOG(WARNING, "delete persistence pod %s failed",
                        it->first.c_str());
                continue;
            }
            to_del_pod.push_back(it->first);
            ReleaseResource(it->second.requirement()); 
        }  
    }
    for (size_t i = 0; i < to_del_pod.size(); i++) {
        pods_descs_.erase(to_del_pod[i]); 
    }
    background_threads_.DelayTask(
                100, 
                boost::bind(&AgentImpl::LoopCheckPods, this)); 
}

void AgentImpl::HandleMasterChange(const std::string& new_master_endpoint) {
    if (new_master_endpoint.empty()) {
        LOG(WARNING, "the master endpoint is deleted from nexus");
    }
    if (new_master_endpoint != master_endpoint_) {
        MutexLock lock(&mutex_master_endpoint_);
        LOG(INFO, "master change to %s", new_master_endpoint.c_str());
        master_endpoint_ = new_master_endpoint; 
        if (master_) {
            delete master_;
        }
        if (!rpc_client_->GetStub(master_endpoint_, &master_)) {
            LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
            return;
        }  
    }
}

bool AgentImpl::RegistToMaster() {
    boost::function<void(const std::string)> handler;
    handler = boost::bind(&AgentImpl::HandleMasterChange, this, _1);
    bool ok = master_watcher_->Init(handler);
    if (!ok) {
        LOG(WARNING, "fail to watch on nexus");
        return false;
    }
    MutexLock lock(&mutex_master_endpoint_);
    
    master_endpoint_ = master_watcher_->GetMasterEndpoint();

    if (!rpc_client_->GetStub(master_endpoint_, &master_)) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
        return false;
    }

    if (!PingMaster()) {
        LOG(WARNING, "connect master %s failed", master_endpoint_.c_str()); 
    }
    LOG(INFO, "regist to master %s", master_endpoint_.c_str());
    return true;
}

int AgentImpl::AllocResource(const Resource& requirement) {
    lock_.AssertHeld();
    if (resource_capacity_.millicores >= requirement.millicores()
            && resource_capacity_.memory >= requirement.memory()) {
        boost::unordered_set<int32_t> ports;
        for (int i = 0; i < requirement.ports_size(); i++) {
            int32_t port = requirement.ports(i);
            if (resource_capacity_.used_port.find(port) 
                    != resource_capacity_.used_port.end()) {
                return -1; 
            }
            ports.insert(port);
        }
        boost::unordered_set<int32_t>::iterator it = ports.begin();
        for (; it != ports.end(); ++it) {
            resource_capacity_.used_port.insert(*it);
        }
        resource_capacity_.millicores -= requirement.millicores(); 
        resource_capacity_.memory -= requirement.memory();

        return 0;
    }
    return -1;
}

void AgentImpl::ReleaseResource(const Resource& requirement) {
    lock_.AssertHeld();

    resource_capacity_.millicores += requirement.millicores();
    resource_capacity_.memory += requirement.memory();
    for (int i = 0; i < requirement.ports_size(); i++) {
        resource_capacity_.used_port.erase(requirement.ports(i));
    }
}

void AgentImpl::ConvertToPodPropertiy(const PodInfo& show_pod_info, PodPropertiy* pod_propertiy) {
    pod_propertiy->set_pod_id(show_pod_info.pod_id);
    pod_propertiy->set_job_id(show_pod_info.job_id);
    pod_propertiy->mutable_pod_desc()->CopyFrom(
                            show_pod_info.pod_desc);
    pod_propertiy->mutable_pod_status()->CopyFrom(
                            show_pod_info.pod_status);
    std::string initd_endpoint = "127.0.0.1:";
    initd_endpoint.append(
            boost::lexical_cast<std::string>(
                    show_pod_info.initd_port));
    pod_propertiy->set_initd_endpoint(initd_endpoint);
    pod_propertiy->set_pod_path(show_pod_info.pod_path);
    pod_propertiy->set_job_name(show_pod_info.job_name);
    return;
}

void AgentImpl::ShowPods(::google::protobuf::RpcController* /*cntl*/,
                         const ::baidu::galaxy::ShowPodsRequest* req,
                         ::baidu::galaxy::ShowPodsResponse* resp,
                         ::google::protobuf::Closure* done) {
    MutexLock scope_lock(&lock_);
    resp->set_status(kOk);
    do {
        if (req->has_podid()) {
            PodInfo show_pod_info;          
            if (0 != 
                    pod_manager_.ShowPod(req->podid(), 
                                         &show_pod_info)) { 
                LOG(WARNING, "pod %s not exists", 
                             req->podid().c_str());
                resp->set_status(kUnknown);
                break;
            }
            PodPropertiy* pod_propertiy = resp->add_pods();
            ConvertToPodPropertiy(show_pod_info, 
                                  pod_propertiy);
            break;
        }     
        std::vector<PodInfo> pods; 
        pod_manager_.ShowPods(&pods);
        std::vector<PodInfo>::iterator it = pods.begin();
        for (; it != pods.end(); ++it) {
            ConvertToPodPropertiy(*it, resp->add_pods());            
        }
        LOG(INFO, "pods show size %d", resp->pods_size());
    } while (0);
    done->Run();    
    return;
}

void AgentImpl::CollectSysStat() {
    do {
        LOG(INFO, "start collect sys stat");
        ResourceStatistics tmp_statistics;
        bool ok = GetGlobalCpuStat();
        if (!ok) {
            LOG(WARNING, "fail to get cpu usage");
            break;
        }
        ok = GetGlobalMemStat();
        if (!ok) {
            LOG(WARNING, "fail to get mem usage");
            break;
        }
        ok = GetGlobalIntrStat();
        if (!ok) {
            LOG(WARNING, "fail to get interupt usage");
            break;
        }
        ok = GetGlobalIOStat();
        if (!ok) {
            LOG(WARNING, "fail to get IO usage");
            break;
        }
        ok = GetGlobalNetStat();
        if (!ok) {
            LOG(WARNING, "fail to get Net usage");
            break;
        }
        stat_->collect_times_++;
        if (stat_->collect_times_ < MIN_COLLECT_TIME) {
            LOG(WARNING, "collect times not reach %d", MIN_COLLECT_TIME);
            break;
        }
        if (CheckSysHealth()) {
            MutexLock scope_lock(&lock_);
            if (state_ == kAlive) {
                break;
            }
            recover_threshold_++;
            if (recover_threshold_ > FLAGS_agent_recover_threshold) {
                state_ = kAlive;
                lock_.Unlock();
                OnlineAgentRequest request;
                OnlineAgentResponse response;
                MutexLock lock(&mutex_master_endpoint_);
                request.set_endpoint(endpoint_);
                if (!rpc_client_->SendRequest(master_,
                                              &Master_Stub::OnlineAgent,
                                              &request,
                                              &response,
                                              5, 1)) {
                    LOG(WARNING, "send online request fail");
                }
            }
            break;
        }else {
            MutexLock scope_lock(&lock_);
            if (state_ == kOffline) {
                break;
            }
            state_ = kOffline;
            recover_threshold_ = 0;
            KillAllPods();
            LOG(WARNING, "sys health checker kill all pods");
            lock_.Unlock();
            OfflineAgentRequest request;
            OfflineAgentResponse response;
            MutexLock lock(&mutex_master_endpoint_);
            request.set_endpoint(endpoint_);
            if (!rpc_client_->SendRequest(master_,
                                         &Master_Stub::OfflineAgent,
                                         &request,
                                         &response,
                                         5, 1)) {
                LOG(WARNING, "send offline request fail");
            }
        }
    } while(0); 
    background_threads_.DelayTask(FLAGS_stat_check_period, boost::bind(&AgentImpl::CollectSysStat, this));
    return;
}

bool AgentImpl::CheckSysHealth() {
    if (fabs(FLAGS_max_cpu_usage) <= 1e-6 && stat_->cpu_used_ > FLAGS_max_cpu_usage) {
        LOG(WARNING, "cpu uage %f reach threshold %f", stat_->cpu_used_, FLAGS_max_cpu_usage);
        return false;
    }
    if (fabs(FLAGS_max_mem_usage) <= 1e-6 && stat_->mem_used_ > FLAGS_max_mem_usage) {
        LOG(WARNING, "mem usage %f reach threshold %f", stat_->mem_used_, FLAGS_max_mem_usage);
        return false;
    }
    if (fabs(FLAGS_max_disk_r_bps) <= 1e-6 && stat_->disk_read_Bps_ > FLAGS_max_disk_r_bps) {
        LOG(WARNING, "disk read Bps %f reach threshold %f", stat_->disk_read_Bps_, FLAGS_max_disk_r_bps);
        return false;
    }
    if (fabs(FLAGS_max_disk_w_bps) <= 1e-6 && stat_->disk_write_Bps_ > FLAGS_max_disk_w_bps) {
        LOG(WARNING, "disk write Bps %f reach threshold %f", stat_->disk_write_Bps_, FLAGS_max_disk_w_bps);
        return false;
    }
    if (fabs(FLAGS_max_disk_r_rate) <= 1e-6 && stat_->disk_read_times_ > FLAGS_max_disk_r_rate) {
        LOG(WARNING, "disk write rate %f reach threshold %f", stat_->disk_read_times_, FLAGS_max_disk_r_rate);
        return false;
    }
    if (fabs(FLAGS_max_disk_w_rate) <= 1e-6 && stat_->disk_write_times_ > FLAGS_max_disk_w_rate) {
        LOG(WARNING, "disk write rate %f reach threshold %f", stat_->disk_write_times_, FLAGS_max_disk_w_rate);
        return false;
    }
    if (fabs(FLAGS_max_disk_util) <= 1e-6 && stat_->disk_io_util_ > FLAGS_max_disk_util) {
        LOG(WARNING, "disk io util %f reach threshold %f", stat_->disk_io_util_, FLAGS_max_disk_util);
        return false;
    }
    if (fabs(FLAGS_max_net_in_bps) <= 1e-6 != 0 && stat_->net_in_bps_ > FLAGS_max_net_in_bps) {
        LOG(WARNING, "net in bps %f reach threshold %f", stat_->net_in_bps_, FLAGS_max_net_in_bps);
        return false;
    }
    if (fabs(FLAGS_max_net_out_bps) <= 1e-6 && stat_->net_out_bps_ > FLAGS_max_net_out_bps) {
        LOG(WARNING, "net out bps %f reach threshold %f", stat_->net_out_bps_, FLAGS_max_net_out_bps);
        return false;
    }
    if (fabs(FLAGS_max_net_in_pps) <= 1e-6 && stat_->net_in_pps_ > FLAGS_max_net_in_pps) {
        LOG(WARNING, "net in pps %f reach threshold %f", stat_->net_in_bps_, FLAGS_max_net_in_pps);
        return false;
    }
    if (fabs(FLAGS_max_net_out_pps) <= 1e-6 && stat_->net_out_pps_ > FLAGS_max_net_out_pps) {
        LOG(WARNING, "net out pps %f reach threshold %f", stat_->net_out_pps_, FLAGS_max_net_out_pps);
        return false;
    }
    if (fabs(FLAGS_max_intr_rate) <= 1e-6  && stat_->intr_rate_ > FLAGS_max_intr_rate) {
        LOG(WARNING, "interupt rate %f reach threshold %f", stat_->intr_rate_, FLAGS_max_intr_rate);
        return false;
    }
    if (fabs(FLAGS_max_soft_intr_rate) <= 1e-6 && stat_->soft_intr_rate_ > FLAGS_max_soft_intr_rate) {
        LOG(WARNING, "soft interupt rate %f reach threshold %f", stat_->soft_intr_rate_, FLAGS_max_soft_intr_rate);
        return false;
    }
    return true;
}

bool AgentImpl::GetGlobalCpuStat() {
    ResourceStatistics statistics;
    std::string path = "/proc/stat";
    FILE* fin = fopen(path.c_str(), "r");
    if (fin == NULL) {
        LOG(WARNING, "open %s failed", path.c_str());
        return false; 
    }

    ssize_t read;
    size_t len = 0;
    char* line = NULL;
    if ((read = getline(&line, &len, fin)) == -1) {
        LOG(WARNING, "read line failed err[%d: %s]", 
                errno, strerror(errno)); 
        fclose(fin);
        return false;
    }
    fclose(fin);

    char cpu[5];
    int item_size = sscanf(line, 
                           "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld", 
                           cpu,
                           &(statistics.cpu_user_time),
                           &(statistics.cpu_nice_time),
                           &(statistics.cpu_system_time),
                           &(statistics.cpu_idle_time),
                           &(statistics.cpu_iowait_time),
                           &(statistics.cpu_irq_time),
                           &(statistics.cpu_softirq_time),
                           &(statistics.cpu_stealstolen),
                           &(statistics.cpu_guest)); 

    free(line); 
    line = NULL;
    if (item_size != 10) {
        LOG(WARNING, "read from /proc/stat format err"); 
        return false;
    }
    stat_->last_stat_ = stat_->cur_stat_;
    stat_->cur_stat_ = statistics;
    long total_cpu_time_last = 
    stat_->last_stat_.cpu_user_time
    + stat_->last_stat_.cpu_nice_time
    + stat_->last_stat_.cpu_system_time
    + stat_->last_stat_.cpu_idle_time
    + stat_->last_stat_.cpu_iowait_time
    + stat_->last_stat_.cpu_irq_time
    + stat_->last_stat_.cpu_softirq_time
    + stat_->last_stat_.cpu_stealstolen
    + stat_->last_stat_.cpu_guest;
    long total_cpu_time_cur =
    stat_->cur_stat_.cpu_user_time
    + stat_->cur_stat_.cpu_nice_time
    + stat_->cur_stat_.cpu_system_time
    + stat_->cur_stat_.cpu_idle_time
    + stat_->cur_stat_.cpu_iowait_time
    + stat_->cur_stat_.cpu_irq_time
    + stat_->cur_stat_.cpu_softirq_time
    + stat_->cur_stat_.cpu_stealstolen
    + stat_->cur_stat_.cpu_guest;
    long total_cpu_time = total_cpu_time_cur - total_cpu_time_last;
    if (total_cpu_time < 0) {
        LOG(WARNING, "invalide total cpu time cur %ld last %ld", total_cpu_time_cur, total_cpu_time_last);
        return false;
    }     

    long total_used_time_last = 
    stat_->last_stat_.cpu_user_time 
    + stat_->last_stat_.cpu_system_time
    + stat_->last_stat_.cpu_nice_time
    + stat_->last_stat_.cpu_irq_time
    + stat_->last_stat_.cpu_softirq_time
    + stat_->last_stat_.cpu_stealstolen
    + stat_->last_stat_.cpu_guest;

    long total_used_time_cur =
    stat_->cur_stat_.cpu_user_time
    + stat_->cur_stat_.cpu_nice_time
    + stat_->cur_stat_.cpu_system_time
    + stat_->cur_stat_.cpu_irq_time
    + stat_->cur_stat_.cpu_softirq_time
    + stat_->cur_stat_.cpu_stealstolen
    + stat_->cur_stat_.cpu_guest;
    long total_cpu_used_time = total_used_time_cur - total_used_time_last;
    if (total_cpu_used_time < 0)  {
        LOG(WARNING, "invalude total cpu used time cur %ld last %ld", total_used_time_cur, total_used_time_last);
        return false;
    }
    double rs = total_cpu_used_time / static_cast<double>(total_cpu_time);
    stat_->cpu_used_ = rs;
    return true;
}

bool AgentImpl::GetGlobalMemStat(){
    FILE* fp = fopen("/proc/meminfo", "rb");
    if (fp == NULL) {
        return false;
    }
    std::string content;
 	char buf[1024];
 	int len = 0;
    while ((len = fread(buf, 1, sizeof(buf), fp)) > 0) {
        content.append(buf, len);
    }
    std::vector<std::string> lines;
    boost::split(lines, content, boost::is_any_of("\n"));
    int64_t total_mem = 0;
    int64_t free_mem = 0;
    int64_t buffer_mem = 0;
    int64_t cache_mem = 0;
    int64_t tmpfs_mem = 0;
    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = lines[i];
        std::vector<std::string> parts;
        if (line.find("MemTotal:") == 0) {
            boost::split(parts, line, boost::is_any_of(" "));
            if (parts.size() < 2) {
                fclose(fp);
                return false;
            }
            total_mem = boost::lexical_cast<int64_t>(parts[parts.size() - 2]);
        }else if (line.find("MemFree:") == 0) {
            boost::split(parts, line, boost::is_any_of(" "));
            if (parts.size() < 2) {
                fclose(fp);
                return false;
            }
            free_mem = boost::lexical_cast<int64_t>(parts[parts.size() - 2]);
        }else if (line.find("Buffers:") == 0) {
            boost::split(parts, line, boost::is_any_of(" "));
            if (parts.size() < 2) {
                fclose(fp);
                return false;
            }
            buffer_mem = boost::lexical_cast<int64_t>(parts[parts.size() - 2]);
        }else if (line.find("Cached:") == 0) {
            boost::split(parts, line, boost::is_any_of(" "));
            if (parts.size() < 2) {
                fclose(fp);
                return false;
            }
            cache_mem = boost::lexical_cast<int64_t>(parts[parts.size() - 2]);
        }
    }
    fclose(fp);
    
    FILE* fin = popen("df -h", "r");
    if (fin != NULL) {
        std::string content;
        char buf[1024];
        int len = 0;
        while ((len = fread(buf, 1, sizeof(buf), fin)) > 0) {
            content.append(buf, len);
        }
        std::vector<std::string> lines;
        boost::split(lines, content, boost::is_any_of("\n"));
        for (size_t n = 0; n < lines.size(); n++) {
            std::string line = lines[n];
            if (line.find("tmpfs")) {
                std::vector<std::string> parts;
                boost::split(parts, line, boost::is_any_of(" "));
                tmpfs_mem = boost::lexical_cast<int64_t>(parts[1]);
                LOG(WARNING, "detect tmpfs %s %d", parts[1].c_str(), tmpfs_mem);
                tmpfs_mem = tmpfs_mem * 1024 * 1024 * 1024;
                break;
            } else {
                continue;
            }
        }
    } else {
        LOG(WARNING, "popen df -h err");
    }
    if (pclose(fin) == -1) {
        LOG(WARNING, "pclose err");
    }
    stat_->mem_used_ = (total_mem - free_mem - buffer_mem - cache_mem + tmpfs_mem) / boost::lexical_cast<double>(total_mem);
 
    return true;
}

bool AgentImpl::GetGlobalIntrStat() {
    uint64_t intr_cnt = 0;
    uint64_t softintr_cnt = 0;
    std::string path = "/proc/stat";
    std::ifstream stat(path.c_str());
    if (!stat.is_open()) {
        LOG(WARNING, "open proc stat fail.");
        return false;
    } 
    std::vector<std::string> lines;
    std::string content; 
    stat >> content;
    stat.close();
    boost::split(lines, content, boost::is_any_of("\n"));
    for (size_t n = 0; n < lines.size(); n++) {
        std::string line = lines[n];
        if (line.find("intr")) {
            std::vector<std::string> parts;
            boost::split(parts, line, boost::is_any_of(" "));
            intr_cnt = boost::lexical_cast<int64_t>(parts[1]);
        } else if (line.find("softirq")) {
            std::vector<std::string> parts;
            boost::split(parts, line, boost::is_any_of(" "));
            softintr_cnt = boost::lexical_cast<int64_t>(parts[1]);
        }
        continue;
    }
    stat_->last_stat_ = stat_->cur_stat_;
    stat_->cur_stat_.interupt_times = intr_cnt;
    stat_->cur_stat_.soft_interupt_times = softintr_cnt;
    stat_->intr_rate_ = (stat_->cur_stat_.interupt_times - stat_->last_stat_.interupt_times) / FLAGS_stat_check_period * 1000; 
    stat_->soft_intr_rate_ = (stat_->cur_stat_.soft_interupt_times - stat_->last_stat_.soft_interupt_times) / FLAGS_stat_check_period * 1000;
    return true;
}

bool AgentImpl::GetGlobalIOStat() {
    FILE *fin = popen("iostat -x", "r");
    if (fin != NULL) {
        char buf[1024];
        int len = 0;
        std::string content;
        while ((len = fread(buf, 1, sizeof(buf), fin)) > 0) {
            content.append(buf, len);
        }
        std::vector<std::string> lines;
        boost::split(lines, content, boost::is_any_of("\n"));
        for (size_t n = 0; n < lines.size(); n++) {
            std::string line = lines[n];
            if (line.find("sda")) {
                std::vector<std::string> parts;
                boost::split(parts, line, boost::is_any_of(" "));
                stat_->disk_read_times_ = boost::lexical_cast<double>(parts[3]);
                stat_->disk_write_times_ = boost::lexical_cast<double>(parts[4]);
                stat_->disk_read_Bps_ = boost::lexical_cast<double>(parts[5]);
                stat_->disk_write_Bps_ = boost::lexical_cast<double>(parts[6]);
                stat_->disk_io_util_ = boost::lexical_cast<double>(parts[lines.size() - 1]);
                break;
            } else {
                continue;
            }
        }
    }else {
        LOG(WARNING, "popen iostat fail err_code");
    }
    if (pclose(fin) == -1) {
        LOG(WARNING, "pclose err");
    }
    return true;
}

bool AgentImpl::GetGlobalNetStat() {
    std::string path = "/proc/net/dev";
    std::ifstream stat(path.c_str());
    if (!stat.is_open()) {
        LOG(WARNING, "open dev stat fail.");
        return false;
    }
    std::string content;
    stat >> content;
    stat.close();
    stat_->last_stat_ = stat_->cur_stat_;
    std::vector<std::string> lines;
    boost::split(lines, content, boost::is_any_of("\n"));
    for (size_t n = 0; n < lines.size(); n++) {
        std::string line = lines[n];
        if (line.find("eth0") || line.find("xgbe0")) {
            std::vector<std::string> parts;
            boost::split(parts, line, boost::is_any_of(" "));
            std::vector<std::string> tokens;
            boost::split(tokens, parts[0], boost::is_any_of(":"));
            stat_->cur_stat_.net_in_bits = boost::lexical_cast<int64_t>(tokens[1]);
            stat_->cur_stat_.net_in_packets = boost::lexical_cast<int64_t>(parts[1]);
            stat_->cur_stat_.net_out_bits = boost::lexical_cast<int64_t>(tokens[8]);
            stat_->cur_stat_.net_out_packets = boost::lexical_cast<int64_t>(parts[9]);
        }
        continue;
    }
    stat_->net_in_bps_ = (stat_->cur_stat_.net_in_bits - stat_->last_stat_.net_in_bits) / FLAGS_stat_check_period * 1000;
    stat_->net_out_bps_ = (stat_->cur_stat_.net_out_bits - stat_->last_stat_.net_out_bits) / FLAGS_stat_check_period * 1000;
    stat_->net_in_pps_ = (stat_->cur_stat_.net_in_packets - stat_->last_stat_.net_in_packets) / FLAGS_stat_check_period * 1000;
    stat_->net_out_pps_ = (stat_->cur_stat_.net_out_packets - stat_->last_stat_.net_out_packets) / FLAGS_stat_check_period * 1000;
    return true;
}
}   // ending namespace galaxy
}   // ending namespace baidu
