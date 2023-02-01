#!/bin/bash
# Install  mpstat and lmsensors
core0_path=$1
core1_path=$2
core0temp_path=$3
core1temp_path=$4
SECONDS=0
while true
do 
	#if (($i==1))
	#then
	#	mpstat -P 0|grep "%usr" >> log_cpu
	#fi
	#mpstat 4 1 -P 0| tail -1|awk '{print "CPU0 Usage: " 100-$12}'
	#mpstat 4 1 -P 1| tail -1|awk '{print "CPU1 Usage: " 100-$12}'
	
	mpstat 4 1 -P 0| tail -1|awk '{print  100-$12}'>> $core0_path
	mpstat 4 1 -P 1| tail -1|awk '{print  100-$12}'>> $core1_path
	sensors| grep "Core 0" |awk '{print $3}'>>$core0temp_path
	sensors| grep "Core 1" |awk '{print $3}'>>$core1temp_path

	#echo $i
	#sleep 1
done
echo "#### TERMINATING CPU_STAT"
