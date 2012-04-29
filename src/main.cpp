/**
 * \file sort.seq.cpp
 */

#include <vector>
#include <iostream>
#include <stdlib.h>
#include "sort.h"
#include "common.h"
using namespace std;

extern vector<int> def_arr;

int main(int argc, char **argv) {

	int len;
	if (argc > 1)
		len = atoi(argv[1]);
	else {
		len = LEN;
	}

	Timer timer;

	sort(argc, argv, len, timer);

	cout << "solution took: " << timer.get() * 1.0e-3 << " usec" << endl;
	cout << endl << "exiting" << endl;
}
