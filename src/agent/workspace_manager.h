// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#ifndef AGENT_WORKSPACE_MANAGER_H
#define AGENT_WORKSPACE_MANAGER_H
#include <map>
#include <string>
#include <stdint.h>
#include "agent/workspace.h"
#include "common/mutex.h"
#include "proto/task.pb.h"
namespace galaxy{

class WorkspaceManager{
public:
    WorkspaceManager(const std::string & root_path)
        : m_mutex(NULL), 
          m_root_path(root_path),
          m_data_path() {
        m_mutex = new common::Mutex();
    }
    ~WorkspaceManager(){
        delete m_mutex;
        Workspace * tmp = NULL;
        std::map<int64_t, DefaultWorkspace*>::iterator map_it = m_workspace_map.begin();
        for(;map_it != m_workspace_map.end();++map_it){
            tmp = map_it->second;
            tmp->Clean();
            m_workspace_map.erase(map_it);
            if(tmp != NULL){
                delete tmp;
            }
        }
    }
    bool Init();
    //根据task info 创建一个workspace
    int Add(const TaskInfo &task_info);
    int Remove(int64_t  task_info_id);
    DefaultWorkspace* GetWorkspace(const ::galaxy::TaskInfo &task_info);
private:
    std::map<int64_t, DefaultWorkspace*>  m_workspace_map;
    common::Mutex * m_mutex;
    std::string m_root_path;
    std::string m_data_path;
};

}
#endif /* !AGENT_WORKSPACE_MANAGER_H */
