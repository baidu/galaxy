#! /bin/sh

mkdir -p ./tmp/
rm -rf ./tmp/*
for((i=0;i<$task_num;i++)) do
	./bfs_client get /wordcount/shuffle/$task_id/$i ./tmp/$i
done;

cat ./tmp/* | ./reducer > ./result

./bfs_client put ./result /wordcount/result/part-$task_id

