Galaxy ResMan客户端使用
===================

#客户端名称
galaxy_res_client

#配置galaxy.flag
在galaxy客户端同级目录需要放置一个galaxy.flag配置文件,主要配置如下内容
```
--username=xxx  
--token=xxx
--nexus_addr=xxx1:8888,xxx2:8888
--nexus_root=xxx
--resman_path=xxx 
```
注：--nexus_root和--resman_path不需要强制配置，可以使用默认值

测试配置是否正确
```
./galaxy_client list_tags
  -  tag      
----------------
```

如果出现如果下错误信息，则配置错误
```
    1. get resman endpoint from nexus failed: Candidate
    2. SendRequest fail:RPC_ERROR_RESOLVE_ADDRESS
```

#galaxy_res_client使用方法
运行./galaxy_res_client获取运行方法
```
galaxy_res_client.
Usage:
  container usage:
      galaxy_res_client create_container -f <jobconfig>
      galaxy_res_client update_container -f <jobconfig> -i id
      galaxy_res_client remove_container -i id
      galaxy_res_client list_containers [-o cpu,mem,volums]
      galaxy_res_client show_container -i id

  agent usage:
      galaxy_res_client add_agent -p pool -e endpoint
      galaxy_res_client set_agent -p pool -e endpoint
      galaxy_res_client show_agent -e endpoint [-o cpu,mem,volums]
      galaxy_res_client remove_agent -e endpoint
      galaxy_res_client list_agents [-p pool -t tag -o cpu,mem,volums]
      galaxy_res_client online_agent -e endpoint
      galaxy_res_client offline_agent -e endpoint

      galaxy_res_client preempt -i container_group_id -e endpoint

  safemode usage:
      galaxy_res_client enter_safemode
      galaxy_res_client leave_safemode

  status usage:
      galaxy_res_client status

  tag usage:
      galaxy_res_client create_tag -t tag -f endpoint_file
      galaxy_res_client list_tags [-e endpoint]
  pool usage:
      galaxy_res_client list_pools -e endpoint

  user usage:
      galaxy_res_client add_user -u user -t token
      galaxy_res_client remove_user -u user
      galaxy_res_client list_users
      galaxy_res_client show_user -u user
      galaxy_res_client grant_user -u user -p pool -o [add/remove/set/clear]
                                   -a [create_container,remove_container,update_container,
                                   list_containers,submit_job,remove_job,update_job,list_jobs] 
      galaxy_res_client assign_quota -u user -c millicores -d disk_size -s ssd_size -m memory_size -r replica
Options: 
      -f specify config file, job config file or label config file.
      -i specify container id.
      -p specify agent pool.
      -e specify endpoint.
      -u specity user.
      -t specify agent tag or token.
      -o specify operation [cpu,mem,volums].
      -a specify authority split by ,
      -c specify millicores, such as 1000
      -r specify replica, such as 100
      -d specify disk size, such as 1G
      -s specify ssd size, such as 1G
      -m specify memory size, such as 1G
```

## 使用说明
### container容器相关命令
container相关命令中id指的是容器id，容器组id就是jobid
#### create_container 创建容器组
    参数：-f(必选)指定job描述配置文件，文件格式是json格式，可用命令./galaxy_client json命令生成样例
    用法：galaxy_res_client create_container -f job.json

#### update_container 更新容器组
    参数：
        1. -f（必选）指定job描述配置文件 ，文件格式是json格式，用命令./galaxy_client json命令生成样例
        2. -i（必选）指定需要更新容器组的id, 也就是jobid
    用法:
        ./galaxy_res_client update_container -f job.json -i jobid

#### remove_container 删除容器组
    参数：-i指定需要删除的容器组id,也就是jobid
    用法：./galaxy_client remove_container -i jobid

#### list_containers 列出所有的容器组
    参数: -o（可选） 值为cpu,mem,volums(用逗号分隔)
            * 当不加这个参数时，会列出cpu、mem、volums的信息；
            * 当-o others(非cpu,mem,volums)则不会列出cpu、mem、volums的信息，仅列出基本信息
    用法：
        ./galaxy_client list_containers
        ./galaxy_client list_containers -o cpu
        ./galaxy_client list_containers -o base
    说明：
        r/a/p:
            1. r:处于ready状态的容器数量
            2. a:处于allocating状态的容器数量
            3. p:处于pending状态的容器数量

```
  -  id                               replica  r/a/p   cpu(a/u)      mem(a/u)           volums(med/a/u)          create_time          update_time        
-----------------------------------------------------------------------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3       15       15/0/0  45.000/0.000  135.000G/294.938M  kSsd/150.000G/0.000      2016-06-12 19:21:52  2016-06-12 19:21:52
  -  -                                -        -       -             -                  kDisk/600.000G/469.068M  -                    -                  
  -  -                                -        -       -             -                  kTmpfs/120.000G/0.000    -                    -                  
  1  job_20160613_174502_536_diting   1        1/0/0   2.000/0.000   1.000G/1022.832M   kSsd/1.000G/120.085G     2016-06-13 17:45:02  2016-06-13 17:45:02
  -  -                                -        -       -             -                  kDisk/3.000G/720.480G    -                    -                  
  -  -                                -        -       -             -                  kTmpfs/1.000G/1008.242M  -                    -                  
  2  job_20160614_154034_885_example  1        0/0/1   0.000/0.000   0.000/0.000        -                        2016-06-14 15:40:34  2016-06-14 15:40:34
  3  job_20160615_154548_366_test     1        1/0/0   2.000/0.000   1.562G/36.000K     kDisk/1.074G/2.082K      2016-06-15 15:45:48  2016-06-15 15:45:48
```

```
  -  id                               replica  r/a/p   cpu(a/u)      create_time          update_time        
---------------------------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3       15       15/0/0  45.000/0.000  2016-06-12 19:21:52  2016-06-12 19:21:52
  1  job_20160613_174502_536_diting   1        1/0/0   2.000/0.000   2016-06-13 17:45:02  2016-06-13 17:45:02
  2  job_20160614_154034_885_example  1        0/0/1   0.000/0.000   2016-06-14 15:40:34  2016-06-14 15:40:34
  3  job_20160615_154548_366_test     1        1/0/0   2.000/0.000   2016-06-15 15:45:48  2016-06-15 15:45:48
```

```
  -  id                               replica  r/a/p   create_time          update_time        
-------------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3       15       15/0/0  2016-06-12 19:21:52  2016-06-12 19:21:52
  1  job_20160613_174502_536_diting   1        1/0/0   2016-06-13 17:45:02  2016-06-13 17:45:02
  2  job_20160614_154034_885_example  1        0/0/1   2016-06-14 15:40:34  2016-06-14 15:40:34
  3  job_20160615_154548_366_test     1        1/0/0   2016-06-15 15:45:48  2016-06-15 15:45:48
```

#### show_container 展示容器组, 列出的信息包括:创建容器时提交的job配置信息，podinfo信息
    参数: -i指定需要展示的容器组id,也就是jobid 
    用法:
        ./galaxy_client show_container -i job_20160612_192152_72_ts3 -o cpu
    说明:
        基本信息：base infomation，workspace volum infomation, data volums infomation
        容器信息：cgroups infomation, containers infomation
        
```
base infomation
  user    version                           priority     cmd_line  max_per_host  tag  pools
---------------------------------------------------------------------------------------------
  galaxy  ver_20160612_19:33:18_1349330448  kJobService  -         10            sjy  -    

workspace volum infomation
  size     type       medium  dest_path   readonly  exclusive  use_symlink
----------------------------------------------------------------------------
  20.000G  kEmptyDir  kDisk   /home/work  false     false      false      

data volums infomation
  -  size     type       medium  source_path  dest_path    readonly  exclusive  use_symlink
---------------------------------------------------------------------------------------------
  0  20.000G  kEmptyDir  kDisk   -            /home/disk1  false     false      false      
  1  10.000G  kEmptyDir  kSsd    -            /home/myssd  false     false      false      
  2  8.000G   kEmptyDir  kTmpfs  -            /home/ramfs  false     false      false      

cgroups infomation
  -  id  cpu_cores  cpu_excess  mem_size  mem_excess  tcp_recv_bps  tcp_recv_excess  tcp_send_bps  tcp_send_excess  blk_weight
--------------------------------------------------------------------------------------------------------------------------------
  0  0   3.000      false       9.000G    false       55.000M       true             35.000M       true             2         

containers infomation
  -   id      endpoint          status  cpu(a/u)     mem(a/u)        volums(id/medium/a/u)                   last_error
-------------------------------------------------------------------------------------------------------------------------
  0   pod_0   xxxx:6666         Ready   3.000/0.000  9.000G/17.129M  vol_0 kDisk 20.000G/0.000 /home/disk1   kResOk    
  -   -       -                 -       -            -               vol_1 kSsd 10.000G/0.000 /home/myssd    -         
  -   -       -                 -       -            -               vol_2 kTmpfs 8.000G/0.000 /home/ramfs   -         
  -   -       -                 -       -            -               vol_3 kDisk 20.000G/31.271M /home/work  -         
  1   pod_1   xxxx:6666         Ready   3.000/0.000  9.000G/33.367M  vol_0 kDisk 20.000G/0.000 /home/disk1   kResOk    
  -   -       -                 -       -            -               vol_1 kSsd 10.000G/0.000 /home/myssd    -         
  -   -       -                 -       -            -               vol_2 kTmpfs 8.000G/0.000 /home/ramfs   -         
  -   -       -                 -       -            -               vol_3 kDisk 20.000G/31.271M /home/work  -         

  .
  .
  .

  14  pod_9   xxxx:6666         Ready   3.000/0.000  9.000G/3.418M   vol_0 kDisk 20.000G/0.000 /home/disk1   kResOk    
  -   -       -                 -       -            -               vol_1 kSsd 10.000G/0.000 /home/myssd    -         
  -   -       -                 -       -            -               vol_2 kTmpfs 8.000G/0.000 /home/ramfs   -         
  -   -       -                 -       -            -               vol_3 kDisk 20.000G/31.271M /home/work  -         

```

### agent相关命令
#### add_agent 向galaxy机器池中增加一台机器
如果想在galaxy中增加一台机器，需要做如下两步:
1. 使用add_agent命令，将机器添加到对应的机器池(pool)中
2. 在agent上安装agent程序
    
    参数:
        1. -p（必选） 指定的机器池名称
        2. -e（必选）endpoint，形如ip:port
    用法：
        ./galaxy_res_client add_agent -p pool -e xxxx:6666

#### set_agent 更改机器的机器池(pool) 
    参数：
        1. -p（必选） 指定的机器池名称
        2. -e（必选）endpoint，形如ip:port
    用法:
        ./galaxy_res_client set_agent -p pool -e xxxx:6666

#### remove_agent 从galaxy中删除一台机器
    参数:
        -e endpoint，形如ip:port
    用法:
        ./galaxy_res_client remove_agent -e xxxx:6666

#### online_agent 上线一台机器
    参数:
        -e endpoint，形如ip:port
    用法:
        ./galaxy_res_client online_agent -e xxxx:6666

#### offline_agent 下线一条机器
    参数:
        -e endpoint，形如ip:port
    用法:
        ./galaxy_res_client offline_agent -e xxxx:6666

#### show_agent 展示agent上的所有容器信息
    参数:
        1. -e（必选）endpoint
        2. -o（可选—）值为cpu,mem,volums(用逗号分隔)
            * 当不加这个参数时，会列出cpu、mem、volums的信息；
            * 当-o others(非cpu,mem,volums)则不会列出cpu、mem、volums的信息，仅列出基本信息
    用法: 
        ./galaxy_res_client show_agent -e xxxx:6666
        ./galaxy_res_client show_agent -e xxxx:6666 -o cpu
        ./galaxy_res_client show_agent -e xxxx:6666 -o base
    说明:
        status是容器的状态：Pending, Allocating, Ready, Finish, Error, Destroying, Terminated
        last_error是容器分配失败的错误提示，kResOk表示没有错误
```
containers infomation
  -  id                                 endpoint          status  last_error  cpu(a/u)     mem(a/u)        vol(medium/a/u)                 
---------------------------------------------------------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3.pod_11  xxxx:6666         Ready   kResOk      3.000/0.000  9.000G/15.832M  kDisk/20.000G/0.000 /home/disk1 
  -  -                                  -                 -       -           -            -               kSsd/10.000G/0.000 /home/myssd  
  -  -                                  -                 -       -           -            -               kTmpfs/8.000G/0.000 /home/ramfs 
  -  -                                  -                 -       -           -            -               kDisk/20.000G/31.271M /home/work
  1  job_20160612_192152_72_ts3.pod_12  xxxx:6666         Ready   kResOk      3.000/0.000  9.000G/16.875M  kDisk/20.000G/0.000 /home/disk1 
  -  -                                  -                 -       -           -            -               kSsd/10.000G/0.000 /home/myssd  
  -  -                                  -                 -       -           -            -               kTmpfs/8.000G/0.000 /home/ramfs 
  -  -                                  -                 -       -           -            -               kDisk/20.000G/31.271M /home/work
  2  job_20160612_192152_72_ts3.pod_6   xxxx:6666         Ready   kResOk      3.000/0.000  9.000G/16.516M  kDisk/20.000G/0.000 /home/disk1 
  -  -                                  -                 -       -           -            -               kSsd/10.000G/0.000 /home/myssd  
  -  -                                  -                 -       -           -            -               kTmpfs/8.000G/0.000 /home/ramfs 
  -  -                                  -                 -       -           -            -               kDisk/20.000G/31.271M /home/work
  3  job_20160612_192152_72_ts3.pod_7   xxxx:6666         Ready   kResOk      3.000/0.000  9.000G/15.910M  kDisk/20.000G/0.000 /home/disk1 
  -  -                                  -                 -       -           -            -               kSsd/10.000G/0.000 /home/myssd  
  -  -                                  -                 -       -           -            -               kTmpfs/8.000G/0.000 /home/ramfs 
  -  -                                  -                 -       -           -            -               kDisk/20.000G/31.271M /home/work
```
```
containers infomation
  -  id                                 endpoint          status  last_error  cpu(a/u)   
-------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3.pod_11  xxxx:6666         Ready   kResOk      3.000/0.000
  1  job_20160612_192152_72_ts3.pod_12  xxxx:6666         Ready   kResOk      3.000/0.000
  2  job_20160612_192152_72_ts3.pod_6   xxxx:6666         Ready   kResOk      3.000/0.000
  3  job_20160612_192152_72_ts3.pod_7   xxxx:6666         Ready   kResOk      3.000/0.000
```

```
containers infomation
  -  id                                 endpoint          status  last_error  
-----------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3.pod_11  xxxx:6666         Ready   kResOk      
  1  job_20160612_192152_72_ts3.pod_12  xxxx:6666         Ready   kResOk      
  2  job_20160612_192152_72_ts3.pod_6   xxxx:6666         Ready   kResOk      
  3  job_20160612_192152_72_ts3.pod_7   xxxx:6666         Ready   kResOk     
```
#### list_agents 列出所有agent
    参数:
        1. -p（可选）指定机器池名称
        2. -t（可选）指定机器tag号
        3. -o（可选—）值为cpu,mem,volums(用逗号分隔)
            * 当不加这个参数时，会列出cpu、mem、volums的信息；
            * 当-o others(非cpu,mem,volums)则不会列出cpu、mem、volums的信息，仅列出基本信息
    用法:
        ./galaxy_res_client list_agents ：列出所有的机器
        ./galaxy_res_client list_agents -t hlf -o base ：列出tag为hlf的所有机器
        ./galaxy_res_client list_agents -p test -o base ：列出test机器池中的所有机器
        ./galaxy_res_client list_agents -p test -t hlf ：列出test机器池中的tag为hlf的机器
    说明:
        status是机器的状态: Unknown, Alive, Dead, Offline
        containers表示该机器上的容器个数

```
  -   endpoint          status  pool      tags                    containers
------------------------------------------------------------------------------
  0   xxxx:6666         Alive   haolifei  sjy                     0         
  1   xxxx:6666         Alive   test      haolifei,sjy            0         
  2   xxxx:6666         Alive   test      hlf                     2         
  3   xxxx:6666         Alive   test      sjy                     2         
  4   xxxx:6666         Alive   test      sjy                     4         
  5   xxxx:6666         Alive   test      sjy                     1
```

### safemode 安全模式命令
#### enter_safemode 进入安全模式, 表示galaxy不再接受新提交的job，但进入安全模式之前的job正常运行
        用法: ./galaxy_res_client enter_safemode
#### leave_safemode 离开安全模式
        用法: ./galaxy_res_client leave_safemode

### status galaxy的整体情况
    用法：./galaxy_res_client status
```
cluster agent infomation
  total  alive  dead
----------------------
  16     16     0   

cluster cpu infomation
  total    assigned  used 
----------------------------
  384.000  49.000    0.000

cluster memory infomation
  total   assigned  used 
---------------------------
  1.625T  258.562G  0.000

cluster volumes infomation
  -  medium  total     assigned  used   device_path
-----------------------------------------------------
  0  kSsd    13.296T   151.000G  0.000  -          
  1  kDisk   214.469T  604.074G  0.000  -          

cluster pools infomation
  -  name      total  alive
-----------------------------
  0  haolifei  1      1    
  1  test      15     15   

cluster other infomation
  total_cgroups  total_containers  in_safe_mode
-------------------------------------------------
  4              17                false       

```
### tag相关命令
#### create_tag 给机器贴标签
    参数:
        1. -t 指定tag
        2. -f endpoint文件，内容形式(ip:port)如下:
```
xxxx0:6666
xxxx1:6666
xxxx3:6666
```
    用法:
        galaxy_res_client create_tag -t test -f endpoint.ep

#### list_tags
    参数:
        1. -e（可选）指定endpoint，形如ip:port
    用法:
        galaxy_res_client list_tags
        galaxy_res_client list_tags -e xxxx:6666
```
  -  tag      
----------------
  0  test 
  1  hlf      
```

### pool机器池相关命令
    参数:
        1. -e（必选）指定endpoint，形如ip:port
    用法:
       galaxy_res_client list_pools -e xxxx:6666 

### user用户相关命令
#### add_user 添加用户
    参数：
        1. -u（必选）用户名
        2. -t（必选）token
    用法:
        galaxy_res_client add_user -u galaxy -t galaxy
#### remove_user 删除用户
    参数: -u（必选）用户名
    用法：
        galaxy_res_client remove_user -u galaxy
#### list_users 列出所有用户
    参数：无
    用法: ./galaxy_res_client list_users
```
  -  user       
------------------
  0  abc        
  2  hlf        
```
#### show_user 展示指定的用户信息
    参数: -u（必选）用户名
    用法: ./galaxy_res_client show_user
```
authority infomation
  -  pool      authority                
------------------------------------------
  0  test      kAuthorityCreateContainer
  -  -         kAuthorityRemoveContainer
  -  -         kAuthorityUpdateContainer

quota infomation
  cpu       memory   disk      ssd     replica
------------------------------------------------
  8000.000  15.000T  100.000T  8.000T  3000   

jobs assigned quota infomation
  cpu     memory    disk     ssd       replica
------------------------------------------------
  95.000  514.344G  31.177T  151.000G  33     
```
#### grant_user 赋予某pool机器池的权限给用户
    参数: 
       1. -u（必选）用户名
       2. -p（必选）机器池名称
       3. -o（必选）权限操作类型：add/remove/set/clear
       4. -a (必选）权限类型: create_container,remove_container,update_container,
                              list_containers,submit_job,remove_job,update_job,list_jobs
    用法:
        ./galaxy_res_client grant_user -u galaxy -p test -o add -a create_container
        ./galaxy_res_client grant_user -u galaxy -p test -o remove -a remove_job
#### assign_quota 分配quota给用户
quota指的是用户可运行程序的所有副本总数，cpu核数，disk、ssd大小，内存大小

    参数:
        1. -u（必选）用户名
        2. -c（必选） cpu核数*1000,如2000
        3. -d（必选）disk大小, 单位有K, M, G, T, B, Z
        4. -s（必选）ssd大小, 单位有K, M, G, T, B, Z
        5. -m（必选）内存大小, 单位有K, M, G, T, B, Z
        6. -r （必选）副本数量
    用法:
        galaxy_res_client assign_quota -u galaxy -c 1000 -d 1G -s 800M -m 1G -r 10000
