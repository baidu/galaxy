#! /bin/sh
./bfs_client mkdir /wordcount/src
for((i=0;i<7;i++)) do
	./bfs_client put file/part-$i /wordcount/src/part-$i
done;
