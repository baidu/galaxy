# Galaxy

[![Build Status](https://travis-ci.org/bluebore/galaxy.svg?branch=master)](https://travis-ci.org/bluebore/galaxy)

## Todolist
###主线：
1. Agent的任务生命周期管理  
   在任务down掉时，agent负责重启FLAGS_agent_retry次，重试过程中，向master汇报的状态为"重启中"。
2. 控制台与Web界面  
   可以看到当前集群上所有任务的状态，实例数。  
   Web管理界面，支持任务的提交（上传或提交一个可wget的地址），任务状态的查看，实例数的增减，资源限制调整。
3. Agent的资源限制与隔离功能(开启cgroup agent启动参数加上--container=cgroup 默认是cmd)
4. 支持NFS的chunkserver和tera的tabletserver部署。

###非主线：
1. Agent能收集当期机器的硬件信息（CPU核数、内存、磁盘个数&大小）
2. Web管理界面和tera的集群管理对接
3. Agent支持磁盘空间管理，即Workspace使用的磁盘空间是有配额的
4. Agent支持SSD和多磁盘使用，当前Workspace只能是一个磁盘上的
5. 监控系统，集成在web控制台
   任务运行时间，重启次数

## Milestone
3月底完成第一版demo  
4月底发布第一版，支持Tera集群调度  
6月底发布第二版，支持Spider3.0集群调度  

## 尝鲜
```
comake2 && make -j12
cd sandbox
sh local-run.sh
sh attach-task.sh
#clean
sh local-killall.sh
```
