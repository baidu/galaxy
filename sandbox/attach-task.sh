#!/usr/bin/env sh
rm -rf /tmp/0
../output/bin/galaxy_client localhost:8102 add ftp://cq01-ps-dev197.cq01.baidu.com/tmp/task.sh.tar.gz "sh task.sh" 2
