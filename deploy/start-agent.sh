#!/usr/bin/env sh
master=$1
port=$2
nohup ./bin/agent -container=cgroup -work_dir=./ -master=$master -port=$port --mem=128 --cpu=64  >agent.log 2>&1
 
