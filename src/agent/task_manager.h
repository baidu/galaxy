/*
 * task_manager.h
 * Copyright (C) 2015 wangtaize <wangtaize@baidu.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef AGENT_TASK_MANAGER_H
#define AGENT_TASK_MANAGER_H
#include <vector>
#include "proto/master.pb.h"
#include "common/mutex"
namespace galaxy{
namespace agent{

class TaskManager{

public:
    TaskManager(){
        m_mutex = new Mutex();
    }
    ~TaskManager(){
        if(m_mutex != NULL){
            delete m_mutex;
        }
    }
    int Add(const TaskInfo &task_info);
    int Remove(const TaskInfo &task_info);
    int Status(std::vector<TaskStatus> * task_status_vector);
private:
    Mutex * m_mutex;
    std::vector<TaskRunner> * m_task_runner_vector;
};
}
}


#endif /* !AGENT_TASK_MANAGER_H */
