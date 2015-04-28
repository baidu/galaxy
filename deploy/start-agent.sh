#!/usr/bin/env sh
master=$1
port=$2
mem=$3
cpu=$4
nohup ./bin/agent --container=cgroup --work_dir=./ --master=$master --port=$port --mem=$3 --cpu=$4 --user=root >agent.log 2>&1&
echo "check port is alive"
for i in {1..10}
do
    nc -z 0.0.0.0 $port
    if [ "$?" == 0 ]; then
        echo "agent is started"
        exit 0
    else
        sleep 1
    fi
done
echo "fail to start agent"
exit -1
