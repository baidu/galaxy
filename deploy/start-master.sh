#!/usr/bin/env sh
port=$1
nohup ./bin/master -port=$port >master.log 2>&1 &
