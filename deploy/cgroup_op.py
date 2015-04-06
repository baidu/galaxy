# -*- coding:utf-8 -*-
from fabric import api

def umount():
    with api.settings(warn_only=True):
        api.run('umount /cgroups/cpu')
        api.run('umount /cgroups/memory')
        api.run('umount /cgroups')
        api.run('rm -rf /cgroups')

def mount():

    api.run("mkdir -p /cgroups")
    api.run('mount -t tmpfs cgroup /cgroups')
    api.run('mkdir -p /cgroups/cpu')
    api.run('mount -t cgroup -ocpu cpu /cgroups/cpu')
    api.run('mkdir -p /cgroups/memory')
    api.run('mount -t cgroup -omemory memory /cgroups/memory')
