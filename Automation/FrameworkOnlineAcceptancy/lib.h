#include <stdint.h>
#include <time.h>

#define	timespecadd(tsp, usp, vsp)				\
	do {								\
		(vsp)->tv_sec = (tsp)->tv_sec + (usp)->tv_sec;	\
		(vsp)->tv_nsec = (tsp)->tv_nsec + (usp)->tv_nsec;	\
		if ((vsp)->tv_nsec >= 1000000000L) {			\
			(vsp)->tv_sec++;				\
			(vsp)->tv_nsec -= 1000000000L;		\
		}							\
	} while (0)

#define	timespecsub(tsp, usp, vsp)				\
	do {								\
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;	\
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	\
		if ((vsp)->tv_nsec < 0) {				\
			(vsp)->tv_sec--;				\
			(vsp)->tv_nsec += 1000000000L;		\
		}							\
	} while (0)

static inline uint64_t
TIMESPEC_TO_NSEC(const struct timespec *ts)
{
	if (ts->tv_sec > (UINT64_MAX - ts->tv_nsec) / 1000000000ULL)
		return UINT64_MAX;
	return ts->tv_sec * 1000000000ULL + ts->tv_nsec;
}
