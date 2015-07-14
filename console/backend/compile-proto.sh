#!/usr/bin/env sh
echo "build task proto"
protoc -I ../../src/proto/  --python_out=src/galaxy/ ../../src/proto/task.proto
echo "build master proto"
protoc -I ../../src/proto/  --python_out=src/galaxy/ ../../src/proto/master.proto
echo "build agent proto"
protoc -I ../../src/proto/  --python_out=src/galaxy/ ../../src/proto/agent.proto
protoc -I ../../src/proto/  --python_out=src/galaxy/ ../../src/proto/monitor.proto
