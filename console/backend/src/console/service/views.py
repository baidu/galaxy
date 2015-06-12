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
from common import decorator as D
from console.service import helper
from bootstrap import settings
from bootstrap import models
from console.quota import views
from galaxy import wrapper
from console.taskgroup import helper
LOG = logging.getLogger("console")

@D.api_auth_required
def list_service(request):
    """
    get current user's service list
    """
    builder = http.ResponseBuilder()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    groupmember_list = models.GroupMember.objects.filter(galaxy_master = master_addr,
                                                         user_name = request.user.username)
    group_id_list = []
    for group_m in groupmember_list:
        group_id_list.append(group_m.group.id)
    if not group_id_list:
        return builder.ok(data=[]).build_json()
    LOG.info(group_id_list)
    db_jobs = models.GalaxyJob.objects.filter(group__id__in = group_id_list)
    LOG.info(db_jobs)
    jobs_dict = {}
    for db_job in db_jobs:
        jobs_dict[db_job.job_id] = db_job
    client = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    status,jobs = client.list_jobs()
    LOG.info(status)
    if not status:
        return builder.error('fail to list jobs').build_json()
    ret = []
    for job in jobs:
        if job.job_id not in jobs_dict:
            continue
        ret.append(job.__dict__)
    return builder.ok(data=ret).build_json()


@csrf_exempt
@D.api_auth_required
def create_service(request):
    """
    create a service
    """
    builder = http.ResponseBuilder()
    group_id = request.POST.get('groupId',None)
    if not group_id:
        return builder.error('groupId is required').build_json()
    try:
        #判断是否有组权限
        group_member_iterator =  models.GroupMemeber.objects.filter(user_name=request.user.username, group__id=int(group_id))
        group_member = None
        for gm in group_member_iterator:
            group_member = gm
        if not group_member:
            return builder.error('group with %s does not exist'%group_id).build_json()
        ret = helper.validate_init_service_group_req(request)
        status,stat = views.get_group_quota_stat(group_member.group, {});
        cpu_total_require = ret['replicate_count'] * ret['cpu_share']
        mem_total_require = ret['memory_limit'] * 1024 * 1024 * 1024 * ret['replicate_count']
        if cpu_total_require > stat['total_cpu_left']:
            return builder.error('cpu %s exceeds the left cpu quota %s'%(cpu_total_require, stat['total_cpu_left'])).build_json()
        if mem_total_require > stat['total_mem_left']:
            return builder.error('mem %s exceeds the left mem %s'%(mem_total_require, stat['total_mem_left'])).build_json()

        galaxy = wrapper.Galaxy(gm.group.galaxy_master, 
                                settings.GALAXY_CLIENT_BIN)
        status,output = galaxy.create_task(ret['name'],ret['pkg_src'],
                                           ret['start_cmd'],
                                           ret['replicate_count'],
                                           ret['memory_limit']*1024*1024*1024,
                                           ret['cpu_share'],
                                           deploy_step_size = ret['deploy_step_size'],
                                           one_task_per_host = ret['one_task_per_host'],
                                           restrict_tags = ret['tag'])
        if not status:
            return builder.error('fail create task').build_json()
        galaxy_job = models.GalaxyJob(group = group_member.group , job_id=int(output))
        galaxy_job.save()
        return builder.ok().build_json()
    except Exception as e:
        return builder.error(str(e)).build_json()

@D.api_auth_required
def kill_service(request):
    builder = http.ResponseBuilder()
    id = request.GET.get('id',None)
    if not id:
        return builder.error('id is required').build_json()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    try:
        galaxy.kill_job(int(id))
        return builder.ok().build_json()
    except:
        return builder.error('fail to kill job %s'%id).build_json()

@D.api_auth_required
def update_service(request):
    builder = http.ResponseBuilder()
    id = request.GET.get('id',None)
    if not id:
        return builder.error('id is required').build_json()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    replicate = request.GET.get('replicate',None)
    if not replicate:
        return builder.error('replicate is required').build_json()

    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    status = galaxy.update_job(id,replicate)
    if status:
        return builder.ok().build_json()
    else:
        return builder.error('fail to kill job').build_json()

