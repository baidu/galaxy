#! /bin/sh

./bfs_client -cat /src/part-$task_id | ./mapper | ./shuffle ./part- $task_num
for ((i=0;i<$task_num;i++)) do
	./bfs_client -mkdir /shuffle/$i
	./bfs_client -put ./part-$i /shuffle/$i/
done;
