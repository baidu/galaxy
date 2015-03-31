# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
import logging
from console import models
from common import http
from console.taskgroup import helper
from console.service import decorator as s_decorator
from console.taskgroup import decorator as t_decorator
from django.db import transaction
from django.views.decorators.csrf import csrf_exempt
LOG = logging.getLogger("console")
# service group 0
@csrf_exempt
@s_decorator.service_name_required
def init_service_group(request):
    LOG.info("init service %s default group"%request.service_name)
    builder = http.ResponseBuilder()
    try:
        ret = helper.validate_init_service_group_req(request)
        with transaction.atomic():
            # save group
            task_group = models.TaskGroup(offset=0,service_id=ret['service'].id)
            task_group.save() 
            # save package
            pkg = models.Package(pkg_type = ret["pkg_type"],pkg_src = ret["pkg_src"],
                                 version = request.POST.get("version",'1.0'))
            pkg.save()

            # save deloy
            deploy = models.DeployRequirement(replicate_count=ret['replicate_count'],
                                              pkg_id=pkg.id,start_cmd=ret['start_cmd'],
                                              task_group_id=task_group.id)
            deploy.save()

            # save memory  require
            memory = models.ResourceRequirement(deploy_id=deploy.id,r_type=models.MEMORY,
                                             name="memory.limit",value=ret['memory_limit'])
            memory.save()

            # save cpu
            cpu = models.ResourceRequirement(deploy_id=deploy.id,r_type=models.CPU,
                                             name="cpu.share",value=ret["cpu_share"])
            cpu.save()

            #call galaxy master api

            return builder.ok().build_json()
    except Exception as e:
        LOG.exception("init service group req is invalidate")
        return builder.error(str(e)).build_json()

@t_decorator.task_group_id_required
def update_task_group(request):
    pass



@s_decorator.service_name_required
def get_task_status(request):
    builder = http.ResponseBuilder()
    LOG.info("get service %s task status "%request.service_name)
    service = models.Service.get_by_name(request.service_name,9527)
    if not service:
        return builder.error("service with name %s does not exist"%request.service_name)\
                      .build_json()
    default_group = models.TaskGroup.get_default(service.id)
    if not default_group:
        return builder.ok(data={'needInit':True}).build_json()
    return builder.ok(data={'needInit':False,'taskList':[]}).build_json()
