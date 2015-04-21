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
from galaxy import sdk
master_port = 8102
def set_up():
    os.system("cd ../../sandbox && sh local-killall.sh >/dev/null 2>&1")
    os.system("cd ../../sandbox && sh local-run.sh >/dev/null 2>&1")
    time.sleep(5)

def test_list_node():
    galaxy_client = sdk.GalaxySDK("localhost:%d"%master_port)
    node_list = galaxy_client.list_all_node()

    assert len(node_list) == 3

def clean():
    os.system("cd ../../sandbox && sh local-killall.sh >/dev/null 2>&1")
