#!/usr/bin/env sh
# NOTE ftp path
../output/bin/galaxy_client localhost:8102 add task task.sh.tar.gz "sh task.sh" 2 1 10
