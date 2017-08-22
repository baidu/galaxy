#!/bin/bash

HOSTNAME=`hostname`
SERVERS="$HOSTNAME:8868,$HOSTNAME:8869,$HOSTNAME:8870,$HOSTNAME:8871,$HOSTNAME:8872"
ROOTPATH=`pwd`
IP=`hostname -i`

function assert_process_ok() {
    ps xf | grep "$1" | grep -v grep >/dev/null
    if [ $? -eq 0 ]; then
        echo "$1 ok"
    else
        echo "$1 err"
        exit 1
    fi
}

# copy files
cp -rf ../thirdsrc/ins/output/bin/nexus ./
cp -rf ../resman ./
cp -rf ../agent ./
cp -rf ../appmaster ./
cp -rf ../appworker ./
cp -rf ../galaxy_client ./
cp -rf ../galaxy_res_client ./

# prepare nexus.flag
echo "--cluster_members=$SERVERS" >nexus.flag

# 1.start nexus
echo "start nexus"
nohup ./nexus --flagfile=nexus.flag --server_id=1 >/dev/null 2>&1 &
nohup ./nexus --flagfile=nexus.flag --server_id=2 >/dev/null 2>&1 &
nohup ./nexus --flagfile=nexus.flag --server_id=3 >/dev/null 2>&1 &
nohup ./nexus --flagfile=nexus.flag --server_id=4 >/dev/null 2>&1 &
nohup ./nexus --flagfile=nexus.flag --server_id=5 >/dev/null 2>&1 &
sleep 10
assert_process_ok "./nexus --flagfile=nexus.flag"

# prepare galaxy.flag
echo "--logbufsecs=0"                                                             >> galaxy.flag
echo "--alsologtostderr=1"                                                        >> galaxy.flag
echo "--username=test"                                                            >> galaxy.flag
echo "--token=test"                                                               >> galaxy.flag
echo "# resman"                                                                   >> galaxy.flag
echo "--nexus_addr=$SERVERS"                                                      >> galaxy.flag
echo "--nexus_root=/galaxy_test"                                                  >> galaxy.flag
echo "--resman_path=/resman"                                                      >> galaxy.flag
echo "--safe_mode_percent=0.7"                                                    >> galaxy.flag
echo "--sched_interval=10"                                                        >> galaxy.flag
echo "--agent_query_interval=2"                                                   >> galaxy.flag
echo "# appmaster"                                                                >> galaxy.flag
echo "--appmaster_port=8123"                                                      >> galaxy.flag
echo "# agent"                                                                    >> galaxy.flag
echo "--nexus_root_path=/galaxy_test"                                             >> galaxy.flag
echo "--nexus_servers=$SERVERS"                                                   >> galaxy.flag
echo "--cmd_line=appworker --nexus_addr=$SERVERS --nexus_root_path=/galaxy_test"  >> galaxy.flag
echo "--cpu_resource=4000"                                                        >> galaxy.flag
echo "--memory_resource=4294967296"                                               >> galaxy.flag
echo "--mount_templat=/bin,/boot,/cgroups,/dev,/etc,/lib,/lib64,/lost+found,/media,/misc,/mnt,/opt,/sbin,/selinux,/srv,/sys,/tmp,/usr,/var,/noah,/noah/download,/noah/modules,/noah/tmp,/noah/bin,/proc,/cgroups/cpu,/cgroups/memory,/cgroups/cpuacct,/cgroups/tcp_throt,/cgroups/blkio,/cgroups/freezer,/cgroups/net_cls,/home/opt" >> galaxy.flag
echo "--agent_port=1025"                                                         >> galaxy.flag
echo "--master_path=/resman"                                                     >> galaxy.flag
echo "--galaxy_root_path=$ROOTPATH"                                              >> galaxy.flag
echo "--cgroup_root_path=/cgroups"                                               >> galaxy.flag
echo "--gc_delay_time=86400"                                                     >> galaxy.flag
echo "--agent_ip=$IP"                                                            >> galaxy.flag
echo "--agent_hostname=$HOSTNAME"                                                >> galaxy.flag
echo "--volum_resource=/dev/vdb:2906620387328:DISK:/home/galaxy"                 >> galaxy.flag

# 2. start resman
echo "start resman"
nohup ./resman --flagfile=galaxy.flag >resman.log 2>&1 &
sleep 5
assert_process_ok "./resman"

# 3.start appmaster
echo "start appmaster"
nohup ./appmaster --flagfile=galaxy.flag >appmaster.log 2>&1 &
sleep 5
assert_process_ok "./appmaster"

# 4.start agent
echo "start agent"
export PATH=$PATH:$ROOTPATH
nohup ./agent --flagfile=galaxy.flag >agent.log 2>&1 &
assert_process_ok "./agent"

# galaxy options
./galaxy_res_client add_agent -e 10.100.40.100:1025 -p main_pool
./galaxy_res_client add_user -u test -t test
./galaxy_res_client grant_user -u test -p main_pool -o add -a create_container,remove_container,update_container,list_containers,submit_job,remove_job,update_job,list_jobs
./galaxy_res_client assign_quota -u test -c 4000 -d 4G  -s 1M e -m 4G -r 1000

sleep 20
./galaxy_client submit -f test.json
./galaxy_client submit -f over.json
