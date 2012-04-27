/**
 * \file seeds.h
 *
 * test input
 */

#ifndef _H_SEEDS_
#define _H_SEEDS_

#include <vector>
#include <stdlib.h>
#include <time.h>
#include "common.h"
using namespace std;

#define LEN 10

vector<int> def_arr;

vector<int> get_test_array(int n = LEN) {
	srand(time(NULL));
	while(n-- > 0) {
		def_arr.push_back(rand() % 1000000);
	}

	return def_arr;
}

#endif // _H_SEEDS_
