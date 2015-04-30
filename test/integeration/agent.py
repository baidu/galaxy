# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-30
import rpc_server
import thread
import socket
import time
import threading
from galaxy import agent_pb2
from galaxy import master_pb2
from galaxy import task_pb2
from sofa.pbrpc import client
#mock agent
class AgentImpl(agent_pb2.Agent):
    def __init__(self,cpu,mem,port,master_addr):
        self._cpu = cpu
        self._mem = mem
        self._port = port
        self._master_addr = master_addr
        self._mutex = thread.allocate_lock()
        self._channel = client.Channel(master_addr)
        self._version = 0
        self._my_addr = "%s:%d"%(socket.gethostname(),port)
        self._agent_id = -1
        self._task_status = {}
    def HeartBeat(self):
        while True:
            with self._mutex:
                master = master_pb2.Master_Stub(self._channel)
                controller = client.Controller()
                controller.SetTimeout(100)
                req = master_pb2.HeartBeatRequest()
                req.cpu_share = self._cpu
                req.mem_share = self._mem
                req.version = self._version
                req.agent_addr = self._my_addr
                status_list = []
                for key in self._task_status:
                    print "running task %s "%key
                    status_list.append(self._task_status[key])
                req.task_status.extend(status_list)
                response = master.HeartBeat(controller,req)
                self._agent_id = response.agent_id
                self._version  = response.version
                print "heart beat version %s agent %s"%(self._version,self._agent_id)
                time.sleep(1)

    def RunTask(self,ctrl,req,done):
        with self._mutex:
            print "run task %d"%req.task_id
            response = agent_pb2.RunTaskResponse()
            response.status = 0
            status = task_pb2.TaskStatus()
            status.task_id = req.task_id
            status.cpu_usage = 0
            status.status = 2
            status.memory_usage = 0
            self._task_status[status.task_id] = status
            return response

    def KillTask(self,ctrl,req,done):
        with self._mutex:
            del self._task_status[req.task_id]
            response = agent_pb2.KillTaskResponse()
            response.status = 0
            print "kill task %d"%req.task_id
            return response 

if __name__ == "__main__":
    agent = AgentImpl(5,1024*1024*1024*10,9527,"localhost:8102")
    heartbeat_t = threading.Thread(target=agent.HeartBeat)
    heartbeat_t.daemon = True
    heartbeat_t.start()
    server = rpc_server.RpcServer(9527,host="0.0.0.0")
    try:
        server.add_service(agent)
        server.start()
    except KeyboardInterrupt:
        server.stop()

