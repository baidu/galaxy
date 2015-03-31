#!/usr/bin/env sh

mkdir -p /cgroup

mount -t tmpfs cgroup /cgroup

mkdir -p /cgroup/cpu

mount -t cgroup -ocpu cpu /cgroup/cpu

mkdir -p /cgroup/memory

mount -t cgroup -omemory memory /cgroup/memory
