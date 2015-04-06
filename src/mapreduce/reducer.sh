#! /bin/sh

echo $TASK_ID
echo $TASK_NUM
mkdir -p ./tmp/
rm -rf ./tmp/*
for((i=0;i < $TASK_NUM; i++)) do
	./bfs_client get /wordcount/shuffle/$TASK_ID/$i ./tmp/$i
done;

cat ./tmp/* | sort | ./reducer > ./result

./bfs_client put ./result /wordcount/result/part-$TASK_ID

