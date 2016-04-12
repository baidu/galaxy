[![Build Status](https://travis-ci.org/baidu/galaxy.svg?branch=galaxy3)](https://travis-ci.org/baidu/galaxy)

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
        2. 服务管理层由AppMaster和AppWorker构成;

 
           +-------------------+-----------------------------+
           |                   |               |             |
           |                   |   MapReduce   |   Spark     |
           |                   |               |             |
           |                   +-----------------------------+
           |                                                 |
           |               Service Management                |
           |                                                 |
           +-------------------------------------------------+
           |                                                 |
           |               Resource Management               |
           |                                                 |
           +-------------------------------------------------+

## 1. 资源管理层（Resource Management)
组件： ResMan + Agents  
一个Galaxy集群只有一个处于工作状态的ResMan，负责容器的调度，为每个容器找到满足部署资源要求的机器；  
ResMan通过和部署在各个机器上的Agent通信，来创建和销毁容器；  
容器： 一个基于linux cgroup和namspace技术的资源隔离环境；   
容器里默认会启动AppWorker进程，是容器内的第一个进程，也就是根进程；   
ResMan不暴露给普通用户接口， 仅供内部组件以及集群管理员使用；  

## 2. 服务管理层 (Service Management)
组件：　AppMaster + AppWorkers  
AppMaster是外界用户操作Galaxy的唯一入口；  
一个Galaxy集群通常只有一个AppMaster，负责服务的部署、更新、启停和状态管理，把服务实例分发到各个机器上的容器内启动并跟踪状态；  
AppMaster通过调用ResMan的RPC接口创建容器，然后容器内拉起AppWorker进程；  
容器内的AppWorker进程通过和AppMaster进程通信，获得需要在容器内执行的命令，包括部署、启停、更新等等；  
AppWorker会汇报服务的状态给AppMaster，例如该实例是否在运行，进程退出码等；  

## 调度逻辑

用户提交的Job内容主要是两部分：资源需求 +　程序部署启停命令
1. Galaxy客户端通过Nexus找到AppMaster的地址；  
2. AppMaster收到Job提交请求后，把资源需求转发给ResMan， 创建相应的容器；  
3. AppWorkers向AppMaster索要命令，然后再容器内部署、启动服务；     
4. 当服务的状态发生变化后，AppWorker立即汇报给AppMaster;

## 容错

1. ResMan有备份，通过Nexus抢锁来Standby;  
2. Agent跟踪每个容器的状态汇报给ResMan，当容器个数不够或者不符合ResMan的要求时，就需要调度：创建或删除容器；  
3. AppWorker负责跟踪用户程序的状态，当用户程序coredump、异常退出或者被cgroup kill后，反馈状态给AppMaster，AppMaster根据指定策略命令AppWorker是否再次拉起。   

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

