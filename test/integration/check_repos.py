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

def run_cmd(cmd):
    try:
        p = subprocess.Popen(cmd,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        stdout,stderr = p.communicate()
        return p.returncode,stdout,stderr
    except:
        -1 ,None,None

def check_github_has_update():
    code,stdout,stderr = run_cmd(["git","fetch","--dry-run"])
    if code != 0:
        print stdout
        print stderr
        return False
    if stdout == ""
        return False
    return True


