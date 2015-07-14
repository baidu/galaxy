# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-05-25

from bootstrap import settings
from galaxy import wrapper
from django.views.decorators.csrf import csrf_exempt
from common import http
from common import decorator as D
@D.api_auth_required
def list_tag(req):

    builder = http.ResponseBuilder()
    master_addr = req.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()

    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    try:
        ret = galaxy.list_tag()
        return builder.ok(data = ret).build_json()
    except:
        return builder.error('fail to tag agent ').build_json()

@csrf_exempt
@D.api_auth_required
def add_tag(req):
    builder = http.ResponseBuilder()
    master_addr = req.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    tag = req.POST.get('tag',None)
    if not tag :
        return builder.error('tag is required').build_json()
    agent_list = req.POST.get('agentList',"")
    agent_set = set(agent_list.splitlines()) 
    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    try:
        status = galaxy.tag_agent(tag, agent_set)
        if status :
            return builder.ok().build_json()
        return builder.error('fail to tag agent ').build_json()
    except:
        return builder.error('fail to tag agent ').build_json()



