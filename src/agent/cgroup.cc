// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/cgroup.h"

#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "common/logging.h"
#include "common/util.h"
#include "common/this_thread.h"
#include "agent/downloader_manager.h"
#include "agent/resource_collector.h"
#include "agent/resource_collector_engine.h"
#include "agent/utils.h"

extern std::string FLAGS_cgroup_root; 

namespace galaxy {

static const std::string RUNNER_META_PREFIX = "task_runner_";
static int CPU_CFS_PERIOD = 100000;
static int MIN_CPU_CFS_QUOTA = 1000;

int CGroupCtrl::Create(int64_t task_id, std::map<std::string, std::string>& sub_sys_map) {
    if (_support_cg.size() <= 0) {
        LOG(WARNING, "no subsystem is support");
        return -1;
    }

    std::vector<std::string>::iterator it = _support_cg.begin();

    for (; it != _support_cg.end(); ++it) {
        std::stringstream ss ;
        ss << _cg_root << "/" << *it << "/" << task_id;
        int status = mkdir(ss.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if (status != 0) {
            if (errno == EEXIST) {
                // TODO
                LOG(WARNING, "cgroup already there");
            } else {
                LOG(FATAL, "fail to create subsystem %s ,status is %d", ss.str().c_str(), status);
                return status;
            }
        }

        sub_sys_map[*it] = ss.str();
        LOG(INFO, "create subsystem %s successfully", ss.str().c_str());
    }

    return 0;
}


//目前不支持递归删除
//删除前应该清空tasks文件
int CGroupCtrl::Destroy(int64_t task_id) {
    if (_support_cg.size() <= 0) {
        LOG(WARNING, "no subsystem is support");
        return -1;
    }

    std::vector<std::string>::iterator it = _support_cg.begin();

    for (; it != _support_cg.end(); ++it) {
        std::stringstream ss ;
        ss << _cg_root << "/" << *it << "/" << task_id;
        int status = rmdir(ss.str().c_str());
        if(status != 0 && errno != ENOENT){
            LOG(FATAL,"fail to delete subsystem %s status %d err[%d: %s]",
                    ss.str().c_str(),
                    status, 
                    errno,
                    strerror(errno));
            return status;
        }
    }

    return 0;
}

int AbstractCtrl::AttachTask(pid_t pid) {
    std::string task_file = _my_cg_root + "/" + "tasks";
    int ret = common::util::WriteIntToFile(task_file, pid);

    if (ret < 0) {
        LOG(FATAL, "fail to attach pid  %d for %s", pid, _my_cg_root.c_str());
        return -1;
    }else{
        LOG(INFO,"attach pid %d for %s successfully",pid, _my_cg_root.c_str());
    }

    return 0;
}

int MemoryCtrl::SetLimit(int64_t limit) {
    std::string limit_file = _my_cg_root + "/" + "memory.limit_in_bytes";
    int ret = common::util::WriteIntToFile(limit_file, limit);

    if (ret < 0) {
        LOG(FATAL, "fail to set limt %lld for %s", limit, _my_cg_root.c_str());
        return -1;
    }

    return 0;
}

int MemoryCtrl::SetSoftLimit(int64_t soft_limit) {
    std::string soft_limit_file = _my_cg_root + "/" + "memory.soft_limit_in_bytes";
    int ret = common::util::WriteIntToFile(soft_limit_file, soft_limit);

    if (ret < 0) {
        LOG(FATAL, "fail to set soft limt %lld for %s", soft_limit, _my_cg_root.c_str());
        return -1;
    }

    return 0;
}


int CpuCtrl::SetCpuShare(int64_t cpu_share) {
    std::string cpu_share_file = _my_cg_root + "/" + "cpu.shares";
    int ret = common::util::WriteIntToFile(cpu_share_file, cpu_share);
    if (ret < 0) {
        LOG(FATAL, "fail to set cpu share %lld for %s", cpu_share, _my_cg_root.c_str());
        return -1;
    }

    return 0;

}
int CpuCtrl::SetCpuPeriod(int64_t cpu_period) {
    std::string cpu_period_file = _my_cg_root + "/" + "cpu.cfs_period_us";
    int ret = common::util::WriteIntToFile(cpu_period_file, cpu_period);
    if (ret < 0) {
        LOG(FATAL, "fail to set cpu period %lld for %s", cpu_period, _my_cg_root.c_str());
        return -1;
    }

    return 0;

}

int CpuCtrl::SetCpuQuota(int64_t cpu_quota) {
    std::string cpu_quota_file = _my_cg_root + "/" + "cpu.cfs_quota_us";
    int ret = common::util::WriteIntToFile(cpu_quota_file, cpu_quota);
    if (ret < 0) {
        LOG(FATAL, "fail to set cpu quota  %ld for %s", cpu_quota, _my_cg_root.c_str());
        return -1;
    }
    LOG(INFO, "set cpu quota %ld for %s", cpu_quota, _my_cg_root.c_str());
    return 0;

}

ContainerTaskRunner::~ContainerTaskRunner() {
    if (collector_ != NULL) {
        ResourceCollectorEngine* engine
            = GetResourceCollectorEngine();
        engine->DelCollector(collector_id_);
        delete collector_;
        collector_ = NULL;
    }
    delete _cg_ctrl;
    delete _mem_ctrl;
    delete _cpu_ctrl;
    delete _cpu_acct_ctrl;
}

int ContainerTaskRunner::Prepare() {
    LOG(INFO, "prepare container for task %ld", m_task_info.task_id());
    //TODO
    std::vector<std::string> support_cg;
    support_cg.push_back("memory");
    support_cg.push_back("cpu");
    support_cg.push_back("cpuacct");
    _cg_ctrl = new CGroupCtrl(_cg_root, support_cg);
    std::map<std::string, std::string> sub_sys_map;
    int status = _cg_ctrl->Create(m_task_info.task_id(), sub_sys_map);
    // NOTE multi thread safe
    if (collector_ == NULL) {
        std::string group_path = boost::lexical_cast<std::string>(m_task_info.task_id());
        collector_ = new CGroupResourceCollector(group_path);
        ResourceCollectorEngine* engine
                = GetResourceCollectorEngine();
        collector_id_ = engine->AddCollector(collector_);
    }
    else {
        collector_->ResetCgroupName(
                boost::lexical_cast<std::string>(m_task_info.task_id()));
    }

    if (status != 0) {
        LOG(FATAL, "fail to create subsystem for task %ld,status %d", m_task_info.task_id(), status);
        return status;
    }

    _mem_ctrl = new MemoryCtrl(sub_sys_map["memory"]);
    _cpu_ctrl = new CpuCtrl(sub_sys_map["cpu"]);
    _cpu_acct_ctrl = new CpuAcctCtrl(sub_sys_map["cpuacct"]);

    std::string uri = m_task_info.task_raw();
    std::string path = m_workspace->GetPath();
    path.append("/");
    path.append("tmp.tar.gz");

    DownloaderManager* downloader_handler = DownloaderManager::GetInstance();
    downloader_handler->DownloadInThread(
            uri,
            path,
            boost::bind(&ContainerTaskRunner::StartAfterDownload, this, _1));
    return 0;
}

void ContainerTaskRunner::StartAfterDownload(int ret) {
    if (ret == 0) {
        std::string tar_cmd = "cd " + m_workspace->GetPath() + " && tar -xzf tmp.tar.gz";
        int status = system(tar_cmd.c_str());
        if (status != 0) {
            LOG(WARNING, "tar -xf failed");
            return;
        }
        Start();
    }
}

void ContainerTaskRunner::PutToCGroup(){
    int64_t mem_size = m_task_info.required_mem();
    double cpu_core = m_task_info.required_cpu();
    LOG(INFO, "resource limit cpu %f, mem %ld", cpu_core, mem_size);
    /*
    std::string mem_key = "memory";
    std::string cpu_key = "cpu";
    for (int i = 0; i< m_task_info.resource_list_size(); i++){
        ResourceItem item = m_task_info.resource_list(i);
        if(mem_key.compare(item.name())==0 && item.value() > 0){
            mem_size = item.value();
            continue;
        }
        if(cpu_key.compare(item.name())==0 && item.value() >0){
            cpu_share = item.value() * 512;
        }

    }*/
    _mem_ctrl->SetLimit(mem_size);
    //_cpu_ctrl->SetCpuShare(cpu_share);
    int64_t quota = static_cast<int64_t>(cpu_core * CPU_CFS_PERIOD);
    if (quota < MIN_CPU_CFS_QUOTA) {
        quota = MIN_CPU_CFS_QUOTA;
    }
    _cpu_ctrl->SetCpuQuota(quota);
    pid_t my_pid = getpid();
    _mem_ctrl->AttachTask(my_pid);
    _cpu_ctrl->AttachTask(my_pid);
    _cpu_acct_ctrl->AttachTask(my_pid);
}

int ContainerTaskRunner::Start() {
    LOG(INFO, "start a task with id %ld", m_task_info.task_id());

    if (IsRunning() == 0) {
        LOG(WARNING, "task with id %ld has been runing", m_task_info.task_id());
        return -1;
    }

    int stdout_fd, stderr_fd;
    std::vector<int> fds;
    PrepareStart(fds, &stdout_fd, &stderr_fd);
    //sequence_id_ ++;
    m_child_pid = fork();

    if (m_child_pid == 0) {
        pid_t my_pid = getpid();
        int ret = setpgid(my_pid, my_pid);
        if (ret != 0) {
            assert(0);
        }
        std::string meta_file = persistence_path_dir_ 
            + "/" + RUNNER_META_PREFIX 
            + boost::lexical_cast<std::string>(sequence_id_);
        int meta_fd = open(meta_file.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
        if (meta_fd == -1) {
            assert(0);
        }
        int64_t value = my_pid;
        LOG(WARNING, "value = %d", my_pid);
        int len = write(meta_fd, (void*)&value, sizeof(value));
        if (len == -1) {
            close(meta_fd);
            assert(0);
        }
        value = m_task_info.task_id();
        len = write(meta_fd, (void*)&value, sizeof(value));
        if (len == -1) {
            close(meta_fd); 
            assert(0);
        }
        if (0 != fsync(meta_fd)) {
            close(meta_fd); 
            assert(0);
        }
        close(meta_fd);

        PutToCGroup();
        StartTaskAfterFork(fds, stdout_fd, stderr_fd);
    } else {
        close(stdout_fd);
        close(stderr_fd);
        m_group_pid = m_child_pid;
    }
    return 0;
}

void ContainerTaskRunner::Status(TaskStatus* status) {
    if (collector_ != NULL) {
        status->set_cpu_usage(collector_->GetCpuUsage());
        status->set_memory_usage(collector_->GetMemoryUsage());
        LOG(WARNING, "cpu usage %f memory usage %ld",
                status->cpu_usage(), status->memory_usage());
    }
    return;
}

void ContainerTaskRunner::StopPost() {
    if (collector_ != NULL) {
        collector_->Clear();
    }
    return;
}

int ContainerTaskRunner::Stop(){
    int status = AbstractTaskRunner::Stop();
    LOG(INFO,"stop  task %ld  with status %d",m_task_info.task_id(),status);
    if(status != 0 ){
        return status;
    }
    //sleep 500 ms for cgroup clear tasks
    common::ThisThread::Sleep(500);
    StopPost();
    status = _cg_ctrl->Destroy(m_task_info.task_id());
    LOG(INFO,"destroy cgroup for task %ld with status %d",m_task_info.task_id(),status);
    return status;
}

bool ContainerTaskRunner::RecoverRunner(const std::string& persistence_path) {
    std::vector<std::string> files;
    if (!GetDirFilesByPrefix(
                persistence_path,
                RUNNER_META_PREFIX,
                &files)) {
        LOG(WARNING, "get meta files failed"); 
        return false;
    }

    if (files.size() == 0) {
        return true; 
    }

    int max_seq_id = -1;
    std::string last_meta_file;
    for (size_t i = 0; i < files.size(); i++) {
        std::string file = files[i]; 
        size_t pos = file.find(RUNNER_META_PREFIX);
        if (pos == std::string::npos) {
            continue; 
        }

        if (pos + RUNNER_META_PREFIX.size() >= file.size()) {
            LOG(WARNING, "meta file format err %s", file.c_str()); 
            continue;
        }

        int cur_id = atoi(file.substr(
                    pos + RUNNER_META_PREFIX.size()).c_str());
        if (max_seq_id < cur_id) {
            max_seq_id = cur_id; 
            last_meta_file = file;
        }
    }

    if (max_seq_id < 0) {
        return false; 
    }

    std::string meta_file = persistence_path + "/" + last_meta_file;
    LOG(DEBUG, "start to recover %s", meta_file.c_str());
    int fin = open(meta_file.c_str(), O_RDONLY);
    if (fin == -1) {
        LOG(WARNING, "open meta file failed %s err[%d: %s]", 
                meta_file.c_str(),
                errno,
                strerror(errno)); 
        return false;
    }

    size_t value;
    int len = read(fin, (void*)&value, sizeof(value));
    if (len == -1) {
        LOG(WARNING, "read meta file failed err[%d: %s]",
                errno,
                strerror(errno)); 
        close(fin);
        return false;
    }
    LOG(DEBUG, "recove gpid %lu", value);
    int ret = killpg((pid_t)value, 9);
    if (ret != 0 && errno != ESRCH) {
        LOG(WARNING, "fail to kill process group %lu", value); 
        return false;
    }

    value = 0;
    len = read(fin, (void*)&value, sizeof(value));
    if (len == -1) {
        LOG(WARNING, "read meta file failed err[%d: %s]",
                errno,
                strerror(errno));
        close(fin);
        return false;
    }
    close(fin);

    std::vector<std::string> support_cgroup;  
    // TODO configable
    support_cgroup.push_back("memory");
    support_cgroup.push_back("cpu");
    support_cgroup.push_back("cpuacct");
    
    LOG(DEBUG, "destroy cgroup %lu", value);
    CGroupCtrl ctl(FLAGS_cgroup_root, support_cgroup);
    int max_retry_times = 10;
    ret = -1;
    while (max_retry_times-- > 0) {
        ret = ctl.Destroy(value);
        if (ret != 0) {
            common::ThisThread::Sleep(100);
            continue;
        }    
        break;
    }
    if (ret == 0) {
        LOG(DEBUG, "destroy cgroup %lu success", value);
        return true; 
    }
    return false;
}

}
