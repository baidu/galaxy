# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
import os
import time
import logging
from SOAPpy import WSDL
from SOAPpy import headerType
from django import shortcuts
from bootstrap import settings
from common import http
from django.contrib import auth
from django.contrib.auth import models
import xml.etree.ElementTree as ET
LOG = logging.getLogger("console")

def auto_login_required(func):
    def auto_login_wrapper(request, *args, **kwds):
        if request.user.is_authenticated():
            return func(request, *args, **kwds)
        ticket = request.GET.get('ticket',None)
        cas_url = "%s?service=%s"%(settings.UUAP_CAS_SERVER, settings.MY_HOST)
        if not ticket:
            LOG.info("redirect to %s"%cas_url)
            return shortcuts.redirect(cas_url)
        else:
            user = auth.authenticate(ticket = ticket, service = settings.MY_HOST)
            if not user:
                return shortcuts.redirect(cas_url)
            else:
                auth.login(request, user)
                return func(request, *args, **kwds)
    return auto_login_wrapper



def auth_ticket(ticket, my_url):
    """
    <cas:serviceResponse xmlns:cas='http://www.yale.edu/tp/cas'>
    <cas:authenticationSuccess>
    <cas:user>wangtaize</cas:user>
    </cas:authenticationSuccess>
    </cas:serviceResponse>
    """
    client = http.HttpClient()
    auth_url = settings.UUAP_VALIDATE_URL
    response = client.do_post(auth_url,
                              [('service',my_url),('ticket',ticket)],
                              content_to_file = False)
    if response['error'] is None:
        root = ET.fromstring(response['content'])
        success = root.find('{http://www.yale.edu/tp/cas}authenticationSuccess')
        if success is not None:
            user = success.find('{http://www.yale.edu/tp/cas}user')
            if user is not None:
                return user.text
    return None


class UUAPBackend(object):
    def __init__(self):
        server = WSDL.Proxy(settings.UIC_SERVICE)
        hd = headerType(data={"appKey":settings.UIC_KEY})
        server.soapproxy.header = hd
        self.server = server 
    def authenticate(self, ticket=None, service=None):
        username = auth_ticket(ticket, service)
        if not username:
            return None
        i_user = list(models.User.objects.filter(username=username))
        if not i_user:
            uic_user = server.getUserByUsername(arg0 = username)
            if hasattr(uic_user,'username'):
                new_user = models.User(username = uic_user.username,
                                       email = uic_user.email)
                new_user.save()
                return new_user
            return None
        return i_user[0]
    def get_user(self, user_id):
        try:
            return models.User.objects.get(pk=user_id)
        except models.User.DoesNotExist:
            return None
