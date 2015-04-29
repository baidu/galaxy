## Galaxy 集成测试框架

### 环境依赖
* python2.7
* pip
* sofapb-rpc python
* nose
以下是具体安装步骤
```

git clone https://github.com/BaiduPS/sofa-pbrpc
#安装前记得修改depends.mk里面protobuf安装目录
cd sofa-pbrpc/python && python setup.py install

#安装 pip
请参照pip.baidu.com

#安装nose
pip install nose
```


### galaxy python sdk 加入到 PYTHONPATH
进入目录[console backend](../../console/backend/)
```
sh compile-proto.sh
export PYTHONPATH=`pwd`/src
```
### 本地运行集成测试
sh inte-test-on-local.sh

### 集群测试部署
sh inte-test-on-cluster.sh

### python 怎么和master通讯
使用sofapb-rpc python实现和master 通讯，目前在[python-sdk](../../console/backend/src/galaxy/sdk.py),实现了一些sdk功能，如果要测试
agent,可以参照sdk生成python rpc client代码

