#!/usr/bin/env sh
echo "start echo"
for ((i=0;i<1000;i++)) do
	echo $i;
	sleep 1;
done;
