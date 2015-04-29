# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-28
import subprocess
import os
import signal

class ShellHelper(object):

    @classmethod
    def run_with_returncode(cls, command,
                                universal_newlines=True,
                                useshell=True,
                                env=os.environ):
        """run with return code
        """
        try:
            p = subprocess.Popen(command,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE,
                                  shell=useshell,
                                  universal_newlines=universal_newlines,
                                  env=env)
            output, errout = p.communicate()
            return p.returncode, output, errout
        except Exception:
            return -1, None, None
    @classmethod
    def run_with_process(cls,command):
        p = subprocess.Popen(command,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            shell=True)
        return p

    @classmethod
    def kill_child_processes(cls,parent_pid, 
                                 sig=signal.SIGTERM):
        p = subprocess.Popen("ps -o pid --ppid %d --noheaders" % parent_pid, 
                                      shell=True,
                                      stdout=subprocess.PIPE)
        output, errout = p.communicate()
        retcode = p.returncode
        for pid_str in output.splitlines():
            os.kill(int(pid_str), sig)

class MasterCtrl(object):
    """
    master¿ØÖÆÆ÷
    """
    def __init__(self,master_bin,
                      run_path,
                      port,
                      output="./log"):
        self.master_bin = str(master_bin)
        self.run_path = str(run_path)
        self.port = port
        self.output = output
        self.master_process = None

    def start(self):
        if not os.path.exists(self.run_path):
            ShellHelper.run_with_returncode("mkdir -p %s"%self.run_path)
        os.chdir(self.run_path)
        start_cmd = "%s --port=%s >%s 2>&1"%(self.master_bin,self.port,self.output)
        self.master_process = ShellHelper.run_with_process(start_cmd)
        if self.master_process.poll() is None:
            print "%s has been started"%self.master_bin
            return True
        else: 
            print "fail to start %s "%self.master_bin
            self.master_process = None
            return False

    def stop(self):
        if not self.master_process:
            return -1,None,None
        ShellHelper.kill_child_processes(self.master_process.pid)
        output, errout = self.master_process.communicate()
        with open(self.output,"r") as fd:
            for line in fd.readlines():
                print line
        return self.master_process.returncode,output,errout
    
