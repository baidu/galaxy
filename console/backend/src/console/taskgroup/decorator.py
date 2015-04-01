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

def task_group_id_required(func):
    def task_group_id_wrapper(req,*args,**kwds):
        builder = http.ResponseBuilder()
        task_group_id = req.GET.get("taskGroupId",None) or req.POST.get("taskGroupId",None)
        if not task_group_id:
            return builder.error("taskGroupId is required").build_json()
        try:
            req.task_group_id = int(task_group_id)
        except:
            LOG.exception("fail to convert task group id %s"%task_group_id)
            return builder.error("taskGroupId is invalide").build_json()
        return func(req,*args,**kwds)
    return task_group_id_wrapper
