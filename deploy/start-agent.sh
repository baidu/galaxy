#!/usr/bin/env sh
master=$1
port=$2
mem=$3
cpu=$4
nohup ./bin/agent --container=cgroup --work_dir=./ --master=$master --port=$port --mem=$3 --cpu=$4 >agent.log 2>&1&
 
