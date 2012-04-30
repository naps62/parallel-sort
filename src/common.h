/**
 * \file common.h
 *
 * Common code shared amongst all implementation
 */

#ifndef _H_COMMON_
#define _H_COMMON_

#include <time.h>
#include <vector>
#include <string>
using std::vector;
using std::string;

#define DEF_CLOCK CLOCK_MONOTONIC

//
// CLASSES AND STRUCTS
//

class Timer {
	struct timespec t_start, t_end;
	public:
		void start();
		void stop();
		long get();
};

//
// HELPER FUNCTIONS
//
#define LEN 10

void dump_arr(vector<int> arr);
string arr_str(vector<int> arr);
vector<int> get_test_array(int n = LEN);

#endif // _H_COMMON_
