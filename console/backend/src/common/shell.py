# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
import os
import subprocess
import sys
import time


USE_SHELL = sys.platform.startswith("win")

class ShellHelper(object):

    def run_with_retuncode(self, command,
                                universal_newlines=True,
                                useshell=USE_SHELL):
      
        try:
            p = subprocess.Popen(command,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE,
                                  shell=useshell,
                                  universal_newlines=universal_newlines)
            output, errout = p.communicate()
            return p.returncode, output, errout
        except Exception:
            return -1, None, None

    def run_with_realtime_print(self, command,
                                universal_newlines=True,
                                useshell=USE_SHELL,
                                env=os.environ,
                                print_output=True):
        """run with realtime print
        """
        try:
            p = subprocess.Popen(command,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT,
                                  shell=useshell,
                                  env=env)
            for line in iter(p.stdout.readline, ''):
                if print_output:
                    sys.stdout.write(line)
                else:
                    sys.stdout.write('\r')
                    sys.stdout.write(self.loader.next_label())
            sys.stdout.write('\r')
            p.wait()
            return p.returncode
        except Exception:
            return -1

    