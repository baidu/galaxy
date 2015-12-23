// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _AGENT_IMPL_H_
#define _AGENT_IMPL_H_

#include <string>
#include <map>

#include "boost/unordered_set.hpp"

#include "sofa/pbrpc/pbrpc.h"
#include "proto/galaxy.pb.h"
#include "proto/agent.pb.h"
#include "proto/master.pb.h"

#include "mutex.h"
#include "thread_pool.h"
#include "rpc/rpc_client.h"

#include "agent/pod_manager.h"
#include "master/master_watcher.h"
#include "agent/persistence_handler.h"

namespace baidu {
namespace galaxy {

class AgentImpl : public Agent {

public:
    AgentImpl();
    virtual ~AgentImpl(); 

    bool Init();

    virtual void Query(::google::protobuf::RpcController* cntl, 
                       const ::baidu::galaxy::QueryRequest* req,
                       ::baidu::galaxy::QueryResponse* resp,
                       ::google::protobuf::Closure* done); 
    virtual void RunPod(::google::protobuf::RpcController* cntl,
                        const ::baidu::galaxy::RunPodRequest* req,
                        ::baidu::galaxy::RunPodResponse* resp,
                        ::google::protobuf::Closure* done);
    virtual void KillPod(::google::protobuf::RpcController* cntl,
                         const ::baidu::galaxy::KillPodRequest* req,
                         ::baidu::galaxy::KillPodResponse* resp,
                         ::google::protobuf::Closure* done);
    virtual void ShowPods(::google::protobuf::RpcController* cntl,
                         const ::baidu::galaxy::ShowPodsRequest* req,
                         ::baidu::galaxy::ShowPodsResponse* resp,
                         ::google::protobuf::Closure* done);

    void HandleMasterChange(const std::string& new_master_endpoint);
private:
    void KeepHeartBeat();
    
    bool RegistToMaster();
    bool CheckGcedConnection();

    bool PingMaster();

    bool RestorePods();
    
    void LoopCheckPods();
    
    void KillAllPods();
    // ret ==  0 alloc success
    //        -1 alloc failed
    int AllocResource(const Resource& requirement);
    void ReleaseResource(const Resource& requirement);

    void ConvertToPodPropertiy(const PodInfo& info, PodPropertiy* pod_propertiy);

    void CreatePodInfo(const ::baidu::galaxy::RunPodRequest* req, 
                       PodInfo* pod_info);
    
    void CollectSysStat();
    bool GetGlobalCpuStat();
    bool GetGlobalMemStat();
    bool GetGlobalIntrStat();
    bool GetGlobalNetStat();
    bool GetGlobalIOStat();
    bool CheckSysHealth();

    struct ResourceCapacity {
        int64_t millicores; 
        int64_t memory;
        // TODO initd port not included
        boost::unordered_set<int32_t> used_port;
        ResourceCapacity() 
            : millicores(0),
              memory(0),
              used_port() {
        }
    };
    
    struct ResourceStatistics {
        // cpu
        long cpu_user_time; 
        long cpu_nice_time;
        long cpu_system_time;
        long cpu_idle_time;
        long cpu_iowait_time;
        long cpu_irq_time;
        long cpu_softirq_time;
        long cpu_stealstolen;
        long cpu_guest;

        long cpu_cores;

        //mem
        long memory_rss_in_bytes;
        long tmpfs_in_bytes;

        //interupt
        long interupt_times;
        long soft_interupt_times;

        //net
        long net_in_bits;
        long net_out_bits;
        long net_in_packets;
        long net_out_packets;
    
        ResourceStatistics() :
            cpu_user_time(0),
            cpu_nice_time(0),
            cpu_system_time(0),
            cpu_idle_time(0),
            cpu_iowait_time(0),
            cpu_irq_time(0),
            cpu_softirq_time(0),
            cpu_stealstolen(0),
            cpu_guest(0),
            cpu_cores(0),
            memory_rss_in_bytes(0),
            tmpfs_in_bytes(0),
            interupt_times(0),
            soft_interupt_times(0),
            net_in_bits(0),
            net_out_bits(0),
            net_in_packets(0),
            net_out_packets(0) {}
    };  

    struct SysStat {
        ResourceStatistics last_stat_;
        ResourceStatistics cur_stat_;
        double cpu_used_;
        double mem_used_;
        double disk_read_Bps_;
        double disk_write_Bps_;
        double disk_read_times_;
        double disk_write_times_;
        double disk_io_util_;
        double net_in_bps_;
        double net_out_bps_;
        double net_in_pps_;
        double net_out_pps_;
        double intr_rate_;
        double soft_intr_rate_;
        uint64_t collect_times_;
        SysStat():last_stat_(),
                  cur_stat_(),
                  cpu_used_(0.0),
                  mem_used_(0.0),
                  disk_read_Bps_(0.0),
                  disk_write_Bps_(0.0),
                  disk_read_times_(0.0),
                  disk_write_times_(0.0),
                  disk_io_util_(0.0),
                  net_in_bps_(0.0),
                  net_out_bps_(0.0),
                  net_in_pps_(0.0),
                  net_out_pps_(0.0),
                  intr_rate_(0.0),
                  soft_intr_rate_(0.0),
                  collect_times_(0) {}
        ~SysStat(){
        }
    };
    
private:
    std::string master_endpoint_;
    
    Mutex lock_;
    ThreadPool background_threads_;
    RpcClient* rpc_client_;
    std::string endpoint_;
    Master_Stub* master_;
    ResourceCapacity resource_capacity_;

    MasterWatcher* master_watcher_;
    Mutex mutex_master_endpoint_;

    PodManager pod_manager_;
    std::map<std::string, PodDescriptor> pods_descs_; 
    PersistenceHandler persistence_handler_;
    SysStat* stat_;
    AgentState state_;
    
    
};

}   // ending namespace galaxy
}   // ending namespace baidu


#endif
