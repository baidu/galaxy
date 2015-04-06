# Galaxy
数据中心管理系统，设计用来管理数据中心的数万台机器和数百万应用。

## 支持的功能
1. 机器资源管理   
   负责机器的初始化、资源记账、分配和故障维修。
2. 应用管理
   数据中心里运行的所有应用的信息存储、调度和自动运维。
   能做到机房断电重启后，将所有应用自动恢复。
3. mapreduce编程框架  
   支持已有业务大量的mapreduce程序迁移、运行。  
   mapreduce是一个特殊的应用，包含一系列子应用（用户job）  
   galaxy-master为mapreduce开放特殊的资源接口，此时master起一个rm的作用
4. 分布式存储  
   提供数据中心级的统一存储

##Agent
1. 任务创建、执行和清理
2. 任务生命周期管理、状态汇报
3. 任务资源管理

##master
1. Agent状态管理
2. 资源管理
3. 任务状态管理
4. 任务调度

##sdk
1. 创建任务
2. 查询任务状态
3. 暂停、终止任务

##学习&思考&总结
* [cgroup_层级关系.md](cgroup_层级关系.md)
* [cgroup_mem.md](cgroup_mem.md)
* [process.md](process.md)
* [master_design](Master_design.md)
* [agent_design](agent_design.md)

