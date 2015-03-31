# -*- coding:utf-8 -*-
import logging
from django.db import models

LOG = logging.getLogger("console")
# resource type
CPU = 0
MEMORY = 1
REOUCE_TYPE_LIST=[CPU,MEMORY]
# pkg type
FTP = 0
HTTP = 1
P2P = 2
RAW = 3
PKG_TYPE_LIST = [FTP,HTTP,P2P,RAW]
class Service(models.Model):
    """
    service model 
    """
    name = models.CharField(max_length=100)
    user_id = models.IntegerField()
    create_date = models.DateTimeField(auto_now_add=True)
    modify_date = models.DateTimeField(auto_now=True)
    is_delete = models.BooleanField(default=False)
    @classmethod
    def get_by_user(cls,user_id):
        if not user_id:
            return []
        return cls.objects.filter(user_id=user_id,is_delete=False)

    @classmethod
    def get_by_id(cls,id):
        try:
            return cls.objects.get(pk = id)
        except Exception as e:
            LOG.exception("fail to query service with %d"%id)
            return None
    @classmethod
    def get_by_name(cls,name,user_id):
        if not name or not user_id:
            return None
        query_set = cls.objects.filter(user_id=user_id,name=name)[0:1]
        for result in query_set:
            return result
        return None

class TaskGroup(models.Model):
    offset = models.IntegerField()
    service_id = models.IntegerField()
    def is_default(self):
        if offset == 0:
            return True
        return False
    @classmethod
    def get_default(cls,service_id):
        groups = cls.objects.filter(service_id=service_id,offset=0)[0:1]
        for g in groups:
            return g
        return None

class Task(models.Model):
    task_id = models.IntegerField()
    task_group_id = models.IntegerField()

class DeployRequirement(models.Model):

    replicate_count = models.IntegerField()
    pkg_id = models.IntegerField()
    task_group_id = models.IntegerField()
    start_cmd = models.TextField()
    create_date = models.DateTimeField(auto_now_add=True)
    modify_date = models.DateTimeField(auto_now=True)

class Package(models.Model):
    pkg_type = models.IntegerField()
    #currently location is a file path
    pkg_src = models.TextField()
    version = models.TextField()
    create_date = models.DateTimeField(auto_now_add=True)
    modify_date = models.DateTimeField(auto_now=True)


class ResourceRequirement(models.Model):
    deploy_id = models.IntegerField()
    #cpu or memory
    r_type = models.IntegerField()
    #limit 
    name = models.CharField(max_length=100)
    value = models.IntegerField()
    create_date = models.DateTimeField(auto_now_add=True)
    modify_date = models.DateTimeField(auto_now=True)


