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
MASTER_PORT = 11059
AGENT_PORT = 13460
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

def test_list_node():
    client = sdk.GalaxySDK("127.0.0.1:%d"%MASTER_PORT)
    node_list = client.list_all_node()
    assert len(node_list) == 1

def teardown():
    status,output,stderr = master_ctrl.stop()
    server.stop()
    if os.path.exists(CASE_TMP_FOLDER):
        utils.ShellHelper.run_with_returncode("rm -rf %s"%CASE_TMP_FOLDER)


