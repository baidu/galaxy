## Galaxy 集成测试框架
实现原理很简单，使用python写测试用例,例如[case_list_all_nodes.py](case_list_all_nodes.py),包含set_up clean函数，以及包含test关键字函数,
然后runner.py 执行所有case文件

### 环境依赖

```
git clone https://github.com/BaiduPS/sofa-pbrpc
cd sofa-pbrpc/python && python setup.py install
```

### galaxy python sdk 加入到 PYTHONPATH
进入目录[console backend](../../console/backend/)
```
sh compile-proto.sh
export PYTHONPATH=`pwd`/src
```

### 执行case
```
#-d指定测试case所在目录
python runner.py -d .
```

### python 怎么和master通讯
使用sofapb-rpc python实现和master 通讯，目前在[python-sdk](../../console/backend/src/galaxy/sdk.py),实现了一些sdk功能，如果要测试
agent,可以参照sdk生成python rpc client代码

