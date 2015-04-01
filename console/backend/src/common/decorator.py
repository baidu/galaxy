# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30

from common import http
def post_required(func):
    """
    http request post method decorator
    """
    def post_wrapper(request,*args,**kwds):
        res = http.ResponseBuilder()
        if request.method != 'POST':
            return res.error("post is required").build_json()
        return func(request,*args,**kwds)
    return post_wrapper


