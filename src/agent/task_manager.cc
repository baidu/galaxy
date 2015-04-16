// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/task_manager.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <boost/lexical_cast.hpp>

#include "common/logging.h"
#include "agent/cgroup.h"

#include "agent/utils.h"

extern std::string FLAGS_container;
extern std::string FLAGS_cgroup_root;
extern std::string FLAGS_agent_work_dir;

namespace galaxy {

const std::string META_PATH = "/meta/";
const std::string META_FILE_PREFIX = "meta_";

bool TaskManager::Init() {
    const int MKDIR_MODE = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    m_task_meta_dir = FLAGS_agent_work_dir + "/" + META_PATH;
    if (access(m_task_meta_dir.c_str(), F_OK) != 0) {
        if (mkdir(m_task_meta_dir.c_str(), MKDIR_MODE) != 0) {
            LOG(WARNING, "mkdir data failed %s err[%d: %s]",
                    m_task_meta_dir.c_str(),
                    errno,
                    strerror(errno)); 
            return false;
        }         
        return true;
    }

    LOG(INFO, "recove meta data %s", m_task_meta_dir.c_str());
    // if meta_path exists do clear for process
    // clear by Runner Static Function 
    std::vector<std::string> meta_files;
    if (!GetDirFilesByPrefix(
                m_task_meta_dir, 
                META_FILE_PREFIX, 
                &meta_files)) {
        LOG(WARNING, "list directory files failed"); 
        return false;
    }
    for (size_t i = 0; i < meta_files.size(); i++) {
        LOG(DEBUG, "recove meta file %s", meta_files[i].c_str());
        std::string meta_file_name = m_task_meta_dir + "/" + meta_files[i];
        bool ret = false;
        if (FLAGS_container.compare("cgroup") == 0) {
            ret = ContainerTaskRunner::RecoverRunner(meta_file_name);     
        }
        else {
            ret = CommandTaskRunner::RecoverRunner(meta_file_name);            
        }
        if (!ret) {
            return false;
        }
        std::string rmdir = "rm -rf " + meta_file_name; 
        if (system(rmdir.c_str()) == -1) {
            LOG(WARNING, "rm meta failed rm %s err[%d: %s]",
                    rmdir.c_str(),
                    errno, strerror(errno)); 
            return false;
        }
    }
    return true;
}


int TaskManager::Add(const ::galaxy::TaskInfo& task_info,
                     DefaultWorkspace *  workspace) {
    MutexLock lock(m_mutex);
    LOG(INFO, "add task with id %d", task_info.task_id());
    if (m_task_runner_map.find(task_info.task_id()) != m_task_runner_map.end()) {
        LOG(WARNING, "task with id %d has exist", task_info.task_id());
        return 0;
    }
    // do download
    TaskInfo my_task_info(task_info);
    TaskRunner* runner = NULL;
    std::string persistence_path = FLAGS_agent_work_dir 
        + "/" + META_PATH 
        + "/" + META_FILE_PREFIX + boost::lexical_cast<std::string>(task_info.task_id());

    const int MKDIR_MODE = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    if (mkdir(persistence_path.c_str(), MKDIR_MODE) != 0) {
        LOG(WARNING, "mkdir data failed %s err[%d: %s]",
                persistence_path.c_str(),
                errno,
                strerror(errno));     
        return -1;
    }  
    if(FLAGS_container.compare("cgroup") == 0){
        LOG(INFO,"use cgroup task runner for task %d",task_info.task_id());
        runner = new ContainerTaskRunner(my_task_info,FLAGS_cgroup_root, workspace);
    }else{
        LOG(INFO,"use command task runner for task %d",task_info.task_id());
        runner = new CommandTaskRunner(my_task_info,workspace);
    }
    runner->PersistenceAble(persistence_path);
    int ret = runner->Prepare();
    if(ret != 0 ){
        LOG(INFO,"fail to prepare runner ,ret is %d",ret);
        return ret;
    }
    m_task_runner_map[task_info.task_id()] = runner;
    //ret = runner->Start();
    //if (ret == 0) {
    //    LOG(INFO, "add task with id %d successfully", task_info.task_id());
    //    m_task_runner_map[task_info.task_id()] = runner;
    //} else {
    //    LOG(FATAL, "fail to add task with %d", task_info.task_id());
    //}
    return ret;
}

int TaskManager::Remove(const int64_t& task_info_id) {
    MutexLock lock(m_mutex);
    if (m_task_runner_map.find(task_info_id) == m_task_runner_map.end()) {
        LOG(WARNING, "task with id %d does not exist", task_info_id);
        return 0;
    }
    TaskRunner* runner = m_task_runner_map[task_info_id];
    if(NULL == runner){
        return 0;
    }
    int status = runner->Stop();
    if(status == 0){
        LOG(INFO,"stop task %d successfully",task_info_id);
    }
    m_task_runner_map.erase(task_info_id);
    delete runner;
    return status;
}

int TaskManager::Status(std::vector< TaskStatus >& task_status_vector) {

    std::vector<int64_t> dels;
    {
        MutexLock lock(m_mutex);
        std::map<int64_t, TaskRunner*>::iterator it = m_task_runner_map.begin();
        for (; it != m_task_runner_map.end(); ++it) {
            TaskStatus status;
            it->second->Status(&status);
            //status.set_task_id(it->first);
            //int ret = it->second->IsRunning();
            //if(ret == 0){
            //    status.set_status(RUNNING);
            //}else if(ret == 1){
            //    status.set_status(COMPLETE);
            //    dels.push_back(it->first);
            //}else{
            //    if (it->second->ReStart() == 0) {
            //        status.set_status(RESTART);
            //    } else {
            //        // if restart failed,
            //        // 1. retry times more than limit, no need retry any more.
            //        // 2. stop failed
            //        // 3. start failed
            //        status.set_status(ERROR);
            //    }
            //}

            task_status_vector.push_back(status);
        }
    }
    for (uint32_t i = 0; i < dels.size(); i++) {
        Remove(dels[i]);
    }
    return 0;
}

}



