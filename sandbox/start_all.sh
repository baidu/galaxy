echo "start nexus"
cd ../ins/sandbox && nohup ./start_all.sh &> ins_start.log &
sleep 2
echo "start master"
nohup  ../master --flagfile=galaxy.flag >master.log 2>&1 &
echo "start aget"

sleep 1
echo "start scheduler"
nohup ../scheduler --flagfile=galaxy.flag >scheduler.log 2>&1 &
