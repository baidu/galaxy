# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
import logging
from common import shell

LOG = logging.getLogger("console")

class Galaxy(object):
    def __init__(self, master_addr,bin_path):
        self.master_addr = master_addr
        self.shell_helper = shell.ShellHelper()
        self.bin_path = bin_path
    def create_task(self,url,cmd_line,replicate_count):
        add_task_command = [self.bin_path,self.master_addr,'add',url,"'%s'"%cmd_line,replicate_count]
        code,stdout,stderr = self.shell_helper.run_with_retuncode(add_task_command)
        if code != 0 :
            LOG.error("fail to add task for "+stderr)
            return False
        return True




