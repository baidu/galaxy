# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-21
from galaxy import sdk

master_port = 9527
agent_port = 9529
def set_up():
    print "start master"
    print "start agent"


def test_list_node():
    galaxy_client = sdk.GalaxySDK("localhost:%d"%master_port)
    galaxy_client.list_all_node()

def clean():
    print "clean"
