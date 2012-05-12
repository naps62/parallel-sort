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

void dump_arr(vector<unsigned int> arr);
string arr_str(vector<unsigned int> arr);
vector<int> get_test_array(unsigned int n = LEN);
void read_arr(vector<unsigned int> &arr, unsigned int start);

#endif // _H_COMMON_
