#!/usr/bin/env sh
rm -rf /tmp/0
# NOTE ftp path
../output/bin/galaxy_client localhost:8102 add task.sh.tar.gz "sh task.sh" 2
