This folder contains the files used for performing a latency and execution test of a single task in a core.
This is useful for checking the effectiveness of a kernel and hw config made for task isolation.
The source code of the tester is inspired on cyclic test and creates and runs a periodic task( with sched fifo) , on a hopefully isolated cpu, 
every period( given by the the user).
The task contains a hook for inserting a job body  in order to allow the user to test the latency of its own task.
In addition to this there's the infrastructure ( or a possible example, used in our own test )  of a full test suit.

Tester files ( "gcc -o start main.c periodic.c") -lpthread to comple:
  --- main.c  -> main of the tester
  --- periodic.c/periodic.h -> library used for the creation and monitoring of periodic tasks
  --- lib.h --> Library containing timespec utils
  
 Infrastructure files:
  --- cpu_stats.sh -> monitor and saves the cpu and temperature usage of cores 0 and 1 ( can be extended to multiple cores modifying the script).
  --- logger.sh ->    creates the configuration before the logs ( creates folders , puts -1 in sched_rt_runtime , and performs an irq offload).
                      After creating the configuration the script runs different tests( rebooting the computer from one to another).
                      Each single test is performed running an empty task with two different. priorities (100 us and 1000 us) on core 1 and starting
                      a stressor (with stress-ng) command on the non isolated core0 ( this can be changed).
                      The stressors are:
                      1- cache
                      2- vm
                      3- context
                      4- load
                      5- no stressors
                      Each test runs for 30 mins.
  --- offload.sh ->   takes the isolated mask and performs an offload of irqs to non-isolated cores.
  --- 
