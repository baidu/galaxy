# -*- coding:utf-8 -*-
from common import http
def get_conf(request):
    builder = http.ResponseBuilder()
    home = [{"nodeName":"服务列表","href":"#service","nodestyle":["level1"],"children":[]},
    {"nodeName":"集群信息","href":"#cluster","nodestyle":["level1"],"children":[]},
    {"nodeName":"配置集群","href":"#setup","nodestyle":["level1"],"children":[]}]
    service = [{ "nodeName" : "Tera", "nodestyle":["level1"], "children" : [ { "nodestyle":["level2"],"subpage":"status","nodeName" : "服务状态", "children" : []}] }]
    return builder.ok(data = {'home':home,'service':service}).build_json()


