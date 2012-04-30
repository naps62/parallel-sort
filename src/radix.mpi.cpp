/**
 * \file sort.seq.cpp
 */

#include "common.h"
#include <stdlib.h>
#include <vector>
#include <limits>
#include <iostream>
#include <mpi.h>
using namespace std;

#define GET_BUCKET_NUM(elem, mask, g, i) (((elem) & (mask)) >> ((g) * (i)))

enum OrderCodes {
	ORDER_CORRECT = 0,
	ORDER_ERROR_BORDERS = -1,
	ORDER_ONLY_MASTER = -2,
};

enum Tags {
	TAG_KEY_SEND = 1,
	TAG_CHECK_ORDER = 2,
};

void test_data(vector<int> *arr, int id, int size) {
	srand(time(NULL));
	for(unsigned int i = 0; i < arr->size(); ++i)
		(*arr)[i] = rand();
	/*switch (id) {
		case 0:
			(*arr)[0] = 0x0000;
			(*arr)[1] = 0x0001;
			(*arr)[2] = 0x0001;
			(*arr)[3] = 0x0001;
			(*arr)[4] = 0x0002;
			(*arr)[5] = 0x0002;
			(*arr)[6] = 0x0002;
			(*arr)[7] = 0x0002;
			(*arr)[8] = 0x0003;
			(*arr)[9] = 0x0003;
			break;
		case 1:
			(*arr)[0] = 0x0000;
			(*arr)[1] = 0x0000;
			(*arr)[2] = 0x0000;
			(*arr)[3] = 0x0001;
			(*arr)[4] = 0x0001;
			(*arr)[5] = 0x00E4;
			(*arr)[6] = 0x0001;
			(*arr)[7] = 0x0001;
			(*arr)[8] = 0x0001;
			(*arr)[9] = 0x0002;
			break;
		case 2:
			(*arr)[0] = 0x0001;
			(*arr)[1] = 0x0001;
			(*arr)[2] = 0x0001;
			(*arr)[3] = 0x0002;
			(*arr)[4] = 0x0002;
			(*arr)[5] = 0x0002;
			(*arr)[6] = 0x0002;
			(*arr)[7] = 0x0002;
			(*arr)[8] = 0x0003;
			(*arr)[9] = 0x0003;
			break;
		case 3:
			(*arr)[0] = 0x0000;
			(*arr)[1] = 0x0001;
			(*arr)[2] = 0x0001;
			(*arr)[3] = 0x0002;
			(*arr)[4] = 0x0002;
			(*arr)[5] = 0x0003;
			(*arr)[6] = 0x0003;
			(*arr)[7] = 0x0003;
			(*arr)[8] = 0x0003;
			(*arr)[9] = 0x0003;
			break;
	}*/
}

/**
 * it is assumed that B = P, only one bucket per processor
 */
void radix_mpi(vector<int> *&arr, int id, int size) {

	unsigned int p = size;					// num of processors
	unsigned int n = arr->size();			// problem size	
	unsigned int n_per_cpu = n / p;			// number of elements per cpu
	unsigned int g = 2;						// num bits for each pass
	unsigned int b = (1 << g);				// num of buckets (2^g)
	unsigned int r = b / g;					// num of rounds

	unsigned int mask = ((1 << g) - 1);		// initial mask to get key


	vector<vector<int> > buckets(b);  // the buckets
	vector<int> bin_counts(b);		  // bin counts for each processor
	vector<int> bin_counts_trans(p);  // bin counts transposed

	vector<int> bin_counts_accum(p);  // accumulated values for bin_count. indicates for each process, where the values for this bucket should go
	vector<int> *this_bucket;

	vector<MPI_Request> requests(p);  // request handles for key communication
	vector<MPI_Status>  status(p);	  // status of key communication recvs

	for(unsigned int i = 0; i < r; ++i)
		bin_counts[i] = 0;

	for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {

		// clear buckets
		for(unsigned int i = 0; i < bin_counts.size(); ++i) {
			bin_counts[i] = 0;
			buckets[i].clear();
		}

		// fill buckets and bin_count
		for(vector<int>::iterator elem = arr->begin(); elem != arr->end(); ++elem) {
			unsigned int bucket = GET_BUCKET_NUM(*elem, mask, g, round);
			bin_counts[bucket]++;
			buckets[bucket].push_back(*elem);
		}

		bin_counts_trans[id] = bin_counts[id];
		// transpose bin_count info
		MPI_Alltoall(&bin_counts[0], 1, MPI_INT, &bin_counts_trans[0], 1, MPI_INT, MPI_COMM_WORLD);

		// count total size of bucket for this process, and alloc it. also compute bin_counts_accum
		int total_bucket_size = bin_counts_trans[0];
		bin_counts_accum[0] = 0;
		for(unsigned int i = 1; i < bin_counts_trans.size(); ++i) {
			bin_counts_accum[i] = total_bucket_size;
			total_bucket_size += bin_counts_trans[i];
		}

		this_bucket = new vector<int>(total_bucket_size);

		// send keys across each process
		for(unsigned int i = 0; i < b; ++i) {
			// send data from a single bucket to its corresponding process
			if (i != id) {
				MPI_Isend(&(buckets[i][0]), buckets[i].size(), MPI_INT, i, TAG_KEY_SEND, MPI_COMM_WORLD, &requests[i]);
			}
		}

		// recv keys
		for(unsigned int i = 0; i < p; ++i) {
			// if its the same process, copy data from buckets[i] to this_bucket
			if (i == id)
				memcpy(&(*this_bucket)[ bin_counts_accum[i] ], &(buckets[i][0]), buckets[i].size() * sizeof(int));

			// otherwise recv data from process i
			else
				MPI_Recv(&(*this_bucket)[ bin_counts_accum[i] ], bin_counts_trans[i], MPI_INT, i, TAG_KEY_SEND, MPI_COMM_WORLD, &status[i]);
		}

		delete arr;
		arr = this_bucket;

	}

	if (arr->size() > 0)
		cout << id << " " << arr_str(*arr) << endl;
	
}

int check_array_order(vector<int> *&arr, int id, int size) {

	// check local array order
	for(unsigned int i = 1; i < arr->size(); ++i)
		if ((*arr)[i - 1] > (*arr)[i])
			return i;

	int is_ordered = 1, reduce_val;
	int next_val;
	MPI_Request request;
	MPI_Status status;

	// all processes but first send first element to previous process
	if (id > 0) {
		// if local array is size 0, send max_int
		int val_to_send = (arr->size() == 0) ? numeric_limits<int>::max() : (*arr)[0];
		MPI_Isend(&val_to_send, 1, MPI_INT, id - 1, TAG_CHECK_ORDER, MPI_COMM_WORLD, &request);
	}

	// all processes but last receive element from next process and compare it to their last one
	if (id < size - 1) {
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

int main(int argc, char **argv) {
	int id, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	Timer timer;
	int len;

	if (argc > 1)	len = atoi(argv[1]);
	else			len = LEN;

	vector<int> *arr = new vector<int>(len / size);
	// generate test data
	test_data(arr, id, size);

	cout << "[" << id << "] -> " << arr->size() << " " << arr_str(*arr) << endl;

	MPI_Barrier(MPI_COMM_WORLD);

	timer.start();
	radix_mpi(arr, id, size);
	timer.stop();

	int order = check_array_order(arr, id, size);
	switch (order) {
		case ORDER_CORRECT:
			cout << "[" << id << "] CORRECT! Result is ordered" << endl;
			break;
		case ORDER_ERROR_BORDERS:
			cout << "WRONG IN BORDERS!" << endl;
			break;
		case ORDER_ONLY_MASTER:
			break;
		default:
			cout << "[" << id << "] WRONG! Order fails at index " << order << endl;
			break;
	}
	
	MPI_Barrier(MPI_COMM_WORLD);

	cout << "id: " << id << " - solution took: " << timer.get() * 1.0e-3 << " usec" << endl;

	MPI_Finalize();
}
