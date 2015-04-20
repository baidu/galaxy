#!/usr/bin/env sh
# NOTE ftp path
../output/bin/galaxy_client localhost:8102 add task task.sh.tar.gz "sh task.sh" 1 0.8 5 spider
