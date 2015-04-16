// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com
// Date  : 2015-04-13
#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H

#include <map>
#include <vector>
#include <string>
#include <sqlite3.h>
#include "common/mutex.h"

namespace galaxy{
struct AgentLoad{
    int64_t agent_id;
    int64_t mem_total;
    int64_t mem_left;
    double cpu_total;
    double cpu_left;
    double load;
    int32_t task_count;
    std::string addr;
};

struct AgentUsedPort{
    int64_t id;
    int64_t agent_id;
    int32_t port;
};



class Scheduler{
public:
    virtual int Init() = 0;
    virtual double CalcLoad(const AgentLoad& load) = 0;
    virtual int Save(const AgentLoad& load) = 0 ;
    virtual int Delete(int64_t agent_id) = 0;
    virtual int Schedule(int64_t mem_limit,double cpu_limit,
                         const std::vector<int32_t>& ports,
                         AgentLoad* load) = 0;

    virtual ~Scheduler(){}
};

class SqliteScheduler:public Scheduler{

public:
    SqliteScheduler();
    double CalcLoad(const AgentLoad& load);
    int Save(const AgentLoad& load);
    int Delete(int64_t agent_id);
    int Schedule(int64_t mem_limit,double cpu_limit,
                 const std::vector<int32_t>& ports,
                 AgentLoad* load);
    int Init();
    ~SqliteScheduler();
private:
    bool IsPortFit(const std::map<int32_t,int32_t>& used_ports,std::vector<int32_t> ports);
    bool Exists(int64_t agent_id);
    int Update(const AgentLoad& load);
    int Insert(const AgentLoad& load);
private:
    Mutex mutex_;
    sqlite3* db_;
};

}
#endif /* !MASTER_SCHEDULER_H */
