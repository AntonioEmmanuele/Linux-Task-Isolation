#!/bin/bash
isolated=$(cat /proc/cmdline | sed -e 's/^.*isolcpus=//' -e 's/ .*$//') # thanks to SO
nohz_full=$(cat /proc/cmdline | sed -e 's/^.*nohz_full=//' -e 's/ .*$//')

cpu=0
cmask=''
while ((cpu < $(nproc --all)))
do
	if [[ -z $(echo $isolated | grep $cpu) ]]
	then
		cmask="$cmask,$cpu"
	fi
	((cpu++))
done
cmask="${cmask:1}"

for i in $(ls /proc/irq)
do
    if [[ -d "/proc/irq/$i" ]]
    then
        echo $cmask > /proc/irq/$i/smp_affinity_list
    fi
done


for i in $(ls /proc/irq)
do
    if [[ -d "/proc/irq/$i" ]]
    then
    	if [[ $(tail /proc/irq/$i/smp_affinity_list) != $cmask ]]
    	then
    		echo $(tail /proc/irq/$i/smp_affinity_list) 
    		echo " Error in irq /proc/irq/$i/smp_affinity_list)"
        fi
    fi
done

