# -*- coding:utf-8 -*-
from django.conf import urls

urlpatterns = urls.patterns('trace.views',
     (r'^job$', 'index'),
     (r'^job/all', 'job_all'),
     (r'^job/detail', 'job_detail'),
     (r'^pod/detail', 'pod_detail'),
     (r'^job/stat', 'job_stat'),
     (r'^job/event', 'job_event'),
     (r'^pod$', 'get_pod'),
     (r'^pod/stat', 'pod_stat'),
     (r'^pod/event', 'pod_event'),
     (r'^pod/task', 'task_event'),
     (r'^pod/allevent', 'get_pod_event_by_jobid'),
     (r'^query$', 'query'),
)

