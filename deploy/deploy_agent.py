# -*- coding:utf-8 -*-
from fabric import api


def fetch(ftp):
    api.run('mkdir -p /home/galaxy/agent')
    with api.cd('/home/galaxy/agent'):
        with api.settings(warn_only=True):
            api.run('rm galaxy.tar.gz')
        api.run('wget %s'%ftp)
        api.run('tar -zxvf galaxy.tar.gz')

def stop(port):
    with api.cd("/home/galaxy/agent/galaxy"):
        api.run('sh ./bin/stop-agent.sh %s'%port)


def start(port,master,mem=128,cpu=64):
    with api.cd('/home/galaxy/agent/galaxy'):
        api.run('sh ./bin/start-agent.sh %s %s %s %s '%(master,port,mem,cpu))

def migrate(ftp,port,master,mem=128,cpu=64):
    stop(port)
    fetch(ftp)
    start(port,master,mem=mem,cpu=cpu)

def first(ftp,port,master,mem=128,cpu=64):
    fetch(ftp)
    start(port,master,mem=mem,cpu=cpu)

def fixlib():
    with api.cd("/usr/lib64"):
        with api.settings(warn_only=True):
            api.run('ln -s liblber-2.3.so.0.2.31 liblber-2.2.so.7')
