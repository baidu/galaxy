#!/usr/bin/env sh
pid_list=`ps -ef | grep agent | grep work_dir | grep 12345  | grep -v grep | awk '{print $2}'`
if [ -n "$pid_list" ];then
   echo "[INFO] close the existing $1"
   for pid in $pid_list;do
        kill  $pid
        sleep 1 
        kill -0 $pid >/dev/null 2>&1
        ret=$?;if [[ $ret == 0 ]];then kill -9 $pid ;fi
        echo "[INFO] $1 with pid $pid closed"
   done
   echo "[INFO] close agent successfully"
else
   echo "[INFO] no agent is existing"
fi  


