#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "lib.h"

//#include <math.h>
struct monitor{
	uint64_t latency_max;
	uint64_t latency_cumulative_sum;
	uint64_t latency_min;
	uint64_t latency_act;
	uint64_t et_min;
	uint64_t et_max;
	uint64_t et_cumulative_sum;
	uint64_t et_act;
	uint64_t ex_count;
};

//unsigned int user_read_time ( in ns ), unsigned int real_time_task_period ( in ns ),unsigned int offset
//# define CALC_QUEUE_SIZE(user_read_time,rt_period,offset) (2*((unsigned int)ceil(user_read_time/rt_period)+offset))

/*
//struct monitor ,unsigned int user_read_time, unsigned int real_time_task_period,unsigned int offset
#define INIT_MONITOR(monitor,user_read_time,rt_period,offset){	\
	monitor.head=0;	\
	monitor.tail=0;	\
	monitor.queue_size=(2*(ceil(user_read_time/rt_period)+offset));\ // the reader can read half of the queue without overlapping
	uint64_t v[monitor.queue_size]={0},evts[monitor.queue_size]={0};\
	monitor.vals=v;	\
	monitor.evt_num=evt_num;\

}    */

//unsigned int calc_queue_size(unsigned int , unsigned int);
struct PeriodicTask {
	struct timespec phase;
	struct timespec period;
	struct timespec releaseTime;
	struct timespec startingTime;
	struct monitor* mt;

	int priority;
	void* (*code)(void*);
	void* data;
};
void isol_caller(int cpu);
void launch(struct PeriodicTask* _periodicTask_, int core);
