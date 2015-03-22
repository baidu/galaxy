/*
 * workspace.cc
 * Copyright (C) 2015 wangtaize <wangtaize@baidu.com>
 *
 * Distributed under terms of the MIT license.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <unistd.h>
#include "agent/workspace.h"

namespace galaxy{
namespace agent{

int DefaultWorkspace::Create(){
    if(m_has_created){
        return 0;
    }
    m_mutex->Lock();
    if(m_has_created){
        m_mutex->Unlock();
        return 0;
    }
    //TODO safe path join
    std::stringstream ss;
    ss << m_root_path << "/" << m_task_info->task_name();
    m_task_root_path = ss.str();
    int status ;
    status = mkdir(m_task_root_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(status == 0 ){
        m_has_created = true;
    }
    m_mutex->Unlock();
    return status;
}

std::string DefaultWorkspace::GetPath(){
    return m_task_root_path;
}

int DefaultWorkspace::Clean(){
    if(!m_has_created){
        return 0;
    }
    m_mutex->Lock();
    if(!m_has_created){
        m_mutex->Unlock();
        return 0;
    }
    struct stat st;
    int ret = 0;
    if (stat(m_task_root_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)){
        std::string rm_cmd ;
        rm_cmd = "rm -rf " + m_task_root_path;
        ret = system(rm_cmd.c_str());
        if (ret == 0 ){
            m_has_created = false;
        }
    }
    m_mutex->Unlock();
    return ret;
}
bool DefaultWorkspace::IsExpire(){
    return false;
}
}
}



