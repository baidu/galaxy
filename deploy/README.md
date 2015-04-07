### galaxy 部署脚本

#### 建立机器信任关系
在一批机器上面找一台机器A和剩余机器建立信任关系

#### 在A机器上面安装部署工具

* install python-pip ,对于内部同学请参考pip.baidu.com
* pip install fabric

#### 打包
可以使用 make-package.sh打包，将带上启停脚本

#### 部署master

```
#第一次部署到建立信任关系的host1上面
fab -H host1 -f deploy_master.py first:port=9527,ftp=包下载ftp url
#查看更多命令包含migrate,start,stop,fetch
fab -f deploy_master.py -l
```

#### 部署agent

```
#第一次部署到host2 host3上面
fab -H host2,host3 -f deploy_agent.py first:port=12345,master=host1:9527,ftp=包下载ftp url
#查看更多部署命令包含migrate,start,stop,fetch
fab -f deploy_agent.py -l
```



