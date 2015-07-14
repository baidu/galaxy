# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30

from galaxy import monitor_pb2
def build_monitor_conf(data):
    rules = []
    for rule in data['rules']:
        # config watch
        watch = monitor_pb2.WatchMsg()
        watch.name = rule['watch']['name']
        watch.regex = rule['watch']['regex']

        # config trigger
        trigger = monitor_pb2.TriggerMsg()
        trigger.name = rule['trigger']['name']
        trigger.threshold = rule['trigger']['threshold']
        trigger.range = 10

        # config action
        action = monitor_pb2.ActionMsg(send_list=rule['action']['send_list'])
        action.title = rule['action']['title']
        action.content = rule['action']['content']
        
        pb_rule = monitor_pb2.RuleMsg(watch=watch, trigger=trigger, action=action)
        pb_rule.input = data['logInput']
        rules.append(rule)
    conf = monitor_pb2.MonitorConfigMsg(rules = rules)
    return conf
