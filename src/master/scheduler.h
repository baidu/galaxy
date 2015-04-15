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
}

struct AgentUsedPort{
    int64_t id;
    int64_t agent_id;
    int32_t port;
}



class Scheduler{
public:
    virtual int Init() = 0;
    virtual void Save(const AgentLoad& load) = 0 ;
    virtual void Delete(int64_t agent_id) = 0;
    virtual int64_t Schedule(int64_t mem_limit,double cpu_limit,const std::vector<int32_t>& ports) = 0;
    virtual ~Scheduler(){}
};

class SqliteScheduler:public Scheduler{

public:
    SqliteScheduler();
    void Save(const AgentLoad& table);
    void Delete(int64_t agent_id);
    int64_t Schedule(int64_t mem_limit,double cpu_limit,const std::vector<int32_t>& ports);
    int Init();
    ~SqliteScheduler(){}
private:
    bool IsPortFit(const std::map<int32_t,int32_t>& used_ports,std::vector<int32_t> ports);
private:
    Mutex mutex_;
    sqlite3* db_;
};

}
#endif /* !MASTER_SCHEDULER_H */
