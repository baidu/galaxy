#!/usr/bin/env sh
echo $TASK_ID
echo $TASK_NUM
echo "start echo"
for ((i=0;i<10;i++)) do
	echo $i;
	sleep 1;
done;
