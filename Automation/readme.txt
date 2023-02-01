This folder contains the script used for checking if the current kernel configuration is OK for running isolated tasks in tickless mode.
-- Files:
    FrameworkOnlineAcceptancy:  This folder contains a framework used for creating isolated tasks, at each creation an the first isolated cpu
                                free is used for the task spawning .
                                In case no isolated cpu is available the task is not spawned.
                                This is done mantaining a bitmask of the isolated cpus in a shared memory area.
    -offload.sh:                Takes the mask of the isolated cores in input and performs an offload of irqs to non isolated cores.
    -KernelConfig.sh:           Checks if the current kernel config is ok for running tasks in isolation tickless mode, sets the bootloader booting the                                     kernel with the isolated cpus given by the user in input  and  performs an offload of irqs in case the grub is not updated.
    
    So in order to run tasks in isol mode:
    
    1-  run KernelConfig
    2-  In case the kernel config is ok then set the isolated cpus, this step will make an automatic reboot since the grub file is changed .
        ( in case the user doesn't change the configuration the offload is performed in this step)
    3-  After the reboot run offload.sh 
    4-  put the user task in job_body function in main.c in FrameworkOnlineAcceptancy and then compile it with  gcc -o start main.c periodic.c -lpthread
    5-  Run ./start "period in us"
    6-  Return to step 4 for another task, the framework will automatically find the first available isolated core
