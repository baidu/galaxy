#! /bin/sh

for((i=0;i<$task_num;i++)) do
	mkdir -p ./tmp/
	./bfs_client -get /shuffle/$task_id/part-$i ./tmp/$i
done;

cat ./tmp/* | ./reducer > ./result

./bfs_client -put ./result /result/part-$task_id

