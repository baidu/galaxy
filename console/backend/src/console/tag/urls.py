# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-05-25

from django.conf import urls

urlpatterns = urls.patterns("console.tag.views",
        (r'^list', 'list_tag'),
        (r'^add', 'add_tag'),
)
