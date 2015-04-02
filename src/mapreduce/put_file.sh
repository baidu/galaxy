#! /bin/sh
./bfs_client -mkdir /galaxy/src
for((i=0;i<13;i++)) do
	./bfs_client -put file/part-$i /galaxy/src/part-$i
done;
