# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-01
from django.conf import urls


urlpatterns = urls.patterns("console.cluster.views",
        (r'^status','get_status'),
)
