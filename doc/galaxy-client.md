Galaxy客户端使用
===================

#客户端名称
galaxy_client

#配置galaxy.flag
在galaxy客户端同级目录需要放置一个galaxy.flag配置文件,主要配置如下内容
'''
--username=xxx  
--token=xxx
--nexus_addr=xxx1:8888, xxx2:8888
--resman_path=xxx 
--appmaster=xxx
'''
注：--resman_path和--appmaster不需要强制配置，可以使用默认值

测试配置是否正确
```
./galaxy_client list -o ppp(whatever you want)
-  id  name     type         status   r/p/d/die/f  repli  create_time update_time
-----------------------------------------------------------------------------------
```

如果出现如果下错误信息，则配置错误
```
    1. get appmaster endpoint from nexus failed: Candidate
    2. SendRequest fail:RPC_ERROR_RESOLVE_ADDRESS
```

#galaxy用法
运行./galaxy_client获取运行方法
```
galaxy_client.
Usage:
      galaxy submit -f <jobconfig>
      galaxy update -f <jobconfig> -i id [-t breakpoint -o pause|continue|rollback]
      galaxy stop -i id
      galaxy remove -i id
      galaxy list [-o cpu,mem,volums]
      galaxy show -i id [-o cpu,mem,volums]
      galaxy exec -i id -c cmd
      galaxy json [-n jobname -t num_task -d num_data_volums -p num_port -a num_packages in data_package -s num_service]
Options: 
      -f specify config file, job config file or label config file.
      -c specify cmd.
      -i specify job id.
      -t specify specify task num or update breakpoint, default 0.
      -d spicify data_volums num, default 1
      -p specify port num, default 1
      -a specify packages num in data_package, default 1
      -s specify service num, default 1
      -n specify job name
      -o specify operation.
```

## 使用说明
### submit 提交一个job
    参数：-f(必选)指定job描述配置文件，文件格式是json格式
    用法：./galaxy_client submit -f job.json

### update 更新一个job，支持容器、副本多断点更新；支持更新暂停，回滚
    参数：
        1. -f（必选）指定job描述配置文件 ，文件格式是json格式
        2. -i（必选）指定需要更新的jobid, 当仅需要批量更新job，不考虑暂停点，则-f和-i两个参数足够了
        3. -t指定暂停点，更新job时，job更新的副本数达到这个值时则暂停更新
        4. -o指定更新job时的操作，pause表示暂停，continue表示继续，rollback表示回滚
    用法:
        批量更新：./galaxy_client update -i jobid -f job.json  
        断点更新：./galaxy_client update -i jobid -f job.json -t 5
        继续更新：./galaxy_client update -i jobid -o continue
        多断点更新：./galaxy_client update -i jobid -t 5 -o continue
        暂停更新：./galaxy_client update -i jobid -o pause
        回滚：./galaxy_client update -i jobid -o rollback

### stop 停止一个job
    参数：-i指定需要更新的jobid
    用法：./galaxy_client stop -i jobid

### remove 删除一个job
    参数：-i指定需要更新的jobid
    用法：./galaxy_client remove -i jobid

### list 列出所有的job
    参数: -o（可选） 值为cpu,mem,volums(用逗号分隔)
            * 当不加这个参数时，会列出cpu、mem、volums的信息；
            * 当-o others(非cpu,mem,volums)则不会列出cpu、mem、volums的信息，仅列出基本信息
    用法：
        ./galaxy_client list
        ./galaxy_client list -o cpu
        ./galaxy_client list -o base
    说明：
        r/p/d/die/f:
            1. r:处于running状态的副本树
            2. p:处于pending状态的副本数
            3. d:处于部署状态的副本数
            4. die:死亡的副本数
            5. f:失败的副本数

```
-  id                                 name     type         status   r/p/d/die/f  repli  cpu(a/u)      memory(a/u)        volums(med/a/u)          create_time          update_time        
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  0  job_20160612_192152_72_ts3       ts3      kJobService  Running  15/0/0/0/0   15     45.000/0.000  135.000G/294.938M  kSsd/150.000G/0.000      2016-06-12 19:21:52  2016-06-12 19:39:16
  -  -                                -        -            -        -            -      -             -                  kDisk/600.000G/469.068M  -                    -                  
  -  -                                -        -            -        -            -      -             -                  kTmpfs/120.000G/0.000    -                    -                  
```

```
-  id                               name     type         status   r/p/d/die/f  repli  cpu(a/u)      create_time          update_time        
-------------------------------------------------------------------------------------------------------------------------------------------------
0  job_20160612_192152_72_ts3       ts3      kJobService  Running  15/0/0/0/0   15     45.000/0.000  2016-06-12 19:21:52  2016-06-12 19:39:16
```

```
-  id                               name     type         status   r/p/d/die/f  repli  create_time          update_time        
-----------------------------------------------------------------------------------------------------------------------------------
0  job_20160612_192152_72_ts3       ts3      kJobService  Running  15/0/0/0/0   15     2016-06-12 19:21:52  2016-06-12 19:39:16

```

### show 展示一个job, 列出的信息包括:提交job时的job配置信息，pod运行信息podinfo，service信息
    参数：  
        1. -i（必选）指定需要更新的jobid
        2. -o（可选） 值为cpu,mem,volums(用逗号分隔)
    用法:
        ./galaxy_client show -i job_20160612_192152_72_ts3 -o cpu
    说明:
        基本信息：base infomation，job description base infomation，job description deploy infomation，job description pod workspace_volum infomation
                  job description pod data_volums infomation等 
        pod信息: podinfo
        service信息：serviceinfo
        
```
base infomation
  id                          status   create_time          update_time        
---------------------------------------------------------------------------------
  job_20160612_192152_72_ts3  Running  2016-06-12 19:21:52  2016-06-12 19:39:16

job description base infomation
  name  type         run_user
-------------------------------
  ts3   kJobService  galaxy  

job description deploy infomation
  replica  step  interval  max_per_host  tag  pools
-----------------------------------------------------
  15       2     0         10            sjy  test 

job description pod workspace_volum infomation
  size     type       medium  dest_path   readonly  exclusive  use_symlink
----------------------------------------------------------------------------
  20.000G  kEmptyDir  kDisk   /home/work  false     false      false      

job description pod data_volums infomation
  size     type       medium  dest_path    readonly  exclusive  use_symlink
-----------------------------------------------------------------------------
  20.000G  kEmptyDir  kDisk   /home/disk1  false     false      false      
  10.000G  kEmptyDir  kSsd    /home/myssd  false     false      false      
  8.000G   kEmptyDir  kTmpfs  /home/ramfs  false     false      false      

job description pod task infomation
=========================================================
job description pod task [0] base infomation
  -  id  cpu(cores/excess)  memory(size/excess)  tcp_throt(r/re/s/se)       blkio  ports(name/port) 
------------------------------------------------------------------------------------------------------
  0  0   3.000/false        9.000G/false         55.000M/true/35.000M/true  2      REDIS/dynamic    
  -  -   -                  -                    -                          -      linkbase1/dynamic

job description pod task [0] exe_package infomation
-----------------------------------------------
start_cmd: cd bin && sh -x start.sh

stop_cmd: cd bin && sh -x stop.sh

dest_path: bin

version: 2.0.42

job description pod task [0] data_package infomation
-----------------------------------------------
reload_cmd: sh -x reload.sh
  -  version  dest_path
-------------------------
  0  1.1.4    data1    

job description pod task [0] services infomation
  -  name      port_name  use_bns
-----------------------------------
  0  service0  REDIS      false  
  1  service1  linkbase1  false  

podinfo infomation
  -   podid   endpoint   status   fails  container  last_error  update  cpu(a/u)     create_time          update_time        
--------------------------------------------------------------------------------------------------------------------------------------
  0   pod_0   XXXX:6666  Running  0      Ready      kResOk      Yes     3.000/0.000  2016-06-14 19:03:49  2016-06-12 19:39:16
  1   pod_1   XXXX:6666  Running  0      Ready      kResOk      Yes     3.000/0.000  2016-06-12 19:35:22  2016-06-12 19:39:16
  7   pod_2   XXXX:6666  Running  0      Ready      kResOk      Yes     3.000/0.000  2016-06-12 19:33:32  2016-06-12 19:39:16
  8   pod_3   XXXX:6666  Running  0      Ready      kResOk      Yes     3.000/0.000  2016-06-14 18:04:34  2016-06-12 19:39:16

services infomation
  -  podid  service_addr  name  port  status
---------------------------------------------
```

### exec 执行命令
    参数:
        1. -i（必选） 指定jobid
        2. -c（必选）指定要执行的命令
    用法：
        ./galaxy_client -i jobid -c cmd
    
### json 生成一json格式的job配置文件
    参数：
        1. -n(可选) 指定jobname，默认为example
        2. -t(可选) 指定task数，默认1
        3. -d(可选) 指定data_volums数，默认1
        4. -p(可选) 指定port数，默认1
        5. -a(可选) 指定data package数，默认1
        6. -s(可选) 指定services数，默认1
    用法:
        ./galaxy_client json

```
{
    "name": "example",
    "type": "kJobService",
    "deploy": {
        "replica": 1,
        "step": 1,
        "interval": 1,
        "max_per_host": 1,
        "tag": "",
        "pools": "example,test"
    },
    "pod": {
        "workspace_volum": {
            "size": "300M",
            "type": "kEmptyDir",
            "medium": "kDisk",
            "dest_path": "/home/work",
            "readonly": false,
            "exclusive": false,
            "use_symlink": false
        },
        "data_volums": [
            {
                "size": "800M",
                "type": "kEmptyDir",
                "medium": "kDisk",
                "dest_path": "/home/data/0",
                "readonly": false,
                "exclusive": false,
                "use_symlink": true
            }
        ],
        "tasks": [
            {
                "cpu": {
                    "millicores": 1000,
                    "excess": false
                },
                "mem": {
                    "size": "800M",
                    "excess": false
                },
                "tcp": {
                    "recv_bps_quota": "30M",
                    "recv_bps_excess": false,
                    "send_bps_quota": "30M",
                    "send_bps_excess": false
                },
                "blkio": {
                    "weight": 500
                },
                "ports": [
                    {
                        "name": "example_port00",
                        "port": "12300"
                    }
                ],
                "exec_package": {
                    "start_cmd": "sh app_start.sh",
                    "stop_cmd": "",
                    "health_cmd": "",
                    "package": {
                        "source_path": "ftp://***.baidu.com/home/users/***/exec/0/linkbase.tar.gz",
                        "dest_path": "/home/spider/0",
                        "version": "1.0.0"
                    }
                },
                "data_package": {
                    "reload_cmd": "",
                    "packages": [
                        {
                            "source_path": "ftp://***.baidu.com/home/users/***/data/00/linkbase.dict.tar.gz",
                            "dest_path": "/home/spider/00/dict",
                            "version": "1.0.0"
                        }
                    ]
                },
                "services": [
                    {
                        "service_name": "example_service00",
                        "port_name": "example_port00",
                        "user_bns": false
                    }
                ]
            }
        ]
    }
}
```
