#!/bin/bash

# remove jobs
jobs=`./galaxy_client list | grep job | awk '{print $2}'`
if [ -n "$jobs" ]
    then
        for jobid in $jobs
        do
            ./galaxy_client remove -i $jobid
        done
        sleep 10
fi

# 1.kill appworker
# killall appworker

# 2.stop appmaster
killall appmaster

# 3.stop resman
killall resman

# 4.stop agent
killall agent

# 5.stop nexus
killall nexus

rm -rf ./nexus ./appmaster ./appworker ./resman ./agent ./galaxy_client ./galaxy_res_client
rm -rf ./nexus.flag ./galaxy.flag
rm -rf ./binlog *.log *.INFO* *.WARNING*
rm -rf ./data ./gc_dir/ ./work_dir/
