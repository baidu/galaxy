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
from galaxy import sdk
from nose import tools

CASE_TMP_FOLDER = os.sep.join([os.environ.get("GALAXY_CASE_FOLDER"),"case_job_test"])
MASTER_BIN_PATH = os.environ.get("MASTER_BIN_PATH")
MASTER_PORT = 13456
TASK_SCRIPT="""#!/usr/bin/env sh
for ((i=0;i<1000;i++)) do
    echo $i;
    sleep 1;
done;
"""
def setup():
    global master_ctrl
    global package
    master_ctrl = utils.MasterCtrl(MASTER_BIN_PATH,CASE_TMP_FOLDER,MASTER_PORT) 
    ret = master_ctrl.start()
    if not ret :
        print "fail to start master"
        assert False
    time.sleep(1)
    with open("task.sh","w") as fd:
        fd.write(TASK_SCRIPT)
    utils.ShellHelper.run_with_returncode("tar -zxvf task.sh.tar.gz task.sh && cp task.sh.tar.gz /tmp")
    package = "ftp://%s/tmp/task.sh.tar.gz"%socket.gethostname()

def test_create_job():
    client = sdk.GalaxySDK("127.0.0.1:%d"%MASTER_PORT)
    ret,job_id = client.make_job("task","ftp",package,"sh task.sh",replicate_num=0)
    assert job_id >= 0
    status,job_list = client.list_all_job()
    assert status
    assert len(job_list) == 1
    client.kill_job(job_id)
    time.sleep(1)
    status,job_list = client.list_all_job()
    assert status
    assert len(job_list) == 0

def teardown():
    status,output,stderr = master_ctrl.stop()
    if os.path.exists(CASE_TMP_FOLDER):
        utils.ShellHelper.run_with_returncode("rm -rf %s"%CASE_TMP_FOLDER)


