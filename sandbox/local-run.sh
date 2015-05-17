#!/usr/bin/env sh
cur_user=`whoami`
echo "start master "
nohup ../output/bin/master --master_port=8102 >./master.log 2>&1 &
sleep 1
echo "start agent"
nohup ../output/bin/agent --agent_work_dir=./agent1/ --agent_port=8201 --agent_http_port=8088 --master_addr=localhost:8102 --mem_gbytes=64 --cpu_num=2 --task_acct=$cur_user >./agent1.log 2>&1 &
nohup ../output/bin/agent --agent_work_dir=./agent2/ --agent_port=8202 --agent_http_port=8089 --master_addr=localhost:8102 --mem_gbytes=4 --cpu_num=3 --task_acct=$cur_user >./agent2.log 2>&1 &
nohup ../output/bin/agent --agent_work_dir=./agent3/ --agent_port=8203 --agent_http_port=8090 --master_addr=localhost:8102 --mem_gbytes=16 --cpu_num=2 --task_acct=$cur_user >./agent3.log 2>&1 &
#use cgroup
#nohup ../output/bin/agent --port=8202 --container=cgroup --master=localhost:8102 >/tmp/agent.log 2>&1 &

