# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
import datetime

from django.contrib import admin
from django.db import models

class Quota(models.Model):
    name = models.CharField(max_length=500)
    cpu_total_limit = models.FloatField()
    mem_total_limit = models.BigIntegerField()
    created = models.DateTimeField(auto_now_add=True)
    modified = models.DateTimeField(auto_now=True)

    def __str__(self):
        return "quota_%s_%d"%(self.name, self.id)

class QuotaAdmin(admin.ModelAdmin):
    list_display = ('id',
                    'name',
                    'cpu_total_limit', 
                    'mem_total_limit',
                    'created', 
                    'modified')
    search_fields = ('id',)

    class Meta:
        pass

    class Admin:
        pass

admin.site.register(Quota, QuotaAdmin)

class Group(models.Model):
    galaxy_master = models.CharField(max_length=500)
    quota = models.ForeignKey(Quota)
    name = models.CharField(max_length=500)
    created = models.DateTimeField(auto_now_add=True)
    modified = models.DateTimeField(auto_now=True)
    description = models.TextField()
    max_cpu_limit = models.FloatField()
    def __str__(self):
        return "group_%s_%d"%(self.name, self.id)



class GroupAdmin(admin.ModelAdmin):
    list_display = ('id',
                    'quota' ,
                    'description', 
                    'created', 
                    'modified',
                    'max_cpu_limit')

    search_fields = ('id','quota__id')
    class Meta:
        pass

    class Admin:
        pass
admin.site.register(Group, GroupAdmin)

class GroupMember(models.Model):
    user_name = models.CharField(max_length=500) 
    group = models.ForeignKey(Group)
    created = models.DateTimeField(auto_now_add=True)
    modified = models.DateTimeField(auto_now=True)

    def __str__(self):
        return "member_%s_%d"%(self.user_name,self.id)

class GroupMemberAdmin(admin.ModelAdmin):
    list_display = ('id',
                    'user_name',
                    'group', 
                    'created', 
                    'modified')

    search_fields = ('id','user_name','group__id','created','modified')
    class Meta:
        pass

    class Admin:
        pass
admin.site.register(GroupMember, GroupMemberAdmin)
class GalaxyJob(models.Model): 
    group = models.ForeignKey(Group)
    job_id = models.IntegerField()
    created = models.DateTimeField(auto_now_add=True)
    modified = models.DateTimeField(auto_now=True)
    meta = models.TextField()
    def __str__(self):
        return "galaxy_job_%d"%self.job_id

class GalaxyJobAdmin(admin.ModelAdmin):
    list_display = ('id',
                    'group', 
                    'job_id',
                    'created', 
                    'modified')

    search_fields = ('id','group__id','job_id')
    class Meta:
        pass

    class Admin:
        pass

admin.site.register(GalaxyJob, GalaxyJobAdmin)
