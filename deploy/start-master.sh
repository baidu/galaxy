#!/usr/bin/env sh
port=$1
nohup ./bin/master --port=$port --task_deploy_timeout=60000 >master.log 2>&1 &
