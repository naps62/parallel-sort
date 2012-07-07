/**
 * \file sort.seq.cpp
 */

#include "common.h"
#include <stdlib.h>
#include <vector>
#include <limits>
#include <iostream>
#include <string>
#include <sstream>
#include <omp.h>
using namespace std;


// given a key and a mask, calcs the bucket to where the key belongs

void test_data(vector<unsigned int> &arr) {
	srand(time(NULL));
	for(unsigned int i = 0; i < arr.size(); ++i)
		arr[i] = rand();

	arr[0] = 7;
	arr[1] = 5;
	arr[2] = 3;
	arr[3] = 2;
}

// count number of bits set to 1 in a number
unsigned int popcount(unsigned int x) {
	unsigned int c;
	for(c = 0; x; ++c) x &= x-1;
	return c;
}

// maps a given bucket to a cpu, given max number of buckets per cpu
#define GET_BUCKET_NUM(elem, mask, g, i) (((elem) & (mask)) >> ((g) * (i)))
#define BUCKET_TO_CPU(bucket)		(((bucket) & (~((bpp) - 1))) >> bpp_bc)
#define BUCKET_IN_CPU(bucket)       ( (bucket) & (  (bpp) - 1)            )

/**
 * it is assumed that B = P, only one bucket per processor
 *
 * \param arr Where to stop the ordered array (one local array per processor)
 * \param id  Id of this processs
 * \param p   Number of processor (equivalent to MPI_size)
 * \param g   Number of bits to use for each mask (must be power of 2)
 */
void radix_omp(radix_arr_t &arr, const unsigned int num_threads, const unsigned int g);




int check_array_order(radix_arr_t &arr) {
	// check local array order
	for(unsigned int i = 1; i < arr.size(); ++i)
		if (arr[i - 1] > arr[i])
			return i;

	return ORDER_CORRECT;
}

int main(int argc, char **argv) {
	
	int g = 4;
	int len;
	int size;

	Timer timer;

	if (argc > 1)	len = atoi(argv[1]);
	else           len = LEN;

	if (argc > 2)	g = atoi(argv[2]);
	else           g = 4;

	if (argc > 3)	size = atoi(argv[3]);
	else           size = 4;

	cerr
		<< "array len   = " << len << endl
		<< "g           = "	<< g << endl
		<< "num_threads = " << size << endl;

	vector<unsigned int> seq_arr(len);
	radix_arr_t arr(len);
	// generate test data
	read_arr(len, 0);
	for(int i = 0; i < len; ++i)
		arr[i] = seq_arr[i];

	//cerr << "initial data: " << arr_str(arr) << endl;

	// the real stuff
	cerr << "starting radix sort...";
	timer.start();
	radix_omp(arr, size, g);
	timer.stop();
	cerr << "finished" << endl << endl;

	//cerr << arr_str(arr) << endl;
	// check array order
	int order = check_array_order(arr);
	switch (order) {
		case ORDER_CORRECT: 	cerr << "CORRECT! Result is ordered" << endl; break;
		default: 				cerr << "WRONG! Order fails at index " << order << endl; break;
	}

	// print time for each process
	cout << timer.get() * 1.0e-3 << endl;
}
