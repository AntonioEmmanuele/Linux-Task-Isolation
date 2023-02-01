#!/bin/bash
if [[ -d logs ]]; then
	echo "Folders already present"
else
	echo "Removing" 
	# First, remove pre existing folder
	sudo rm -r -f logs/
	#generate scripts
	mkdir logs && cd logs && mkdir load cache context vm  no_stress
	cd load && mkdir p100 p1000 && cd ..
	cd cache && mkdir p100 p1000 && cd ..
	cd context && mkdir p100 p1000 && cd ..
    cd vm && mkdir p100 p1000 && cd ..
    cd no_stress && mkdir p100 p1000 && cd ..
	cd ..
fi
# Context 
# Cache
# Load
# Mem
tst=1
tst=$(tail run_tests)
echo $tst
if [[ $tst == 0 ]]; then
	echo " Must run the test"
else
	echo "must stop"
	exit 0
fi 
# check if the file exist, in case create it with Cache100
if ! [[ -f next_test_file ]]; then 
	echo "Next test file not existing, do you want to start the latency test?"
	#read answer 
	#if  [[ "$answer" == yes ]]; then 
		echo " Creating the file "	
		echo Cache100 >next_test_file
		tail next_test_file
	#else 
	#	echo "Closing the application "
	#	exit 0
	#fi
fi

# Load the next test
next_test="Cache100"
next_test=$(tail next_test_file)
# Disabling rt throttling
echo -1 > /proc/sys/kernel/sched_rt_runtime_us
# Make the offload of ir
sudo ./offload.sh

echo "Value of sched_runtime "
tail /proc/sys/kernel/sched_rt_runtime_us

# execute the test and go to sleep
case $next_test in 
#********************CACHE
	Cache100)
	echo " Starting Cache 100"
	# cache 100
	sudo timeout -s SIGINT 30m ./start 100 ./logs/cache/p100/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/cache/p100/c0_usg  logs/cache/p100/c1_usg logs/cache/p100/c0_tmp  logs/cache/p100/c1_tmp & 
	sudo stress-ng --taskset 0 --tz --timeout 30m  --cache 2 --cache-no-affinity&& fg
	wait
	echo " ENDED Cache 100"
	# Load the next test
	rm next_test_file
	echo Cache1000 > next_test_file
	sudo reboot 
	;;
	
	Cache1000)
	echo " Starting Cache 1000"
	# cache 1000
	sudo timeout -s SIGINT 30m ./start 1000 ./logs/cache/p1000/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/cache/p1000/c0_usg  logs/cache/p1000/c1_usg logs/cache/p1000/c0_tmp  logs/cache/p1000/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m --cache 2 --cache-no-affinity && fg
	wait 
	echo " ENDED Cache 1000"
	
	# Load the next test
	rm next_test_file
	echo vm100 > next_test_file
	sudo reboot 
	;;
	
# *****************VM
	vm100)
	echo " Starting vm 100"
	# vm 100
	sudo timeout -s SIGINT 30m ./start 100 ./logs/vm/p100/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/vm/p100/c0_usg  logs/vm/p100/c1_usg logs/vm/p100/c0_tmp  logs/vm/p100/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m  --vm 10 --vm-bytes 2048M&& fg
	wait
	echo " ENDED vm 100"
	# Load the next test
	rm next_test_file
	echo vm1000 > next_test_file
	sudo reboot 
	;;
	
	vm1000)
	echo " Starting vm 1000"
	# vm 1000
	sudo timeout -s SIGINT 30m ./start 1000 ./logs/vm/p1000/rt_lat.txt &  
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/vm/p1000/c0_usg  logs/vm/p1000/c1_usg logs/vm/p1000/c0_tmp  logs/vm/p1000/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m --vm 10 --vm-bytes 2048M && fg
	wait
	echo " ENDED vm 1000"

	# Load the next test
	rm next_test_file
	echo Context100 > next_test_file
	sudo reboot 
	;;


#********************CONTEXT	
	Context100)

	echo " Starting Context 100"
	# Context 100
	sudo timeout -s SIGINT 30m ./start 100 ./logs/context/p100/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/context/p100/c0_usg  logs/context/p100/c1_usg logs/context/p100/c0_tmp  logs/context/p100/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m  --context 2
	wait
	echo " ENDED CONTEXT 100"
	# Load the next test
	rm next_test_file
	echo Context1000 > next_test_file
	sudo reboot 
	;;
	
	Context1000)
	echo " Starting CONTEXT 1000"
	# Context 1000
	sudo timeout -s SIGINT 30m ./start 1000 ./logs/context/p1000/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/context/p1000/c0_usg  logs/context/p1000/c1_usg logs/context/p1000/c0_tmp  logs/context/p1000/c1_tmp &
	sudo stress-ng --taskset 0 --tz --timeout 30m  --context 2 &&fg
	wait 
	echo " ENDED CONTEXT 100S0"
	# Load the next test
	rm next_test_file
	echo load100 > next_test_file
	sudo reboot 
	;;
	
	load100)
#***************LOAD
	echo " Starting load 100"
	# load 100
	sudo timeout  -s SIGINT 30m ./start 100 ./logs/load/p100/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/load/p100/c0_usg  logs/load/p100/c1_usg logs/load/p100/c0_tmp  logs/load/p100/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m  --cpu 1 --cpu-load 100 &&fg
	wait
	echo " ENDED load 100"
	# Load the next test
	rm next_test_file
	echo load1000 > next_test_file
	sudo reboot 
	;;
	
	load1000)
	echo " Starting load 1000"
	# load 1000
	sudo timeout -s SIGINT 30m ./start 1000 logs/load/p1000/rt_lat.txt & 
	sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/load/p1000/c0_usg  logs/load/p1000/c1_usg logs/load/p1000/c0_tmp  logs/load/p1000/c1_tmp &
	sudo stress-ng --taskset 0 --tz  --timeout 30m  --cpu 1 --cpu-load 100 &&fg
	wait
	echo " ENDED load 1000"
	rm next_test_file
	echo no_stress100 > next_test_file
	sudo reboot 
	;;
	
#********************No Stress
    no_stress100)
    echo "Starting no stress 100"
    sudo timeout -s SIGINT 30m ./start 100 ./logs/no_stress/p100/rt_lat.txt &
    sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/no_stress/p100/c0_usg  logs/no_stress/p100/c1_usg logs/no_stress/p100/c0_tmp  logs/no_stress/p100/c1_tmp && fg
    echo " ENDED no stress 100"
    # Load the next test
    rm next_test_file
    echo no_stress1000 > next_test_file
    sudo reboot
    ;;

    no_stress1000)
    echo "Starting no stress 1000"
    sudo timeout -s SIGINT 30m ./start 1000 ./logs/no_stress/p1000/rt_lat.txt &
    sudo timeout 30m taskset -c 0 ./cpu_stat_script.sh logs/no_stress/p1000/c0_usg  logs/no_stress/p1000/c1_usg logs/no_stress/p1000/c0_tmp  logs/no_stress/p1000/c1_tmp && fg
    echo " ENDED no stress 1000"
    # Load the next test
    rm next_test_file
    rm run_tests
    echo 1 > run_tests
    echo Cache100 > next_test_file
    sudo reboot
    ;;
esac
