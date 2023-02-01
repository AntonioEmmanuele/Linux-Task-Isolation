#include "periodic.h"

void* start_routine(void* arg)
{
	struct PeriodicTask* periodicTask = (struct PeriodicTask*) arg;
	unsigned int count=1;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &(periodicTask->phase), NULL);
	clock_gettime(CLOCK_MONOTONIC, &(periodicTask->releaseTime));

	struct timespec wcet_s;
	struct timespec wcet_f;
	while (1) {
		
		// Get the wcet starting time
		clock_gettime(CLOCK_MONOTONIC, &wcet_s);
		// execute the function
		periodicTask->code(periodicTask->data);
		// Get the wcet finishing time
		clock_gettime(CLOCK_MONOTONIC, &wcet_f);
		// Subtraction
		timespecsub(&wcet_f, &wcet_s, &wcet_f);
		periodicTask->mt->et_act=TIMESPEC_TO_NSEC(&wcet_f);
		// Update the fields
		if(periodicTask->mt->et_act > periodicTask->mt->et_max)
			 periodicTask->mt->et_max=periodicTask->mt->et_act;
		if(periodicTask->mt->et_act < periodicTask->mt->et_min||count==1 )
			 periodicTask->mt->et_min=periodicTask->mt->et_act;
		periodicTask->mt->et_cumulative_sum+=periodicTask->mt->et_act;
		
		// calc the next release time 
        	timespecadd(&(periodicTask->releaseTime), &(periodicTask->period), &(periodicTask->releaseTime));
		// wait until the abs value of the next period
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(periodicTask->releaseTime), NULL);
	    	// Get the real release time
		clock_gettime(CLOCK_MONOTONIC, &(periodicTask->startingTime));
		// Calculate the sub
		timespecsub(&(periodicTask->startingTime), &(periodicTask->releaseTime), &(periodicTask->startingTime));
		periodicTask->mt->latency_act=TIMESPEC_TO_NSEC(&(periodicTask->startingTime));
		//update latency_params
		if(periodicTask->mt->latency_act > periodicTask->mt->latency_max)
			 periodicTask->mt->latency_max=periodicTask->mt->latency_act;
		if(periodicTask->mt->latency_act < periodicTask->mt->latency_min||count==1)
			 periodicTask->mt->latency_min=periodicTask->mt->latency_act;
		periodicTask->mt->latency_cumulative_sum+=periodicTask->mt->latency_act;
		count++;
		periodicTask->mt->ex_count=count;
		
	}
}

void launch(struct PeriodicTask* _periodicTask_, int core)
{
	struct sched_param myparam;
    myparam.sched_priority = _periodicTask_->priority;

	pthread_attr_t myattr;
	pthread_attr_init(&myattr);
	pthread_attr_setschedpolicy(&myattr, SCHED_FIFO);
	pthread_attr_setinheritsched(&myattr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&myattr, &myparam);

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);
	pthread_attr_setaffinity_np(&myattr, sizeof(cpu_set_t), &cpuset);

	pthread_t mythread;
	pthread_create(&mythread, &myattr, start_routine, (void*) _periodicTask_);
	pthread_attr_destroy(&myattr);
}
void isol_caller(int cpu)
{
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}
