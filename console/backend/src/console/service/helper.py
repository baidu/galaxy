# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30

def service_to_dict(service):
    ret = {}
    ret["id"] = service.id
    ret["user_id"] = service.user_id
    ret["name"] = service.name
    ret["create_date"] = service.create_date.strftime('%Y-%m-%d')
    ret["modify_date"] = service.modify_date.strftime('%Y-%m-%d')
    return ret
