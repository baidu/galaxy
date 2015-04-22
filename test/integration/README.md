## Galaxy 集成测试框架
实现原理很简单，使用python写测试用例,例如[case_list_all_nodes.py](case_list_all_nodes.py),包含set_up clean函数，以及包含test关键字函数,
然后runner.py 执行所有case文件

### 环境依赖
* python2.7
* pip
* sofapb-rpc python
* paramiko
以下是具体安装步骤
```

git clone https://github.com/BaiduPS/sofa-pbrpc
#安装前记得修改depends.mk里面protobuf安装目录
cd sofa-pbrpc/python && python setup.py install

#安装 pip
请参照pip.baidu.com

#安装paramiko
pip install paramiko
```


### galaxy python sdk 加入到 PYTHONPATH
进入目录[console backend](../../console/backend/)
```
sh compile-proto.sh
export PYTHONPATH=`pwd`/src
```
### 运行inte-test-on-cluster.sh
集群使用tera80~87上面的机器
这个脚本将进行一下操作
* 执行编译命令
* 执行打包命令
* 执行集群部署命令
* 执行本地测试case

需要注意的地方
* 修改galaxy_ci.py 里面的package地址 指向自己开发机的ftp地址

```
#执行集成测试 arg1为机器账户 arg2为机器密码
sh inte-test-on-cluster.sh arg1 arg2
```

### python 怎么和master通讯
使用sofapb-rpc python实现和master 通讯，目前在[python-sdk](../../console/backend/src/galaxy/sdk.py),实现了一些sdk功能，如果要测试
agent,可以参照sdk生成python rpc client代码

