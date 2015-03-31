# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
import logging
from common import http
LOG = logging.getLogger("console")
def user_required(func):
    def user_wrapper(request,*args,**kwds):
        builder = http.ResponseBuilder()
        user_id = request.GET.get("user",None) or request.POST.get("user",None)
        if not user_id:
            return builder.error("user id required").build_json()
        request.user_id = user_id
        return func(request,*args,**kwds)
    return user_wrapper

def service_id_required(func):
    def service_id_wrapper(request, *args, **kwds):
        builder = http.ResponseBuilder()
        service_id = request.GET.get("service",None) or request.POST.get("service",None)
        if not service_id:
            return builder.error("service is required").build_json()
        try:
            request.service_id = int(service_id)
        except Exception as e:
            LOG.exception("fail to convert service to int")
            return builder.error("fail to convert service to int for %s"%e)
        return func(request, *args, **kwds)
    return service_id_wrapper

def service_name_required(func):
    def service_name_wrapper(req, *args, **kwds):
        builder = http.ResponseBuilder()
        service_name = req.GET.get('serviceName',None) or req.POST.get('serviceName',None)
        if not service_name :
            return builder.error("serviceName is required").build_json()
        req.service_name = service_name
        return func(req, *args, **kwds)
    return service_name_wrapper
 
