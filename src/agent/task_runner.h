// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#ifndef AGENT_TASK_RUNNER_H
#define AGENT_TASK_RUNNER_H
#include "proto/task.pb.h"
#include "common/mutex.h"
#include "agent/workspace.h"
namespace galaxy{

class TaskRunner{

public:

   virtual int Prepare() = 0 ;
   /**
    * start task
    * */
   virtual int Start() = 0 ;

   /**
    * restart
    * */
   virtual int ReStart() = 0 ;
   /**
    *stop task
    * */
   virtual int Stop() = 0 ;

   /**
    * check task
    * */
   virtual int IsRunning() = 0 ;

   virtual ~TaskRunner(){}
};
class AbstractTaskRunner:public TaskRunner{

public:
    AbstractTaskRunner(TaskInfo task_info,
                       DefaultWorkspace * workspace)
                       :m_task_info(task_info),
                       m_workspace(workspace),
                       m_has_retry_times(0){}
    virtual int Prepare() = 0;
    virtual int Start() = 0;
    int IsRunning();
    int Stop();
protected:
    void PrepareStart(std::vector<int>& fd_vector,int* stdout_fd,int* stderr_fd);
    void StartTaskAfterFork(std::vector<int>& fd_vector,int stdout_fd,int stderr_fd);
protected:
    TaskInfo m_task_info;
    //task parent pid
    pid_t  m_child_pid;
    pid_t  m_group_pid;
    int m_has_retry_times;
    DefaultWorkspace * m_workspace;
};

class CommandTaskRunner:public AbstractTaskRunner{

public:
    CommandTaskRunner(TaskInfo _task_info,
                      DefaultWorkspace * _workspace)
                      :AbstractTaskRunner(_task_info,_workspace){
    }

    ~CommandTaskRunner() {
    }
    int Prepare(){
        return 0;
    }
    int Start();
    int ReStart();
};


}//galaxy

#endif /* !AGENT_TASK_RUNNER_H */
