### console models 设计

主要包含一下几大对象
* service 相当简单就是一个服务对象，主要记录一个服务名称，创建人，时间描述信息
* task 任务对象
* taskgroup 任务组，一个service创建时会有个默认taskgroup0组，包含所有的实例，当小流量上线时会创建一个taskgroup1临时组，包含小流量实例，当全流量时，会把taskgroup1实例移到默认组里面，并更新上线配置
* deployrequirement 部署元数据，每个taskgroup有多个deployrequirement,用户保存部署数据
* resourcerequirement 资源需求
```
                        service
                        /     \
                       /       \
         (默认组)taskgroup0  taskgroup1(临时组，用于小流量)
         |       |    /   |    |     |    \
       task0    task1/  task2  task3 task4 \
                    /                       \
                  /                          \
                 /                            \
        deployrequirement              deployrequirement      
        |              |               |              |
resourcerequirement   ...     resourcerequirment     ...    

       
```
