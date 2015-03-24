#!/usr/bin/env sh
rm -rf /tmp/0
../output/bin/galaxy_client localhost:8102 task.sh.tar.gz "sh task.sh" 1
