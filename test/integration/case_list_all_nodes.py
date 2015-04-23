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
master_port = 9876
def set_up():
    pass
def test_list_node():
    galaxy_client = sdk.GalaxySDK("yq01-tera81.yq01.baidu.com:%d"%master_port)
    node_list = galaxy_client.list_all_node()

    assert len(node_list) == 7

def clean():
    pass
