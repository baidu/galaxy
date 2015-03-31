# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
import logging
from django.views.decorators.csrf import csrf_exempt
from console.service import decorator as service_decorator
from common import http
from common import decorator as com_decorator
from console import models
from console.service import helper
LOG = logging.getLogger("console")
@service_decorator.user_required
def list_service(request):
    """
    get current user's service list
    """
    builder = http.ResponseBuilder()
    ret_service_list = []
    for m_service in  models.Service.get_by_user(request.user_id):
        ret_service_list.append(helper.service_to_dict(m_service))
    return builder.ok(data=ret_service_list).build_json()

@csrf_exempt
@com_decorator.post_required
@service_decorator.user_required
def create_service(request):
    """
    create a service
    """
    builder = http.ResponseBuilder()
    name = request.POST.get("name",None)
    if not name :
        return builder.error("name is required").build_json()
    service = models.Service(name=name,user_id = request.user_id)
    service.save()
    if service.id is not None:
        return builder.ok().build_json()
    return builder.error("fail to save service %s"%name).build_json()

@service_decorator.service_id_required
def delete_service(request):
    builder = http.ResponseBuilder()
    service = models.Service.get_by_id(request.service_id)
    if not service:
        return builder.error("service with id %s does not exist"%request.service_id).build_json()
    service.is_delete = True
    service.save()
    return builder.ok().build_json()
