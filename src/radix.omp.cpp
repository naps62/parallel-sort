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
#define GET_BUCKET_NUM(elem, mask, g, i) (((elem) & (mask)) >> ((g) * (i)))

enum OrderCodes {
	ORDER_CORRECT = 0,
	ORDER_ERROR_BORDERS = -1,
	ORDER_ONLY_MASTER = -2,
};

enum Tags {
	TAG_KEY_SEND = 1,
	TAG_CHECK_ORDER = 2,
	TAG_COUT_SIZE = 3,
	TAG_COUT_DATA = 4,
	TAG_BUCKET_COUNT = 5,
};

void test_data(vector<unsigned int> arr) {
	srand(time(NULL));
	for(unsigned int i = 0; i < arr.size(); ++i)
		arr[i] = rand() % 1000;
}

// count number of bits set to 1 in a number
unsigned int popcount(unsigned int x) {
	unsigned int c;
	for(c = 0; x; ++c) x &= x-1;
	return c;
}

// maps a given bucket to a cpu, given max number of buckets per cpu
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
void radix_omp(vector<unsigned int> &arr, const unsigned int num_threads, const unsigned int g) {

	const unsigned int b	= (1 << g);				// num of buckets (2^g)
	const unsigned int bpp	= b / num_threads;		// num of buckets per thread
	const unsigned int bpp_bc = popcount(bpp - 1);	// number of bits in bpp. used to compute BUCKET_TO_CPU

	unsigned int mask = ((1 << g) - 1);		// initial mask to get key


	vector<vector<unsigned int> > buckets(b);		// the buckets
	vector<unsigned int> bucket_accum;	// accumulated values for bucket_count. indicates where the values for each bucket should go
	bucket_accum[0] = 0;

	omp_set_num_threads(num_threads);

	#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();

		for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {

			// clear buckets for this thread
			for(unsigned int i = 0; i < bpp; ++i) {
				buckets[thread_num * bpp + i].clear();
			}

			// fill buckets
			for(unsigned int i = 0; i < arr.size(); ++i) {
				unsigned int elem = arr[i];
				unsigned int bucket = GET_BUCKET_NUM(elem, mask, g, round);

				// if this bucket is handled by this thread, insert
				if (BUCKET_TO_CPU(bucket) == thread_num)
					buckets[bucket].push_back(elem);
			}

			#pragma omp barrier

			#pragma omp master
			{
				unsigned int sum = buckets[0].size();
				for(unsigned int i = 1; i < b; ++i) {
					bucket_accum[i] = sum;
					sum += buckets[i].size();
				}
			}

			#pragma omp barrier

			for(unsigned int i = 0; i < bpp; ++i) {
				unsigned int bucket = thread_num * bpp + i;
				unsigned int base_index = bucket_accum[bucket];

				for(unsigned int elem = 0; elem < buckets[bucket].size(); ++elem) {
					arr[base_index + elem] = buckets[bucket][elem];
				}
			}

			#pragma omp barrier
		}
	}
}

int check_array_order(vector<unsigned int> &arr) {
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
	else			len = LEN;

	if (argc > 2)	g = atoi(argv[2]);
	else			g = 4;

	if (argc > 3)	size = atoi(argv[3]);
	else			size = 4;

	cerr
		<< "array len   = " << size << endl
		<< "g           = "	<< g << endl
		<< "num_threads = " << size << endl;

	vector<unsigned int> arr(len);
	// generate test data
	test_data(arr);

	// the real stuff
	cerr << "starting radix sort...";
	timer.start();
	radix_omp(arr, size, g);
	timer.stop();
	cerr << "finished" << endl << endl;

	cerr << arr_str(arr) << endl;
	// check array order
	int order = check_array_order(arr);
	switch (order) {
		case ORDER_CORRECT: 	cerr << "CORRECT! Result is ordered" << endl; break;
		default: 				cerr << "WRONG! Order fails at index " << order << endl; break;
	}

	// print time for each process
	cout << timer.get() * 1.0e-3 << " usec" << endl;
}
