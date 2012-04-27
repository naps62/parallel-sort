/**
 * \file common.h
 *
 * Common code shared amongst all implementation
 */

#ifndef _H_COMMON_
#define _H_COMMON_

#include <time.h>

#define DEF_CLOCK CLOCK_MONOTONIC

/**
 * Class used to measure time between two events
 */
class Timer {
	
	struct timespec t_start, t_end;

	public:
		inline void start() {
			clock_gettime(DEF_CLOCK, &t_start);
		}

		inline void stop() {
			clock_gettime(DEF_CLOCK, &t_end);
		}

		inline long get() {
			return (t_end.tv_sec - t_start.tv_sec) * 1e9 + (t_end.tv_nsec - t_start.tv_nsec);
		}
};

#endif // _H_COMMON_
