/*
 * executor.cc
 * Copyright (C) 2015 wangtaize <wangtaize@baidu.com>
 *
 * Distributed under terms of the MIT license.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "agent/task_runner.h"
#include "common/logging.h"
namespace galaxy{
namespace agent{


//check process with m_child_pid 
int CommandTaskRunner::IsRunning(){
    if(m_child_pid == -1){
        return -1;
    }
    int ret = ::kill(m_child_pid,0);
    return ret;
}

//start process
//1. fork a subprocess A
//2. exec command in the subprocess A 
//TODO add workspace
int CommandTaskRunner::Start(){
    if (m_child_pid != -1){
        return -1;
    }
    m_mutex->Lock("start task lock");
    if (m_child_pid != -1){
        m_mutex->Unlock();
        return -1;
    }
    m_child_pid = fork();
    m_mutex->Unlock();
    if(m_child_pid != 0){
        return -1;
    }
    FILE *fd = popen(m_task_info->cmd_line().c_str(),"r");
    pclose(fd);
    return 0;
}


int CommandTaskRunner::Stop(){
    if(IsRunning()!=0){
        return 0;
    }

}

}
}



