/**
 * \file common.h
 *
 * Common code shared amongst all implementation
 */

#ifndef _H_COMMON_
#define _H_COMMON_

#include <time.h>
#include <iostream>
using namespace std;

#include "seeds.h"

#define DEF_CLOCK CLOCK_MONOTONIC

//
// CLASSES AND STRUCTS
//

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

//
// HELPER FUNCTIONS
//

void dump_arr(vector<int> arr) {
	cout << "[" << arr[0];
	for(unsigned int i = 1; i < arr.size(); ++i)
		cout << ", " << arr[i];
	cout << "]" << endl;
}

bool check_array_order(vector<int> arr) {
	for(unsigned int i = 1; i < arr.size(); ++i)
		if (arr[i-1] > arr[i])
			return false;
	return true;
}

#endif // _H_COMMON_
