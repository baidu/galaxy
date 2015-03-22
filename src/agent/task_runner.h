#ifndef AGENT_TASK_RUNNER_H
#define AGENT_TASK_RUNNER_H
#include "proto/master.pb.h"
#include "common/mutex.h"

namespace galaxy{
namespace agent{

class TaskRunner{

public:
   /**
    * start task
    * */
   virtual int Start() = 0 ;
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

class CommandTaskRunner:public TaskRunner{

public:
    CommandTaskRunner(TaskInfo * _task_info):m_task_info(_task_info),m_child_pid(-1){
        m_mutex = new Mutex();
    }
    ~CommandTaskRunner(){
        delete m_mutex;
    }
    int Start();
    int Stop();
    int IsRunning();
private:
    TaskInfo * m_task_info;
    pid_t  m_child_pid;
    Mutex * m_mutex;
};


}//agent
}//galaxy

#endif /* !AGENT_TASK_RUNNER_H */
