# -*- coding:utf-8 -*-
from fabric import api


def fetch(ftp):
    api.run('mkdir -p /home/galaxy/agent')
    with api.cd('/home/galaxy/agent'):
        with api.settings(warn_only=True):
            api.run('rm galaxy.tar.gz')
        api.run('wget %s'%ftp)
        api.run('tar -zxvf galaxy.tar.gz')

def stop():
    with api.cd("/home/galaxy/agent"):
        api.run('sh ./bin/stop-agent.sh')


def start(port,master):
    with api.cd('/home/galaxy/agent/galaxy'):
        api.run('sh ./bin/start-agent.sh %s %s'%(master,port))


def migrate(ftp,port,master):
    stop()
    fetch(ftp)
    with api.cd('/home/galaxy/agent'):
         api.run('tar -zxvf galaxy.tar.gz')
    start(port,master)

def first(ftp,port,master):
    fetch(ftp)
    start(port,master)

def fixlib():
    with api.cd("/usr/lib64"):
        with api.settings(warn_only=True):
            api.run('ln -s liblber-2.3.so.0.2.31 liblber-2.2.so.7')
