# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
from django.conf import urls

#views
urlpatterns = urls.patterns("console.taskgroup.views",
        (r'^status','get_task_status'),
        (r'^history','get_job_sched_history'),
)

