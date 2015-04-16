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


double SqliteScheduler::CalcLoad(const AgentLoad& load){
    if(load.mem_total == 0 || load.cpu_total == 0.0 ){
        LOG(FATAL,"invalid load input ,mem_total %ld,cpu_total %f",load.mem_total,load.cpu_total);
        return 0.0;
    }
    int64_t mem_used = 1;
    if(load.mem_left < load.mem_total){
        mem_used = load.mem_total - load.mem_left;
    }
    double cpu_used = 0.01;
    if(load.cpu_left < load.cpu_total){
        cpu_used = load.cpu_total - load.cpu_left;
    }
    //TODO mem cpu factor 能够加上调权参数
    double mem_factor = mem_used/static_cast<double>(load.mem_total);
    double cpu_factor = cpu_used/load.cpu_total;
    double task_count_factor = load.task_count == 0 ? 0.1:load.task_count;
    return task_count_factor * mem_factor * cpu_factor;
}

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
                                   "task_count INTEGER NOT NULL,"\
                                   "cpu_total NUMERIC NOT NULL,"\
                                   "cpu_left NUMERIC NOT NULL,"\
                                   "load NUMERIC NOT NULL,"\
                                   "addr TEXT NOT NULL);"\
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

int SqliteScheduler::Save(const AgentLoad& load){
    LOG(INFO,"save load %ld ",load.agent_id);
    if(Exists(load.agent_id)){
        return Update(load);
    }
    return Insert(load);
}


bool SqliteScheduler::Exists(int64_t agent_id){
    std::stringstream ss;
    ss << "select count(*) from agent_load where agent_id=" << agent_id <<";";
    sqlite3_stmt* statement;
    int count = 0;
    if (sqlite3_prepare_v2(db_, ss.str().c_str(), -1, &statement, 0 ) == SQLITE_OK ) {
        int res = sqlite3_step(statement);
        if(res == SQLITE_ROW){
             count = sqlite3_column_int64(statement,0);
        }
        sqlite3_finalize(statement);
    }
    return count == 0 ? false:true;
}


int SqliteScheduler::Insert(const AgentLoad& load){
    std::stringstream ss ;
    ss << "INSERT INTO agent_load(agent_id,mem_total,mem_left,task_count,cpu_total,cpu_left,load,addr) values("
       << load.agent_id <<","<< load.mem_total<<","<<load.mem_left<<","<<load.task_count<<","
       << load.cpu_total<<","<< load.cpu_left <<","<<CalcLoad(load) <<", '"<<load.addr <<"');";
    char * err = 0;
    int ret = sqlite3_exec(db_,ss.str().c_str(),0,0,&err);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to insert agent %ld load for %s",load.agent_id,err);
        return -1;
    }
    LOG(INFO,"insert agent %ld load successfully",load.agent_id);
    return 0;
}


int SqliteScheduler::Update(const AgentLoad& load){
    std::stringstream ss;
    ss <<"UPDATE agent_load set mem_total= "<<load.mem_total <<","
       <<"mem_left="<<load.mem_left <<","
       <<"cpu_total="<<load.cpu_total<<","
       <<"cpu_left="<<load.cpu_left <<","
       <<"task_count="<<load.task_count <<","
       <<"load="<<CalcLoad(load)<<","
       <<"addr='"<<load.addr<<"'"
       <<" WHERE agent_id="<<load.agent_id<<";";
    char * err = 0;
    int ret = sqlite3_exec(db_,ss.str().c_str(),0,0,&err);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to update agent %ld load for %s",load.agent_id,err);
        return -1;
    }
    LOG(INFO,"update agent %ld successfully ",load.agent_id);
    return 0;
}


int  SqliteScheduler::Delete(int64_t agent_id){
    std::stringstream ss;
    ss<<"DELETE FROM agent_load where agent_id="<<agent_id<<";";
    char * err = 0;
    int ret = sqlite3_exec(db_,ss.str().c_str(),0,0,&err);
    if(ret != SQLITE_OK){
        LOG(FATAL,"fail to delete agent %ld load for %s",agent_id,err);
        return -1;
    }
    LOG(INFO,"delete agent %ld successfully ",agent_id);
    return 0;
}



int SqliteScheduler::Schedule(int64_t mem_limit,double cpu_limit,
                                  const std::vector<int32_t>& /*ports*/,
                                  AgentLoad* load){
    LOG(INFO,"schedule for requirement mem_limit %ld, cpu_limit %0.2f",mem_limit,cpu_limit);
    std::stringstream query ;
    query << "select agent_id,addr from agent_load where mem_left >=" << mem_limit
          << " and cpu_left >= "<< cpu_limit << " order by load asc limit 1;";
    sqlite3_stmt* statement;
    int ret = -1;
    if (sqlite3_prepare_v2(db_, query.str().c_str(), -1, &statement, 0 ) == SQLITE_OK ) {
        int res = sqlite3_step(statement);
        if(res == SQLITE_ROW){
            load->agent_id = sqlite3_column_int64(statement,0);
            load->addr = (char *)sqlite3_column_text(statement,1);
            LOG(INFO,"scheduler choosed agent %d , addr %sfor requrement ",load->agent_id,load->addr.c_str());
            ret = 0;
        }
        sqlite3_finalize(statement);
    }
    return ret;
}


bool SqliteScheduler::IsPortFit(const std::map<int32_t,int32_t>& /*used_ports*/,
                                std::vector<int32_t> /*ports*/){

    return true;
}

SqliteScheduler::~SqliteScheduler(){
    if(NULL != db_){
        sqlite3_close_v2(db_);
    }

}

}



