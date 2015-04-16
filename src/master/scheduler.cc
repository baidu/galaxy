// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com
// Date  : 2015-04-14
#include "master/scheduler.h"

#include <iostream>
#include <sstream>
#include "common/logging.h"

namespace galaxy{

SqliteScheduler::SqliteScheduler():
    db_(NULL){}

int SqliteScheduler::Init(){
    MutexLock lock(&mutex_);
    LOG(INFO,"init scheduler sqlit db");
    if(db_ != NULL ){
        return 0;
    }
    int ret = sqlite3_open(":memory:", &db_);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to open memory db ret %d",ret);
        return -1;
    }

    std::string create_agent_load = "BEGIN;"\
                                   "CREATE TABLE agent_load ("\
                                   "agent_id INTEGER PRIMARY KEY  NOT NULL,"\
                                   "mem_total INTEGER NOT NULL,"\
                                   "mem_left INTEGER NOT NULL,"\
                                   "cpu_total NUMERIC NOT NULL,"\
                                   "cpu_left NUMERIC NOT NULL,"\
                                   "load NUMERIC NOT NULL);"\
                                   "CREATE INDEX resource_index ON agent_load(mem_left,cpu_left);"\
                                   "CREATE INDEX load_index ON agent_load(load);"\
                                   "COMMIT;";
    std::string create_agent_used_port = "BEGIN;"\
                                         "CREATE TABLE agent_used_port ("\
                                         "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"\
                                         "agent_id INTEGER NOT NULL,"\
                                         "port INTEGER NOT NULL);"\
                                         "CREATE INDEX agent_id_index ON agent_used_port(agent_id);"\
                                         "COMMIT;";
    char * err = 0;
    ret = sqlite3_exec(db_,create_agent_load.c_str(),0,0,&err);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to create table agent load for %s",err);
        return -1;
    }
    ret = sqlite3_exec(db_,create_agent_used_port.c_str(),0,0,&err);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to create table agent used port for %s",err);
        return -1;
    }
    return 0;

}
void SqliteScheduler::Save(const AgentLoad& load){
    std::stringstream ss ;
    ss <<
}

void SqliteScheduler::Delete(int64_t agent_id){

}

int64_t SqliteScheduler::Schedule(int64_t mem_limit,double cpu_limit,
                                      const std::vector<int32_t>& ports){
       return -1;
}


bool SqliteScheduler::IsPortFit(const std::map<int32_t,int32_t>& used_ports,
               std::vector<int32_t> ports){

    return true;
}



}



