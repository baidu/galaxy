#! /bin/sh

echo $TASK_ID
echo $TASK_NUM

./bfs_client cat /wordcount/src/part-$TASK_ID | ./mapreduce --role=mapper --task_id=$TASK_ID | ./easy_shuffle.sh -r $TASK_NUM -o ./tmp
for ((i=0;i < $TASK_NUM;i++)) do
	./bfs_client mkdir /wordcount/shuffle/$i
	./bfs_client put ./tmp/$i.tmp /wordcount/shuffle/$i/$TASK_ID
done;
