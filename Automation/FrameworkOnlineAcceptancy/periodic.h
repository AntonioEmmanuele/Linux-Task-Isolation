#include "lib.h"

#define SHMCORES_KEY 0xdead
#define SEMCORES_KEY 0xfe55

#define MAX_PROCS 64

#define MONITORING 0
//#include <math.h>
#if MONITORING
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
#endif

struct shm_cores{

	unsigned int num_cores;
	int num_instances; // numero di main eseguiti
	// Contains '1' if a core is isolated, else 0.
	char isolated_mask[MAX_PROCS];
	// mutex
};

//unsigned int calc_queue_size(unsigned int , unsigned int);
struct PeriodicTask {
	struct timespec phase;
	struct timespec period;
	struct timespec releaseTime;
	struct timespec startingTime;
#if MONITORING
	struct monitor* mt;
#endif
	int priority;
	void* (*code)(void*);
	void* data;
};
void isol_caller(int cpu);
void launch(struct PeriodicTask* _periodicTask_, int core);
int getCoreIsolated(struct shm_cores* isolated_cores,int id_sem);
int ungetCoreIsolated(struct shm_cores* isolated_cores, int core,int shmid,int id_sem);
void get_isolated_mask(char* mask);
int generate_core_shm(struct shm_cores** ,int);
int generate_core_sem();