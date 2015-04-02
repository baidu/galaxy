# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
from console import models

class ValidateException(Exception):
    pass


def validate_init_service_group_req(request):
    ret = {}
    # package validate logic
    pkg_type_str = request.POST.get("pkgType",None)
    if not pkg_type_str:
        raise ValidateException("package type is required")
    pkg_type = int(pkg_type_str)
    ret["pkg_type"] = pkg_type
    if pkg_type not in models.PKG_TYPE_LIST:
        raise ValidateException("pkg_type is invalide type")
    pkg_src = request.POST.get("pkgSrc",None)
    if not pkg_src:
        raise ValidateException("pkgSrc is required")
    ret['pkg_src'] = pkg_src

    ret['start_cmd'] = request.POST.get("startCmd",None)
    if not ret['start_cmd']:
        raise ValidateException("startCmd is required")
    #cpu validate logic
    cpu_share_str = request.POST.get("cpuShare",None)
    if not cpu_share_str:
        raise ValidateException("cpu_share is required")
    cpu_share =float(cpu_share_str)
    ret['cpu_share'] = cpu_share

    #memory validate logic
    memory_limit_str = request.POST.get("memoryLimit",None)
    if not memory_limit_str:
        raise ValidateException("memoryLimit is required")
    ret['memory_limit'] = int(memory_limit_str)
    # replicate count
    replicate_count_str = request.POST.get("replicate",None)
    if not replicate_count_str:
        raise ValidateException("replicate is required")
    ret['replicate_count'] = int(replicate_count_str)
    if ret['replicate_count'] < 0:
        raise ValidateException("replicate_count is invalidate")
    return ret
