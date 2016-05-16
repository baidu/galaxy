// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <string>
#include <stdint.h>
#include <vector>

namespace baidu {
namespace galaxy {

class RpcClient;

namespace sdk {

struct User {
    std::string user;
    std::string token;
};
struct Quota {
    int64_t millicore;
    int64_t memory;
    int64_t disk;
    int64_t ssd;
    int64_t replica;
};
enum Authority {
    kAuthorityCreateContainer=1,
    kAuthorityRemoveContainer=2,
    kAuthorityUpdateContainer=3,
    kAuthorityListContainer=4,
    kAuthoritySubmitJob=5,
    kAuthorityRemoveJob=6,
    kAuthorityUpdateJob=7,
    kAuthorityListJobs=8,
};
enum AuthorityAction {
    kActionAdd=1,
    kActionRemove=2,
    kActionSet=3,
    kActionClear=4,
};
struct Grant {
    User user;
    std::vector<std::string> pool;
    AuthorityAction action;
    std::vector<Authority> authority;
};
struct Resource {
    int64_t total;
    int64_t assigned;
    int64_t used;
};
enum VolumMedium {
    kSsd=1,
    kDisk=2,
    kBfs=3,
    kTmpfs=4,
};
struct VolumResource {
    VolumMedium medium;
    Resource volum;
    std::string device_path;
};
struct Volum {
    std::string path;
    int64_t used_size;
    int64_t assigned_size;
    VolumMedium medium;
};
struct CpuRequired {
    int64_t milli_core;
    bool excess;
};
struct MemoryRequired {
    int64_t size;
    bool excess;
};
struct TcpthrotRequired {
    int64_t recv_bps_quota;
    bool recv_bps_excess;
    int64_t send_bps_quota;
    bool send_bps_excess;
};
struct BlkioRequired {
    int32_t weight;
};
struct PortRequired {
    std::string port_name;
    std::string port;
};
enum VolumType {
    kEmptyDir=1,
    kHostDir=2,
};
struct VolumRequired {
    int64_t size;
    VolumType type;
    VolumMedium medium;
    std::string source_path;
    std::string dest_path;
    bool readonly;
    bool exclusive;
    bool use_symlink;
};
enum JobType {
    kJobMonitor=0,
    kJobService=100,
    kJobBatch=200,
    kJobBestEffort=300,
};
enum JobStatus {
    kJobPending=1,
    kJobRunning=2,
    kJobFinished=3,
    kJobDestroying=4,
};
enum PodStatus {
    kPodPending=1,
    kPodReady=2,
    kPodDeploying=3,
    kPodStarting=4,
    kPodServing=5,
    kPodFailed=6,
    kPodFinished=7,
};
enum TaskStatus {
    kTaskPending=1,
    kTaskDeploying=2,
    kTaskStarting=3,
    kTaskServing=4,
    kTaskFailed=5,
    kTaskFinished=6,
};
struct Package {
    std::string source_path;
    std::string dest_path;
    std::string version;
};
struct ImagePackage {
    Package package;
    std::string start_cmd;
    std::string stop_cmd;
};
struct DataPackage {
    std::vector<Package> packages;
    std::string reload_cmd;
};
struct Deploy {
    uint32_t replica;
    uint32_t step;
    uint32_t interval;
    uint32_t max_per_host;
};
struct Service {
    std::string service_name;
    std::string port_name;
    bool use_bns;
};
struct TaskDescription {
    std::string id;
    CpuRequired cpu;
    MemoryRequired memory;
    std::vector<PortRequired> ports;
    ImagePackage exe_package;
    DataPackage data_package;
    std::vector<Service> services;
};
struct PodDescription {
    VolumRequired workspace_volum;
    std::vector<VolumRequired> data_volums;
    std::vector<TaskDescription> tasks;
};
struct JobDescription {
    std::string name;
    JobType type;
    std::string version;
    Deploy deploy;
    PodDescription pod_desc;
};
struct Cgroup {
    std::string id;
    CpuRequired cpu;
    MemoryRequired memory;
    TcpthrotRequired tcp_throt;
    BlkioRequired blkio;
};
struct ContainerDescription {
    uint32_t priority;
    std::string run_user;
    std::string version;
    VolumRequired workspace_volum;
    std::vector<VolumRequired> data_volums;
    std::string cmd_line;
    std::vector<Cgroup> cgroups;
    int32_t max_per_host;
    std::string tag;
    std::vector<std::string> pool_names;
};
enum ContainerStatus {
    kContainerPending=1,
    kContainerAllocating=2,
    kContainerReady=3,
    kContainerError=4,
    kContainerDestroying=5,
    kContainerTerminated=6,
};
enum ContainerGroupStatus {
    kContainerGroupNormal=1,
    kContainerGroupTerminated=2,
};
struct ContainerInfo {
    std::string id;
    std::string group_id;
    int64_t created_time;
    ContainerStatus status;
    PodDescription pod_desc;
    int64_t cpu_used;
    int64_t memory_used;
    std::vector<Volum> volum_used;
    std::vector<std::string> port_used;
    uint32_t restart_counter;
};
enum Status {
    kOk=1,
    kStop=2,
};
struct ErrorCode {
    Status status;
    std::string reason;
};
enum AgentStatus {
    kAgentAlive=1,
    kAgentDead=2,
    kAgentOffline=3,
};
struct AgentInfo {
    std::string version;
    int64_t start_time;
    bool unhealthy;
    std::vector<ContainerInfo> container_info;
    Resource cpu_resoruce;
    Resource memory_resource;
    std::vector<VolumResource> volum_resources;
};

struct EnterSafeModeRequest {
    std::string user;
};
struct EnterSafeModeResponse {
    ErrorCode error_code;
};
struct LeaveSafeModeRequest {
    std::string user;
};
struct LeaveSafeModeResponse {
    ErrorCode error_code;
};
struct StatusRequest {
    User user;
};
struct PoolStatus {
    std::string name;
    uint32_t total_agents;
    uint32_t alive_agents;
};
struct StatusResponse {
    ErrorCode error_code;
    uint32_t alive_agents;
    uint32_t dead_agents;
    uint32_t total_agents;
    Resource cpu;
    Resource memory;
    VolumResource volum;
    uint32_t total_groups;
    uint32_t total_containers;
    std::vector<PoolStatus> pools;
};

struct AddAgentRequest {
    User user;
    std::string endpoint;
    std::string pool;
};
struct AddAgentResponse {
    ErrorCode error_code;
};
struct RemoveAgentRequest {
    User user;
    std::string endpoint;
};
struct RemoveAgentResponse {
    ErrorCode error_code;
};
struct OnlineAgentRequest {
    User user;
    std::string endpoint;
};
struct OnlineAgentResponse {
    ErrorCode error_code;
};
struct OfflineAgentRequest {
    User user;
    std::string endpoint;
};
struct OfflineAgentResponse {
    ErrorCode error_code;
};
struct ListAgentsRequest {
    User user;
    std::string pool;
};
struct AgentStatistics {
    std::string endpoint;
    AgentStatus status;
    std::string pool;
    std::vector<std::string> tags;
    Resource cpu;
    Resource memory;
    VolumResource volum;
    uint32_t total_containers;
};
struct ListAgentsResponse {
    ErrorCode error_code;
    std::vector<AgentStatistics> agents;
};
struct CreateTagRequest {
    User user;
    std::string tag;
    std::vector<std::string> endpoint;
};
struct CreateTagResponse {
    ErrorCode error_code;
};
struct ListTagsRequest {
    User user;
};
struct ListTagsResponse {
    ErrorCode error_code;
    std::vector<std::string> tags;
};
struct ListAgentsByTagRequest {
    User user;
    std::string tag;
};
struct ListAgentsByTagResponse {
    ErrorCode error_code;
    std::vector<std::string> endpoint;
};
struct GetTagsByAgentRequest {
    User user;
    std::string endpoint;
};
struct GetTagsByAgentResponse {
    ErrorCode error_code;
    std::vector<std::string> tags;
};
struct AddAgentToPoolRequest {
    User user;
    std::string endpoint;
    std::string pool;
};
struct AddAgentToPoolResponse {
    ErrorCode error_code;
};
struct RemoveAgentFromPoolRequest {
    User user;
    std::string endpoint;
};
struct RemoveAgentFromPoolResponse {
    ErrorCode error_code;
};
struct ListAgentsByPoolRequest {
    User user;
    std::string pool;
};
struct ListAgentsByPoolResponse {
    ErrorCode error_code;
    std::vector<std::string> endpoint;
};
struct GetPoolByAgentRequest {
    User user;
    std::string endpoint;
};
struct GetPoolByAgentResponse {
    ErrorCode error_code;
    std::string pool;
};
struct AddUserRequest {
    User admin;
    User user;
};
struct AddUserResponse {
    ErrorCode error_code;
};
struct RemoveUserRequest {
    User admin;
    User user;
};
struct RemoveUserResponse {
    ErrorCode error_code;
};
struct ListUsersRequest {
    User user;
};
struct ListUsersResponse {
    ErrorCode error_code;
    std::vector<std::string> user;
};
struct ShowUserRequest {
    User admin;
    User user;
};
struct ShowUserResponse {
    ErrorCode error_code;
    std::vector<std::string> pools;
    std::vector<Authority> authority;
    Quota quota;
    Quota assigned;
};
struct GrantUserRequest {
    User admin;
    Grant grant;
};
struct GrantUserResponse {
    ErrorCode error_code;
};
struct AssignQuotaRequest {
    User admin;
    User user;
    Quota quota;
};
struct AssignQuotaResponse {
    ErrorCode error_code;
};
struct CreateContainerGroupRequest {
    User user;
    uint32_t replica;
    std::string name;
    ContainerDescription desc;
};
struct CreateContainerGroupResponse {
    ErrorCode error_code;
    std::string id;
};
struct RemoveContainerGroupRequest {
    User user;
    std::string id;
};
struct RemoveContainerGroupResponse {
    ErrorCode error_code;
};
struct UpdateContainerGroupRequest {
    User user;
    uint32_t replica;
    std::string id;
    uint32_t interval;
    ContainerDescription desc;
};
struct UpdateContainerGroupResponse {
    ErrorCode error_code;
};
struct ListContainerGroupsRequest {
    User user;
};
struct ContainerGroupStatistics {
    std::string id;
    uint32_t replica;
    uint32_t ready;
    uint32_t pending;
    uint32_t destroying;
    Resource cpu;
    Resource memroy;
    VolumResource volum;
    int64_t submit_time;
    int64_t update_time;
};
struct ListContainerGroupsResponse {
    ErrorCode error_code;
    std::vector<ContainerGroupStatistics> containers;
};
struct ShowContainerGroupRequest {
    User user;
    std::string id;
};
struct ContainerStatistics {
    ContainerStatus status;
    std::string endpoint;
    Resource cpu;
    Resource memroy;
    VolumResource volum;
};
struct ShowContainerGroupResponse {
    ErrorCode error_code;
    ContainerDescription desc;
    std::vector<ContainerStatistics> containers;
};
struct SubmitJobRequest {
    User user;
    JobDescription job;
    std::string hostname;
};
struct SubmitJobResponse {
    ErrorCode error_code;
    std::string jobid;
};
struct UpdateJobRequest {
    User user;
    std::string jobid;
    std::string hostname;
    JobDescription job;
};
struct UpdateJobResponse {
    ErrorCode error_code;
};
struct RemoveJobRequest {
    std::string jobid;
    std::string hostname;
};
struct RemoveJobResponse {
    ErrorCode error_code;
};
struct ListJobsRequest {
    User user;
};

struct JobOverview {
    JobDescription desc;
    std::string jobid;
    JobStatus status;
    int32_t running_num;
    int32_t pending_num;
    int32_t deploying_num;
    int32_t death_num;
    int32_t fail_count;
    int64_t create_time;
    int64_t update_time;
};

struct ListJobsResponse {
    ErrorCode error_code;
    std::vector<JobOverview> jobs;
};
struct ShowJobRequest {
    User user;
    std::string jobid;
};

struct PodInfo {
    std::string podid;
    std::string jobid;
    std::string endpoint;
    PodStatus status;
    std::string version;
    int64_t start_time;
    int32_t fail_count;
};
struct JobInfo {
    std::string jobid;
    JobDescription desc;
    std::vector<PodInfo> pods;
    JobStatus status;
    std::string version;
    int64_t create_time;
    int64_t update_time;
};
struct ShowJobResponse {
    ErrorCode error_code;
    JobInfo job;
};
struct ExecuteCmdRequest {
    std::string jobid;
    std::string cmd;
};
struct ExecuteCmdResponse {
    ErrorCode error_code;
};

struct StopJobRequest {
    std::string jobid;
};
struct StopJobResponse {
    ErrorCode error_code;
};

class ResourceManager {
public:
    explicit ResourceManager(const std::string& nexus_root) ;
    ~ResourceManager();
    bool Login(const std::string& user, const std::string& password);
    bool EnterSafeMode(const EnterSafeModeRequest& request, EnterSafeModeResponse* response);
    bool LeaveSafeMode(const LeaveSafeModeRequest& request, LeaveSafeModeResponse* response);
    bool Status(const StatusRequest& request, StatusResponse* response);
    bool CreateContainerGroup(const CreateContainerGroupRequest& request, CreateContainerGroupResponse* response);
    bool RemoveContainerGroup(const RemoveContainerGroupRequest& request, RemoveContainerGroupResponse* response);
    bool UpdateContainerGroup(const UpdateContainerGroupRequest& request, UpdateContainerGroupResponse* response);
    bool ListContainerGroups(const ListContainerGroupsRequest& request, ListContainerGroupsResponse* response);
    bool ShowContainerGroup(const ShowContainerGroupRequest& request, ShowContainerGroupResponse* response);
    bool AddAgent(const AddAgentRequest& request, AddAgentResponse* response);
    bool RemoveAgent(const RemoveAgentRequest& request, RemoveAgentResponse* response);
    bool OnlineAgent(const OnlineAgentRequest& request, OnlineAgentResponse* response);
    bool OfflineAgent(const OfflineAgentRequest& request, OfflineAgentResponse* response);
    bool ListAgents(const ListAgentsRequest& request, ListAgentsResponse* response);
    bool CreateTag(const CreateTagRequest& request, CreateTagResponse* response);
    bool ListTags(const ListTagsRequest& request, ListTagsResponse* response);
    bool ListAgentsByTag(const ListAgentsByTagRequest& request, ListAgentsByTagResponse* response);
    bool GetTagsByAgent(const GetTagsByAgentRequest& request, GetTagsByAgentResponse* response);
    bool AddAgentToPool(const AddAgentToPoolRequest& request, AddAgentToPoolResponse* response);
    bool RemoveAgentFromPool(const RemoveAgentFromPoolRequest& request, RemoveAgentFromPoolResponse* response);
    bool ListAgentsByPool(const ListAgentsByPoolRequest& request, ListAgentsByPoolResponse* response);
    bool GetPoolByAgent(const GetPoolByAgentRequest& request, GetPoolByAgentResponse* response);
    bool AddUser(const AddUserRequest& request, AddUserResponse* response);
    bool RemoveUser(const RemoveUserRequest& request, RemoveUserResponse* response);
    bool ListUsers(const ListUsersRequest& request, ListUsersResponse* response);
    bool ShowUser(const ShowUserRequest& request, ShowUserResponse* response);
    bool GrantUser(const GrantUserRequest& request, GrantUserResponse* response);
    bool AssignQuota(const AssignQuotaRequest& request, AssignQuotaResponse* response);
private:
    RpcClient* rpc_client_;
    std::string nexus_root_;
};

class AppMaster {
public:
    explicit AppMaster(const std::string& nexus_root);
    ~AppMaster();
    bool SubmitJob(const SubmitJobRequest& request, SubmitJobResponse* response);
    bool UpdateJob(const UpdateJobRequest& request, UpdateJobResponse* response);
    bool StopJob(const StopJobRequest& request, StopJobResponse* response);
    bool RemoveJob(const RemoveJobRequest& request, RemoveJobResponse* response);
    bool ListJobs(const ListJobsRequest& request, ListJobsResponse* response);
    bool ShowJob(const ShowJobRequest& request, ShowJobResponse* response);
    bool ExecuteCmd(const ExecuteCmdRequest& request, ExecuteCmdResponse* response);
private:
    RpcClient* rpc_client_;
    std::string nexus_root_;
};

} //namespace sdk
} //namespace galaxy
} //namespace baidu
