# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-06
import datetime
import logging
from sofa.pbrpc import client
from galaxy import agent_pb2

LOG = logging.getLogger('console')
class BaseEntity(object):
    def __setattr__(self,name,value):
        self.__dict__[name] = value
    def __getattr__(self,name):
        return self.__dict__.get(name,None)

class Agent(object):
    """
    galaxy python sdk
    """
    def __init__(self, agent_addr):
        self.channel = client.Channel(agent_addr)

    def set_password(self, user, password):
        """
        set password
        return:
              if error ,None will be return
        """
        agent_stub = agent_pb2.Agent_Stub(self.channel)
        controller = client.Controller()
        controller.SetTimeout(1.5)
        request = agent_pb2.SetPasswordRequest()
        request.user_name = user
        request.password = password
        try:
            response = agent_stub.SetPassword(controller, request)
            if not response:
                return -1;
            return response.status
        except:
            LOG.exception("fail to call set password")
        return -10

 
