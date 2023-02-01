This folder contains the files used for performing a latency and execution test of a single task in a core.
This is useful for checking the effectiveness of a kernel and hw config made for task isolation.
The source code of the tester is inspired on cyclic test and creates and runs a periodic task( with sched fifo) , on a hopefully isolated cpu, 
every period( given by the the user).
The task contains a hook for inserting a job body  in order to allow the user to test the latency of its own task.
In addition to this there is a test configuration :
