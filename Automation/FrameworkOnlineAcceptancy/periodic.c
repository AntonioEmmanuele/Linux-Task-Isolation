#define _GNU_SOURCE

#include <pthread.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "periodic.h"
#include "parse.h"

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "sys/sysinfo.h"

static void sem_wait(int id_sem, int numsem)     {
	struct sembuf sem_buf;
	sem_buf.sem_num=numsem;
	sem_buf.sem_flg=0;
	sem_buf.sem_op=-1;
	semop(id_sem,&sem_buf,1);   //semaforo rosso
}


static void sem_signal (int id_sem,int numsem)     {
	struct sembuf sem_buf;
	sem_buf.sem_num=numsem;
	sem_buf.sem_flg=0;
	sem_buf.sem_op=1;
	semop(id_sem,&sem_buf,1);   //semaforo verde
}

void* start_routine(void* arg)
{
	struct PeriodicTask* periodicTask = (struct PeriodicTask*) arg;
	unsigned int count=1;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &(periodicTask->phase), NULL);
	clock_gettime(CLOCK_MONOTONIC, &(periodicTask->releaseTime));

#if MONITORING
	struct timespec wcet_s;
	struct timespec wcet_f;
#endif

	while (1) {

#if MONITORING
		// Get the wcet starting time
		clock_gettime(CLOCK_MONOTONIC, &wcet_s);
#endif
		// execute the function
		periodicTask->code(periodicTask->data);

#if MONITORITING
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
#endif
		// calc the next release time 
		timespecadd(&(periodicTask->releaseTime), &(periodicTask->period), &(periodicTask->releaseTime));
		// wait until the abs value of the next period
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(periodicTask->releaseTime), NULL);

#if MONITORING
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
#endif
		
	}
}
/* User function that creates and launches the periodic task described by the struct _periodicTask_ with the affinity setted to core*/
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
/* Isolate the calling thread to a specific cpu*/
void isol_caller(int cpu)
{
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

/* Get the list of the isolated cpus in the system*/
void get_isolated_mask(char* mask){
	// File containing the isolated cpu list
	FILE* isolated_file = NULL;
	// Buff for reading files
	char buf[128];

	//Opening files
	isolated_file = fopen("/sys/devices/system/cpu/isolated", "r");
	if (isolated_file == NULL){
		printf(" Error opening proc directory \n");
		exit(-1);
	}

	for (int i=0;i<MAX_PROCS;i++) mask[i]='0';
	//printf("The mask is %s %d  \n",mask,sizeof(mask));

	fgets(buf, 128, isolated_file);
	if (buf[0] == '\n') {
		printf("There are no isolated cpus\n");
		exit(-1);
	}
	// No longer needed
	fclose(isolated_file);
	// removing the newline 
	buf[strlen(buf)-1]='\0';
	// Finding the mask 
	cpulist_parse(buf,mask);

}

/* Returns the first isolated available core*/
int getCoreIsolated(struct shm_cores* isolated_cores,int id_sem)
{
	//Lock
	sem_wait(id_sem,0);
	
	int core_to_ret=-1;
	for(unsigned int idx=0;idx<isolated_cores->num_cores;idx++){
		if(isolated_cores->isolated_mask[idx]=='1'){
			core_to_ret=idx;
			isolated_cores->isolated_mask[idx]='0';
			isolated_cores->num_instances++;
			break;
		}
	}
	
	// Unlock
	unlock:sem_signal(id_sem,0);
	
	return core_to_ret;
}

/* Frees the core from the usage of the ended task*/
int ungetCoreIsolated(struct shm_cores* isolated_cores, int core,int shmid,int id_sem)
{
	//Lock
	sem_wait(id_sem,0);

	isolated_cores->num_instances--;
	isolated_cores->isolated_mask[core]='1';
	if(isolated_cores-> num_instances==0){
			shmdt(isolated_cores);
			shmctl(shmid, IPC_RMID, 0);
	}
	// Unlock
	unlock:sem_signal(id_sem,0);
	semctl(id_sem,0,IPC_RMID);
	return 1;
}
/* Generate the semaphore used to access the isolated_cores struct*/
int generate_core_sem(){
	int sem_id=semget(SEMCORES_KEY,1,0644|IPC_CREAT);
	if(sem_id<0){
		printf("error getting the semaphore \n");
		return -1;
	}
	//Initialize the 0 sem of the sem-array to value 1
	if(semctl(sem_id,0,SETVAL,1)<0){
		printf("Error generating the semaphore \n ");
		semctl(sem_id,0,IPC_RMID);
		return -1;
	}
	return sem_id;
}

/* Generate the isolated_cores struct, this function must be called AFTER the creation of the semaphore */
int generate_core_shm(struct shm_cores**isolated_cores,int id_sem){
	//Lock
	sem_wait(id_sem,0);
	
	int shmid=shmget(SHMCORES_KEY,sizeof(struct shm_cores),0644);
	int newly_created=0;
	if (shmid<0){
		printf("Creating the shared memory \n ");
		shmid=shmget(SHMCORES_KEY,sizeof(struct shm_cores),0644|IPC_CREAT);
		if (shmid<0){
			printf("Error creating the shm \n");
			goto unlock;
			//return -1;
		}
		newly_created=1;
	}
	
	*isolated_cores=shmat(shmid,NULL,0);
	if (*isolated_cores<0){
		printf("Error attaching the shm \n ");
		// Removing the shm
		shmctl(shmid, IPC_RMID, 0);
		shmid=-1;
		goto unlock;
		//return -1;
	}

	if (newly_created){
		(*isolated_cores)->num_cores=get_nprocs();  // find number of processors
		(*isolated_cores)->num_instances=0; 
		get_isolated_mask((*isolated_cores)->isolated_mask);
	}
	// Unlock
	unlock:sem_signal(id_sem,0);
	return shmid;
}