/**
 * \file sort.seq.cpp
 */

#include "sort.h"
#include <algorithm>
#include <vector>
#include "common.h"
#include <iostream>
#include <mpi.h>
using namespace std;

#define GET_BUCKET_NUM(elem, mask, g, i) ((elem) & (mask) >> ((g) * (i)))

void test_data(vector<int> &arr, int id) {
	switch (id) {
		case 0:
			arr[0] = 0;
			arr[1] = 1;
			arr[2] = 1;
			arr[3] = 1;
			arr[4] = 2;
			arr[5] = 2;
			arr[6] = 2;
			arr[7] = 2;
			arr[8] = 3;
			arr[9] = 3;
			break;
		case 1:
			arr[0] = 0;
			arr[1] = 0;
			arr[2] = 0;
			arr[3] = 1;
			arr[4] = 1;
			arr[5] = 1;
			arr[6] = 1;
			arr[7] = 1;
			arr[8] = 1;
			arr[9] = 2;
			break;
		case 2:
			arr[0] = 1;
			arr[1] = 1;
			arr[2] = 1;
			arr[3] = 2;
			arr[4] = 2;
			arr[5] = 2;
			arr[6] = 2;
			arr[7] = 2;
			arr[8] = 3;
			arr[9] = 3;
			break;
		case 3:
			arr[0] = 0;
			arr[1] = 1;
			arr[2] = 1;
			arr[3] = 2;
			arr[4] = 2;
			arr[5] = 3;
			arr[6] = 3;
			arr[7] = 3;
			arr[8] = 3;
			arr[9] = 3;
			break;
	}
}

void sort(int argc, char *argv[], int len, Timer timer) {
	MPI_Status status;
	int id, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	unsigned int p = size;					// num of processors
	unsigned int n = len;					// problem size	
	unsigned int n_per_cpu = n / p;			// number of elements per cpu
	unsigned int g = 4;						// num bits for each pass
	unsigned int b = (1 << g);				// num of buckets (2^g)
	unsigned int r = b / g;					// num of rounds

	unsigned int mask = ((1 << g) - 1);		// initial mask to get key

	vector<int> arr(10);

	// generate test data
	test_data(arr, id);

	// it is assumed that B = P, only one bucket per processor
	vector<int> bin_counts(r);		  // bin counts for each processor
	vector<int> bin_counts_trans(p);  // bin counts transposed

	for(unsigned int i = 0; i < r; ++i)
		bin_counts[i] = 0;

	for(unsigned int i = 0; mask != 0; mask <<= g, ++i) {

		// perform bin count
		for(vector<int>::iterator elem = arr.begin(); elem != arr.end(); ++elem) {
			unsigned int bucket = GET_BUCKET_NUM(*elem, mask, g, i);
			bin_counts[bucket]++;
		}

		MPI_Alltoall(&bin_counts[0], 1, MPI_INT, &bin_counts_trans[0], 1, MPI_INT, MPI_COMM_WORLD);
	}


	return;
	// create buckets
	vector<vector<int> > buckets(b);

	// find max element
	int max   = *max_element(arr.begin(), arr.end());
	//int pow10 = 1;

	// begin radix sort
	for(unsigned int i = 0; mask != 0; mask <<= g, ++i) {

		// determine which bucket each element should enter
		for(vector<int>::iterator elem = arr.begin(); elem != arr.end(); ++elem) {

			// calculate bucket number
			size_t const bucket_num = (*elem & mask) >> (g * i);
			// add the element to the list in the bucket
			buckets[ bucket_num ].push_back(*elem);
		}

		// transfer results of buckets back int main array
		vector<int>::iterator store_pos = arr.begin();

		for(vector<vector<int> >::iterator bucket = buckets.begin(); bucket != buckets.end(); ++bucket) {

			// for each element in this bucket
			for(vector<int>::iterator bucket_elem = bucket->begin(); bucket_elem != bucket->end(); ++bucket_elem) {
				*store_pos++ = *bucket_elem;
			}

			// clear the current bucket
			bucket->clear();
		}
	}

	MPI_Finalize();
}

