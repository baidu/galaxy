#! /bin/bash
set -e

./clear.sh

hn=`hostname`
echo "--master_port=7810" > galaxy.flag
echo "--master_host=localhost" >> galaxy.flag

echo "--nexus_servers=$hn:8868,$hn:8869,$hn:8870,$hn:8871,$hn:8872" >> galaxy.flag

./start_all.sh
sleep 3

../galaxy submit job1 job_pkg 3 3 3 "echo haha"
