#!/usr/bin/env sh
# NOTE ftp path
../output/bin/galaxy_client add \
                           --master_addr=yq01-tera85.yq01.baidu.com:9876 \
                           --job_name=1234 \
                           --cpu_soft_limit=2 \
                           --cpu_num=4 --mem_gbytes=1 \
                           --replicate_num=4 \
                           --task_raw=task.sh.tar.gz \
                           --restrict_tag=tera\
                           --cmd_line="sh task.sh"
