## deployer
deployer 是基于ssh,用于在多台机器上面执行部署命令已经初始化命令
* 支持使用python编写描述文件，提升部署脚本的维护性

### 安装依赖

* install python-pip ,对于内部同学请参考pip.baidu.com
* pip install [paramiko](https://github.com/paramiko/paramiko/)

### 简单使用
启动galaxy集群
```
#未建立信任关系
python deployer.py migrate -f galaxy.py -u xxx -p xxxx
#建立了信任关系
python deployer.py migrate -f galaxy.py
```

### 所有子命令
```
usage: 
   deployer <command> [<args>...]

positional arguments:
  {stop,fetch,start,init,clean,migrate}
    stop                stop apps on all hosts
    fetch               fetch package to all hosts
    start               start apps on all hosts
    init                init all nodes
    clean               clean all nodes
    migrate             migrate all apps

optional arguments:
  -h, --help            show this help message and exit
```
### 描述文件介绍
```
#这个命令集合，用于描述初始化系统时应该执行什么命令，可以看看galaxy.py里面例子
INIT_SYS_CMDS=[]

#清理系统命令
CLEAN_SYS_CMDS=[]
#需要部署应用列表，可以参照galaxy.py MASTER 和 AGENT描述
APPS=[]
```


