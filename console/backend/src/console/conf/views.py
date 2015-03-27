# -*- coding:utf-8 -*-
from common import http
def get_conf(request):
    builder = http.ResponseBuilder()
    conf = {}
    return builder.ok(data = conf).build_json()


