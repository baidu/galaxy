#!/usr/bin/env sh
port=$1
nohup ./bin/master -master_port=$port -task_deploy_timeout=60000 >> master.log 2>&1 &
echo "check port is alive"
for i in {1..10}
do
    nc -z 0.0.0.0 $port
    if [ "$?" == 0 ]; then
        echo "master is started"
        exit 0
    else
        sleep 1
    fi
done
echo "fail to start master"
exit -1
