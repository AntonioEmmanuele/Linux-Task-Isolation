#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "periodic.h"
#include "signal.h"
#include "string.h"
void* _first_(void* arg)
{
	//printf("Hello World\n");
}

void* _second_(void* arg)
{
        //printf("Ciao Mondo\n");
}

unsigned int must_stop=0;
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
	struct timespec main_period={1,0}; // 10us
	long rt_period=100000; // 100 us
	char log_loc[512];
	strcpy( log_loc,"rt_latencies.txt");
	if (argc<=1)
		printf("# Setting rt-task periodicity to 100 us\n");
	
	else{
		rt_period=atoi(argv[1])*1000;//converting it to NS
		printf("# Rt period in ns %ld \n",rt_period);
	}
	if (argc>=3){
		printf("#INSERTED FILE %s\n ",argv[2]);
		strcpy(log_loc,argv[2]);
		//printf("%s\n ",log_loc);
	}
	/*******************************MONITOR INIT***********************************/
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
	/****************************************************************************/
	
	/*struct PeriodicTask second = {
                .phase = {6, 0},
                .period = {2, 0},
		.priority = 20,
                .code = _second_,
                .data = NULL
        }; launch(&second, 3);*/

	/**************************HANDLER SETUP*****************************/
	// isolate it on another cpu
	isol_caller(0);
	FILE* fp=fopen(log_loc,"w");
	if (fp==NULL){
		printf("### Error opening the file in %s \n", log_loc);
		exit(1);
	}
	else 
		printf("### Opened %s \n", log_loc);
	// Setting the handler in order to close the file
	signal(SIGINT,intHandler);


	set_latency_target();
	if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
	{
		printf("Lock failed");
		exit(1);
	}

	/****************************************************************************/
	struct PeriodicTask first = {
		.phase = {0, 0},
		.period = {0, rt_period},
		.priority = 10,
		.mt=&mt1,
		.code = _first_,
		.data = NULL
	}; launch(&first, 1);
	/******************************************************************************/
	
	fprintf(fp,"Act   Max   Min   Avg\n");	
	/***********************************************************/
	while (1) {
		if(must_stop){
			close(latency_target_fd);
			if(fp!=NULL){
				//printf(" Closing file\n");
				fclose(fp);
				//printf(" Ci sono arrivato \n");
			}
			printf ("############### EXITING \n");
			return 0;
			
		}

			//fprintf(fp,"%ld  %ld  %ld \n", mt1.evt_num[mt1.tail],mt1.wcets[mt1.tail],mt1.vals[mt1.tail]);
		fprintf(fp,"%ld   %ld   %ld   %f\n", mt1.latency_act,mt1.latency_max,mt1.latency_min,(double)mt1.latency_cumulative_sum/(double)mt1.ex_count);

		
		nanosleep(&main_period,NULL);
	}

	return 0;
}
