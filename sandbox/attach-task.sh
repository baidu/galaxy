#!/usr/bin/env sh
# NOTE ftp path
../output/bin/galaxy_client add \
                           --flagfile=client.flag \ 
                           --job_name=1234 \
                           --cpu_soft_limit=1 \
                           --cpu_num=1 --mem_gbytes=1 \
                           --replicate_num=4 \
                           --task_raw=task.sh.tar.gz \
                           --restrict_tag=dev\
                           --one_task_per_host\
                           --cmd_line="sh task.sh"
