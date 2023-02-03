#!/bin/bash
#path_kernel_config=/home/emmo/Scrivania/linux/linux-5.15.86/.config
path_kernel_config=/boot/config-$(uname -r)
count=0
if !(cat $path_kernel_config | grep -q CONFIG_PREEMPT=y) && !(cat $path_kernel_config | grep -q CONFIG_PREEMPT_RT=y)
then
	((count++))
	echo CONFIG_PREEMPT=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_HIGH_RES_TIMERS=y)
then
	((count++))
	echo CONFIG_HIGH_RES_TIMERS=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_NO_HZ_FULL=y)
then
	((count++))
	echo CONFIG_NO_HZ_FULL=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_RCU_NOCB_CPU=y)
then
	((count++))
	echo CONFIG_RCU_NOCB_CPU=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_HZ_1000=y)
then
	((count++))
	echo CONFIG_HZ_1000=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_HZ=1000)
then
	((count++))
	echo CONFIG_HZ!=1000
fi
if !(cat $path_kernel_config | grep -q CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE=y)
then
	((count++))
	echo CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_CPUSETS=y)
then
	((count++))
	echo CONFIG_CPUSETS=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_CPU_ISOLATION=y)
then
	((count++))
	echo CONFIG_CPU_ISOLATION=n
fi
if !(cat $path_kernel_config | grep -q CONFIG_CPU_IDLE_GOV_MENU=y)
then
	((count++))
	echo CONFIG_CPU_ISOLATION=n
fi
if ((count > 0))
then
	echo Recompile the kernel $(uname -r) with above flags enabled
	exit 1
else
	echo Kernel $(uname -r) correctly compiled
fi


flag=0
isolated=$(cat /sys/devices/system/cpu/isolated)
nohz_full=$(cat /sys/devices/system/cpu/nohz_full)

if ! [ -f /sys/devices/system/cpu/isolated ] || ! [ -s /sys/devices/system/cpu/isolated ]
then
	echo There are no isolated cpus
else
	((flag++))
	echo Isolated cpus: $isolated
fi
if ! [ -f /sys/devices/system/cpu/nohz_full ] || ! [ -s /sys/devices/system/cpu/nohz_full ]
then
        echo There are no tickless cpus
else
	((flag++))
	echo Tickless cpus: $nohz_full
	if ((flag == 2))
	then
		if [[ $isolated == $nohz_full ]]
		then
			((flag++))
		fi
	fi
fi



echo -n Do you want to change isolated cpus? [Y/n]. If not, then irqs are migrated to unisolated cpus:
read answer

if [[ $answer == 'Y' ]]
then
	mask=""
	n_proc=$(nproc --all)
	
	echo -n Insert number of isolated cpus:
	read argc
	while (($argc <= 0)) || (($argc > $n_proc - 1))
	do
		echo -n Insert a VALID number of isolated cpus:
		read argc
	done
	cnt=1
	while (($cnt <= argc))
	do
		echo -n Insert cpu no.$cnt:
		read cpu
		tmp=$(echo $mask | grep $cpu)
		while (($cpu < 0)) || (($cpu >= $n_proc)) || ! [ -z $tmp ]
		do
			echo -n Insert a VALID cpu no.$cnt:
			read cpu
			tmp=$(echo $mask | grep $cpu)
		done
		mask="$mask,$cpu"
		((cnt++))
	done
	mask="${mask:1}"
	cmdline='GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"'
	cmdline="${cmdline%?}"
	cmdline="$cmdline isolcpus=$mask nohz_full=$mask\""
	echo $cmdline
	sudo sed -i "10 c ${cmdline}" /etc/default/grub
	sudo update-grub
	#reboot
	sudo reboot
else
	if ((flag == 3))
	then
		sudo ./runtimeConfig.sh
	fi
fi
