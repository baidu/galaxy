# -*- coding:utf-8 -*-

from django.db import models
class Service(models.Model):
    name = models.CharField(max_length=100)
