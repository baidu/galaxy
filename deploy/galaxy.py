# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-20
import os
def build_init_cgroup_cmd(subsys_list):
    all_cmd = []
    cgroup_root = "/cgroups"
    all_cmd.append("mkdir -p %s"%cgroup_root)
    all_cmd.append("mount -t tmpfs cgroup %s"%cgroup_root)
    for subsys in subsys_list:
        sub_path = os.path.sep.join([cgroup_root,subsys])
        all_cmd.append("mkdir -p %s"%sub_path)
        all_cmd.append("mount -t cgroup -o%s none %s"%(subsys,sub_path))
    return all_cmd
def build_clean_cgroup_cmd(subsys_list):
    all_cmd = []
    cgroup_root = "/cgroups"
    for subsys in subsys_list: 
        sub_path = os.path.sep.join([cgroup_root,subsys])
        all_cmd.append("umount %s"%sub_path)
    all_cmd.append("umount %s"%cgroup_root)
    return all_cmd

#当初始化一台机器将执行下面命令
INIT_SYS_CMDS= build_init_cgroup_cmd(["cpu","memory","cpuacct"])
#当清理一台机器时执行下面的命令
CLEAN_SYS_CMDS = build_clean_cgroup_cmd(["cpu","memory","cpuacct"])
#sampe node list
def build_node_list():
    nodes = []
    for i in range(81,88):
        nodes.append("yq01-tera%d.yq01.baidu.com"%i)
    return nodes
#机器列表
NODE_LIST= build_node_list()

#使用字典描述应用
MASTER={
    "package":"ftp://cp01-rdqa-dev400.cp01.baidu.com/tmp/galaxy.tar.gz",
    "start_cmd":"cd galaxy && sh ./bin/start-master.sh 9876",
    "stop_cmd":"cd galaxy && sh ./bin/stop-master.sh 9876",
    "hosts":["yq01-tera81.yq01.baidu.com"],
    "name":"master",
    "workspace":"/home/galaxy/master"
}

AGENT={
    "package":"ftp://cp01-rdqa-dev400.cp01.baidu.com/tmp/galaxy.tar.gz",
    "start_cmd":"cd galaxy && sh ./bin/start-agent.sh  yq01-tera81.yq01.baidu.com:9876 9578 128 24",
    "stop_cmd":"cd galaxy && sh ./bin/stop-agent.sh 9578",
    "hosts":["yq01-tera81.yq01.baidu.com"],
    "name":"agent",
    "workspace":"/home/galaxy/agent"
}
#需要部署的应用
APPS=[MASTER,AGENT]

