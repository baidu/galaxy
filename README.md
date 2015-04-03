# Galaxy

[![Build Status](https://travis-ci.org/bluebore/galaxy.svg?branch=master)](https://travis-ci.org/bluebore/galaxy)

## Todolist
###主线：
1. Web界面  
   * 资源管理界面能展示出两层树形交换网络  
   * 能看到task的输出（stdout、stderr），以便于判断任务失败原因。  
   * 查看历史任务状态
2. 名字服务  
   根据Job获取机器、端口列表  
3. Master HA
4. Mapreduce terasort

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
