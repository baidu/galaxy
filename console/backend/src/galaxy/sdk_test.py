# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-06

import logging
from galaxy import sdk

def test_tag_agent():
    master = sdk.GalaxySDK('localhost:8102')
    status = master.tag_agent('dev',set(['cq01-rdqa-pool056.cq01.baidu.com:8201']))
    print status

def list_tag():
    master = sdk.GalaxySDK('localhost:8102')
    ret = master.list_tag()
    print ret


if __name__ == "__main__":
    test_tag_agent()
    list_tag()

