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
#include <mpi.h>
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

void test_data(vector<unsigned int> *arr, int id, int size) {
	srand(time(NULL) * (id + 1));
	for(unsigned int i = 0; i < arr->size(); ++i)
		(*arr)[i] = rand() % 1000;
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
void radix_mpi(vector<unsigned int> *&arr, const unsigned int id, const unsigned int p, const unsigned int g) {

	//unsigned int p = size;				// num of processors
	//unsigned int g = 2;					// num bits for each pass
	const unsigned int b		= (1 << g);		// num of buckets (2^g)
	const unsigned int bpp	= b / p;			// num of buckets per cpu
	const unsigned int bpp_bc = popcount(bpp - 1);	// number of bits in bpp. used to compute BUCKET_TO_CPU

	unsigned int mask = ((1 << g) - 1);		// initial mask to get key


	vector<vector<unsigned int> > buckets(b);		// the buckets
	vector<vector<unsigned int> > bucket_counts;		// bin counts for each processor
	for(unsigned int i = 0; i < b; ++i)
		bucket_counts.push_back(vector<unsigned int>(p));

	vector<unsigned int> bucket_counts_aux(b); 		// aux vector, has only bucket counts for this process

	vector<vector<unsigned int> > bucket_accum;	// accumulated values for bucket_count. indicates for each process, where the values for each bucket should go
	for(unsigned int i = 0; i < bpp; ++i)
		bucket_accum.push_back(vector<unsigned int>(p));

	vector<unsigned int> bucket_sizes(bpp);
	vector<unsigned int> *this_bucket;			


	// dummy, MPI asks for them
	MPI_Request request;  // request handles for key communication
	MPI_Status  status;	  // status of key communication recvs

	for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {
		// CALCULATE BUCKET COUNTS

		// clear buckets
		for(unsigned int i = 0; i < b; ++i) {
			bucket_counts_aux[i] = 0;
			bucket_counts[i][id] = 0;
			buckets[i].clear();
		}

		// fill buckets and bucket_count
		for(unsigned int i = 0; i < arr->size(); ++i) {
			unsigned int elem = (*arr)[i];
			unsigned int bucket = GET_BUCKET_NUM(elem, mask, g, round);
			bucket_counts_aux[bucket]++;
			bucket_counts[bucket][id]++;
			buckets[bucket].push_back(elem);
		}

		// SEND/RECV BUCKET COUNTS

		// sends my bucket counts to all other processes
		for(unsigned int i = 0; i < p; ++i) {
			if (i != id)
				MPI_Isend(&bucket_counts_aux[0], b, MPI_INT, i, TAG_BUCKET_COUNT, MPI_COMM_WORLD, &request);
		}
		// recv bucket counts from other processes
		for(unsigned int i = 0; i < p; ++i) {
			if (i != id) {
				MPI_Recv(&bucket_counts_aux[0], b, MPI_INT, i, TAG_BUCKET_COUNT, MPI_COMM_WORLD, &status);
				// copy from aux array to global matrix
				for(unsigned int k = 0; k < b; ++k)
					bucket_counts[k][i] = bucket_counts_aux[k];
			}
		}

		// CALCULATE BUCKET_ACCUMS

		// count total size of bucket for this process, and alloc it. also compute bucket_accum
		int total_bucket_size = 0;
		for(unsigned int i = 0; i < bpp; ++i) {
			int single_bucket_size = 0;
			int global_bucket = i + id*bpp;

			for(unsigned int j = 0; j < p; ++j) {
				bucket_accum[i][j] = total_bucket_size;
				single_bucket_size += bucket_counts[global_bucket][j];
				total_bucket_size  += bucket_counts[global_bucket][j];
			}
			bucket_sizes[i] = single_bucket_size;
		}

		this_bucket = new vector<unsigned int>(total_bucket_size);

		// send keys across each process
		for(unsigned int i = 0; i < b; ++i) {
			unsigned int dest = BUCKET_TO_CPU(i);
			unsigned int local_bucket = BUCKET_IN_CPU(i);
			// send data from a single bucket to its corresponding process
			if (dest != id && buckets[i].size() > 0) {
				MPI_Isend(&(buckets[i][0]), buckets[i].size(), MPI_INT, dest, local_bucket, MPI_COMM_WORLD, &request);
			}
		}

		// recv keys
		for(unsigned int b = 0; b < bpp; ++b) {
			unsigned int global_bucket = b + id*bpp;

			for(unsigned int i = 0; i < p; ++i) {
				unsigned int bucket_size = bucket_counts[global_bucket][i];

				if (bucket_size > 0) {
					unsigned int *dest = &(*this_bucket)[ bucket_accum[b][i] ];
					// if its the same process, copy data from buckets[i] to this_bucket
					if (i == id) {
						memcpy(dest, &(buckets[global_bucket][0]), bucket_size * sizeof(int));
					}
	
					// otherwise recv data from process i
					else {
						MPI_Recv(dest, bucket_size, MPI_INT, i, b, MPI_COMM_WORLD, &status);
					}
				}
			}
		}

		delete arr;
		arr = this_bucket;
	}
}

int check_array_order(vector<unsigned int> *&arr, unsigned int id, unsigned int size) {

	// check local array order
	for(unsigned int i = 1; i < arr->size(); ++i)
		if ((*arr)[i - 1] > (*arr)[i])
			return i;

	int is_ordered = 1, reduce_val;
	unsigned int next_val;
	MPI_Request request;
	MPI_Status status;

	// all processes but first send first element to previous process
	if (id > 0) {
		// if local array is size 0, send max_int
		int val_to_send = (arr->size() == 0) ? numeric_limits<int>::max() : (*arr)[0];
		MPI_Isend(&val_to_send, 1, MPI_INT, id - 1, TAG_CHECK_ORDER, MPI_COMM_WORLD, &request);
	}

	// all processes but last receive element from next process and compare it to their last one
	if (id < (unsigned int) size - 1) {
		MPI_Recv(&next_val, 1, MPI_INT, id + 1, TAG_CHECK_ORDER, MPI_COMM_WORLD, &status);

		// this link is ordered if last local value is <= than received value, or if local size is 0
		is_ordered = (arr->size() == 0 || arr->back() <= next_val);
	}

	// reduce all values, to check if border order is met
	MPI_Reduce(&is_ordered, &reduce_val, 1, MPI_INT, MPI_LAND, 0, MPI_COMM_WORLD);

	// reduce result only goes to process 0
	if (id == 0) {
		if (reduce_val)
			return ORDER_CORRECT;
		else
			return ORDER_ERROR_BORDERS;
	}

	return ORDER_ONLY_MASTER;
}

#define MSG_SIZE 100

void ordered_print(char *str, unsigned int id, unsigned int size) {
	// if master, receive data and print it
	if (id == 0) {
		cout << str;
		MPI_Status status;
		for(unsigned int i = 1; i < size; ++i) {
			char buff[MSG_SIZE];
			MPI_Recv(buff, MSG_SIZE, MPI_BYTE, i, TAG_COUT_DATA, MPI_COMM_WORLD, &status);
			cout << buff;
		}
	}

	// else send it to master
	else {
		MPI_Send(str, MSG_SIZE, MPI_BYTE, 0, TAG_COUT_DATA, MPI_COMM_WORLD);
	}
}

int main(int argc, char **argv) {
	
	int g = 4;

	char msg[MSG_SIZE];
	int id, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	Timer timer;
	int len;
	stringstream ss;

	if (argc > 1)	len = atoi(argv[1]);
	else			len = LEN;

	if (argc > 2)	g = atoi(argv[2]);

	if (id == 0) cout << "mask size = " << g << endl << endl;

	vector<unsigned int> *arr = new vector<unsigned int>(len / size);
	// generate test data
	test_data(arr, id, size);

	// the real stuff
	if (id == 0) cout << "starting radix sort...";
	MPI_Barrier(MPI_COMM_WORLD);
	timer.start();
	radix_mpi(arr, id, size, g);
	timer.stop();
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 0) cout << "finished" << endl << endl;

	if (id == 0 && arr->size() > 0) cout << "[" << id << "] " << arr_str(*arr) << endl;
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 1 && arr->size() > 0) cout << "[" << id << "] " << arr_str(*arr) << endl;
	// check array order
	int order = ORDER_CORRECT;//check_array_order(arr, id, size);
	switch (order) {
		case ORDER_CORRECT: 	cout << "CORRECT! Result is ordered" << endl; break;
		case ORDER_ONLY_MASTER: break;
		default: 				cout << "WRONG! Order fails at index " << order << endl; break;
	}

	// print time for each process
	sprintf(msg, "%d: %lf usec\n", id, timer.get() * 1.0e-3);
	ordered_print(msg, id, size);
	MPI_Finalize();
}
