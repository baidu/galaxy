# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-21
import subprocess
import os
import time
import utils
import socket
import threading

import agent
import rpc_server
from galaxy import sdk
from nose import tools

CASE_TMP_FOLDER = os.sep.join([os.environ.get("GALAXY_CASE_FOLDER"),"case_node_test"])
MASTER_BIN_PATH = os.environ.get("MASTER_BIN_PATH")
MASTER_PORT = 12059
AGENT_PORT = 14460
TASK_SCRIPT="""#!/usr/bin/env sh
for ((i=0;i<1000;i++)) do
    echo $i;
    sleep 1;
done;
"""

def setup():
    global master_ctrl
    global package
    global server
    master_ctrl = utils.MasterCtrl(MASTER_BIN_PATH,CASE_TMP_FOLDER,MASTER_PORT) 
    ret = master_ctrl.start()
    if not ret :
        print "fail to start master"
        assert False
    time.sleep(1)
    mock_agent = agent.AgentImpl(8.0,1024*1024*1024*10,AGENT_PORT,"127.0.0.1:%d"%MASTER_PORT)
    heartbeat_t = threading.Thread(target=mock_agent.HeartBeat)
    heartbeat_t.daemon = True
    heartbeat_t.start()
    server = rpc_server.RpcServer(AGENT_PORT,host="0.0.0.0")
    server.add_service(mock_agent)
    server_t = threading.Thread(target=server.start)
    server_t.daemon = True
    server_t.start()
    time.sleep(2)
    utils.ShellHelper.run_with_returncode("tar -zxvf task.sh.tar.gz task.sh && cp task.sh.tar.gz /tmp")
    package = "ftp://%s/tmp/task.sh.tar.gz"%socket.gethostname()

def test_deploy_task():
    client = sdk.GalaxySDK("127.0.0.1:%d"%MASTER_PORT)
    status,job_id=client.make_job("test",
                                  "ftp",
                                  package,
                                  "sh task.sh",
                                  replicate_num=4,
                                  mem_limit=1024,
                                  cpu_limit=1,
                                  deploy_step_interval=1000,
                                  deploy_step_size=1)
    assert status
    time.sleep(6)
    with open(master_ctrl.output) as fd:
        log = fd.read()
        assert log.find("deploy job using concurrent controller") != 0
        assert log.find("deploying with concurrent controller is done") != 0
    

def teardown():
    status,output,stderr = master_ctrl.stop()
    server.stop()
    if os.path.exists(CASE_TMP_FOLDER):
        utils.ShellHelper.run_with_returncode("rm -rf %s"%CASE_TMP_FOLDER)


