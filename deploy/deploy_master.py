# -*- coding:utf-8 -*-
from fabric import api
api.env.hosts=['yq01-tera81.yq01.baidu.com']

def fetch(ftp):
    api.run('mkdir -p /home/galaxy/master')
    with api.cd('/home/galaxy/master'):
        with api.settings(warn_only=True):
            api.run('rm galaxy.tar.gz')
        api.run('wget %s'%ftp)
        api.run('tar -zxvf galaxy.tar.gz')

def stop():
    with api.cd('/home/galaxy/master/galaxy'):
        api.run('sh ./bin/stop-master.sh')


def start(port):
    with api.cd('/home/galaxy/master/galaxy'):
        api.run('sh ./bin/start-master.sh %s'%port)


def migrate(ftp,port):
    stop()
    fetch(ftp)
    start(port)

def first(ftp,port):
    fetch(ftp)
    start(port)



