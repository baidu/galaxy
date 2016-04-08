[![Build Status](https://travis-ci.org/fxsjy/solar.svg?branch=master)](https://travis-ci.org/fxsjy/solar)
[![Build Status](https://travis-ci.org/haolifei/solar.svg?branch=master)](https://travis-ci.org/haolifei/solar)

galaxy 3.0

Galaxy 3.0设计
=============

# 背景

Galaxy3.0是对Galaxy2.0的重构，主要解决以下问题：  

1. 容器管理和服务管理紧耦合：服务的升级和启停都伴随容器的销毁和调度；
2. 没有磁盘管理，只能管理home盘；
3. 不支持用户quota和记账；
4. 机器管理功能缺失；
5. Naming功能可用性低；
6. Trace功能不完善；

# 系统架构

        Galaxy3.0架构上分为2层： 资源管理层和服务管理层，每层都是主从式架构  
        1. 资源管理层由ResMan(Resource Manager)和Agent构成  
        2. 服务管理层由AppMaster和AppWorker构成, 每个服务有自己的AppMaster进程

 
                       Naming _                       Job_A1          Job_A2
                                \                       |               |
                                 \__ AppMaster_A <--> AppWorker_A1   AppWorker_A2
                                         |              |               |
         +----------------------------------------------------------------------+
         |  ResMan  <--------------->  Agent1         Agent2      Agent3 ....
         +----------------------------------------------------------------------+
                                         |              |               |
                                     AppMaster_B <--> AppWorker_B1   AppWorker_B2
                                                        |               |
                                                      Job_B1          Job_B2
上面的图有点抽象，可以举一个不太恰当的例子。

> Galaxy就是一个五星级酒店，每天都有大量游客组团入住和离开。  
> ResMan就是大堂经理，负责按照每个团的人数、房间需求来分配房间，以及记账结账；  
> Agent就是酒店服务员，确保房间就绪，以及游客离开后打扫等；  
> AppMaster是旅游团团长，确保每个游客不掉队，以及组织一些集体活动；  
> AppWorker就是小导游，负责把游客引导到房间，并且跟进游客的状态，汇报给团长；  

## 调度逻辑

用户提交的Job内容主要是两部分：资源需求 +　程序启停  
1. 一个Job提交后，就持久化到Nexus中。 ResMan只关心资源需求部分，ResMan命令有资源的Agent创建容器: 1个Master容器 + N个Worker容器；  
2. Agent在Master容器启动AppMaster进程， Agent在Worker容器启动AppWorker进程； AppMaster进程把自己的地址记录在Nexus上；  
3. Master容器的调度需要在有特殊Tag的机器上，这样确保AppMaster的稳定性；
4. AppWorker进程发RPC请求AppMaster进程， AppMaster分配任务给AppWorker执行， AppWorker反馈任务执行状态给AppMaster；（执行的任务包括：下载程序包、启动程序、停止程序、Load词典、更新程序版本等）

## 容错

1. ResMan有备份，通过Nexus抢锁来Standby;  
2. Agent跟踪每个容器的状态汇报给ResMan，当容器个数不够或者不符合ResMan的要求时，就需要调度：创建或删除容器；  
3. AppWorker负责跟踪用户程序的状态，当用户程序coredump、异常退出或者被cgroup kill后，反馈状态给AppMaster，AppMaster根据指定策略命令AppWorker是否再次拉起。  
4. AppMaster如果异常挂掉，或者所在机器挂掉，ResMan会销毁此容器，并且在其他Agent上创建一个Master容器；  

## 服务发现
1. SDK通过Nexus发现指定的Job的AppMaster地址；  
2. SDK请求AppMaster，发现每个Job实例的地址和当前的服务状态；  
3. AppMaster会定时同步服务地址和状态到第三方Naming系统（如BNS,ZK等);  

## 服务更新
1. SDK通过Nexus发现指定的Job的AppMaster地址；  
2. SDK请求AppMaster, AppMaster将服务更新命令传播给AppWorker, AppWorker将更新状态反馈给AppMaster;  
3. AppWorker和AppMaster的通信方式是Pull的方式，因此AppMaster可以根据当前的情况来决定部署的暂停和步长控制；  
4. 服务的更新都在容器内进行，不涉及到容器的销毁和创建

# 系统依赖
1. Nexus作为寻址和元信息保存  
2. MDT作为用户日志的Trace系统  
3. Sofa-PbRPC作为通信基础库  


