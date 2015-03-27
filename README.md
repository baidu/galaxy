# Galaxy
Galaxy

## Todolist
###主线：
1. 在任务down掉时，agent负责重启FLAGS_agent_retry次，重试过程中，向master汇报的状态为"重启中"。
2. Agent支持磁盘空间管理，即Workspace使用的磁盘空间是有配额的
3. Agent支持SSD和多磁盘使用，当前Workspace只能是一个磁盘上的
4. 控制台可以看到当前集群上所有任务的状态，实例数。


###非主线：
1. Agent能收集当期机器的硬件信息（CPU核数、内存、磁盘个数&大小）
2. 

### 目标
3月底完成第一版demo
4月底第一版上线，支持Tera集群调度

### 尝鲜
```
comake2 && make -j12
cd sandbox
sh local-run.sh
sh attach-task.sh
#clean
sh local-killall.sh
```
