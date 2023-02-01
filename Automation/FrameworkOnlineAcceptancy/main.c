#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "periodic.h"

void* job_body(void* arg)
{
	//printf("Hello World\n");
}

static unsigned int must_stop=0;
void intHandler(int dummy){
	must_stop=1;
	//fclose(fp);
}

/**************** Taken from cyclictest *********************/
int latency_target_fd = -1;
int latency_target_value = 0;
void set_latency_target(void)
{
	struct stat s;
	int ret;

	if (stat("/dev/cpu_dma_latency", &s) == 0) {
		latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
		if (latency_target_fd == -1)
			return;
		ret = write(latency_target_fd, &latency_target_value, 4);
		if (ret == 0) {
			printf("# error setting cpu_dma_latency to %d!n", latency_target_value);
			close(latency_target_fd);
			return;
		}
		printf("# /dev/cpu_dma_latency set to %dus\n", latency_target_value);
	}
}

int main(int argc,char*argv[])
{
	
	struct timespec main_period={1,0}; 
	long rt_period=100000; // 100 us
#if MONITORING
	char log_loc[512];
	strcpy( log_loc,"rt_latencies.txt");
#endif
	if (argc<=1)
		printf("# Setting rt-task periodicity to 100 us\n");
	
	else{
		rt_period=atoi(argv[1])*1000;//converting it to NS
		printf("# Rt period in ns %ld \n",rt_period);
	}
#if MONITORING
	if (argc>=3){
		printf("#INSERTED FILE %s\n ",argv[2]);
		strcpy(log_loc,argv[2]);
		//printf("%s\n ",log_loc);
	}
#endif
	/*******************************MONITOR INIT***********************************/
#if MONITORING
	struct monitor mt1={
		.latency_act=0,
		.latency_min=0,
		.latency_cumulative_sum=0,
		.latency_max=0,
		.et_max=0,
		.et_min=0,
		.et_cumulative_sum=0,
		.et_act=0
	};
#endif
	/****************************************************************************/
	
	/**************************HANDLER SETUP*****************************/
	// isolate it on another cpu, no longer needed since we're running this core from isol_cpu config
	//isol_caller(0);
#if MONITORING
	FILE* fp=fopen(log_loc,"w");
	if (fp==NULL){
		printf("### Error opening the file in %s \n", log_loc);
		exit(1);
	}
	else 
		printf("### Opened %s \n", log_loc);
#endif
	// Setting the handler in order to close the file
	signal(SIGINT,intHandler);


	set_latency_target();
	if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
	{
		printf("Lock failed");
		exit(1);
	}
	
	/******************* SHM INIT ***********************************************/
	// Getting the sem
	int sem_id=generate_core_sem();
	if(sem_id<0) return -1;

	// Getting the struct 
	struct shm_cores* isolated_cores;
	int shmid=generate_core_shm(&isolated_cores,sem_id);

	/****************************************************************************/
	struct PeriodicTask job = {
		.phase = {0, 0},
		.period = {0, rt_period},
		.priority = 10,
#if MONITORING
		.mt=&mt1,
#endif
		.code = job_body,
		.data = NULL
	}; 
	
	int core = getCoreIsolated(isolated_cores,sem_id);
	printf("Core using: %d\n", core);
	if (core >= 0) {
		launch(&job, core);
	} else {
		printf("No isolated cpus available\n");
		exit(1);
	}
	/******************************************************************************/
#if MONITORING
	fprintf(fp,"Act   Max   Min   Avg\n");	
#endif
	/***********************************************************/
	while (1) {
		if(must_stop){
			close(latency_target_fd);
#if MONITORING
			if(fp!=NULL){
				fclose(fp);				
			}
#endif
			int end = ungetCoreIsolated(isolated_cores, core,shmid,sem_id);
			/**********************************/
			if(end==1) printf ("############### EXITING LAST %d\n",core);
			else	printf ("############### EXITING %d\n",core);
			return 0;
			
		}
#if MONITORING
		fprintf(fp,"%ld   %ld   %ld   %f\n", mt1.latency_act,mt1.latency_max,mt1.latency_min,(double)mt1.latency_cumulative_sum/(double)mt1.ex_count);
#endif
		nanosleep(&main_period,NULL);
	}

	return 0;
}
