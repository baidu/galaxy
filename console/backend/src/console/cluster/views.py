# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-04-01

from bootstrap import settings
from common import http
from galaxy import wrapper
SHOW_G_BYTES_LIMIT = 1024 * 1024 * 1024

def str_pretty(total_bytes):
    if total_bytes < SHOW_G_BYTES_LIMIT:
        return "%sM"%(total_bytes/(1024*1024))
    return "%sG"%(total_bytes/(1024*1024*1024))


def get_status(req):
    builder = http.ResponseBuilder()
    master_addr = req.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()

    client = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    machine_list = client.list_node()
    ret = []
    total_node_num = 0
    total_cpu_num = 0
    total_cpu_used = 0
    total_cpu_real_used = 0
    total_mem_real_used = 0
    total_mem_num = 0
    total_mem_used = 0
    total_task_num = 0
    total_node_num = len(machine_list)
    for machine in machine_list:
        total_cpu_num += machine.cpu_share
        total_mem_num += machine.mem_share
        total_task_num += machine.task_num
        total_cpu_used += machine.cpu_used
        total_mem_used += machine.mem_used
        total_mem_real_used += machine.mem_real_used
        total_cpu_real_used += machine.cpu_real_used 
        machine.mem_share = str_pretty(machine.mem_share)
        machine.mem_used = str_pretty(machine.mem_used)
        machine.cpu_used = '%0.2f'%machine.cpu_used
        ret.append(machine.__dict__)
    return builder.ok(data={'machinelist':ret,
                                'total_node_num':total_node_num,
                                'total_mem_used':str_pretty(total_mem_used),
                                'total_cpu_used':"%0.2f"%total_cpu_used,
                                'total_cpu_num':total_cpu_num,
                                'total_mem_num':str_pretty(total_mem_num),
                                'total_task_num':total_task_num,
                                'total_mem_real_used':str_pretty(total_mem_real_used),
                                'total_cpu_real_used':"%0.2f"%total_cpu_real_used}).build_json()
def real_time_usage(req):
    builder = http.ResponseBuilder()
    master_addr = req.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    client = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    machine_list = client.list_node()

