#!/usr/bin/env sh
echo "start master "
nohup ../output/bin/master --port=8102 >./master.log 2>&1 &
sleep 1
echo "start agent"
nohup ../output/bin/agent --work_dir=./agent1/ --port=8201 --http-port=8088 --master=localhost:8102 --mem=64 --cpu=2 >./agent1.log 2>&1 &
nohup ../output/bin/agent --work_dir=./agent2/ --port=8202 --http-port=8089 --master=localhost:8102 --mem=4 --cpu=3 >./agent2.log 2>&1 &
nohup ../output/bin/agent --work_dir=./agent3/ --port=8203 --http-port=8090 --master=localhost:8102 --mem=16 --cpu=2 >./agent3.log 2>&1 &
#use cgroup
#nohup ../output/bin/agent --port=8202 --container=cgroup --master=localhost:8102 >/tmp/agent.log 2>&1 &

