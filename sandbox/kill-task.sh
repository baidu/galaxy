#!/usr/bin/env sh
../output/bin/galaxy_client kill --master_addr=localhost:8102 --task_id=$1
