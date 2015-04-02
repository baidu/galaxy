#! /bin/sh

./bfs_client -cat /wordcount/src/part-$task_id | ./mapper | ./easy_shuffle -r $task_num -o ./
for ((i=0;i<$task_num;i++)) do
	./bfs_client -mkdir /wordcount/shuffle/$i
	./bfs_client -put ./$i.tmp /wordcount/shuffle/$i/$task_id
done;
