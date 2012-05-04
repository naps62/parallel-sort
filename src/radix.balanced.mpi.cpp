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

void test_data(vector<int> &arr, int id, int size) {
	srand(time(NULL) * (id + 1));
	for(unsigned int i = 0; i < arr.size(); ++i)
		arr[i] = rand() % 100;
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
void radix_mpi(vector<int> &arr, const int id, const int p, const unsigned int g) {

	//unsigned int p = size;				// num of processors
	//unsigned int g = 2;					// num bits for each pass
	const unsigned int b		= (1 << g);		// num of buckets (2^g)
	const unsigned int bpp	= b / p;			// num of buckets per cpu
	const unsigned int bpp_bc = popcount(bpp - 1);	// number of bits in bpp. used to compute BUCKET_TO_CPU
	const unsigned int n = arr.size();			// problem size	

	unsigned int mask = ((1 << g) - 1);		// initial mask to get key


	vector<vector<int> > buckets(b);		// the buckets
	vector<vector<int> > bucket_counts;		// bin counts for each processor
	for(unsigned int i = 0; i < b; ++i)
		bucket_counts.push_back(vector<int>(p));

	vector<int> bucket_counts_aux(b); 		// aux vector, has only bucket counts for this process

	// dummy, MPI asks for them
	MPI_Request request;  // request handles for key communication
	MPI_Status  status;	  // status of key communication recvs

	for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {
		// CALCULATE BUCKET COUNTS

		// clear buckets
		for(unsigned int buck = 0; buck < b; ++buck) {
			bucket_counts_aux[buck] = 0;
			bucket_counts[buck][id] = 0;
			buckets[buck].clear();
		}

		// fill buckets and bucket_count
		for(unsigned int buck = 0; buck < arr.size(); ++buck) {
			int elem = arr[buck];
			unsigned int bucket = GET_BUCKET_NUM(elem, mask, g, round);
			bucket_counts_aux[bucket]++;
			bucket_counts[bucket][id]++;
			buckets[bucket].push_back(elem);
		}

		// SEND/RECV BUCKET COUNTS

		// sends my bucket counts to all other processes
		for(unsigned int proc = 0; proc < p; ++proc) {
			if (proc != id)
				MPI_Isend(&bucket_counts_aux[0], b, MPI_INT, proc, TAG_BUCKET_COUNT, MPI_COMM_WORLD, &request);
		}
		// recv bucket counts from other processes
		for(unsigned int proc = 0; proc < p; ++proc) {
			if (proc != id) {
				MPI_Recv(&bucket_counts_aux[0], b, MPI_INT, proc, TAG_BUCKET_COUNT, MPI_COMM_WORLD, &status);
				// copy from aux array to global matrix
				for(unsigned int buck = 0; buck < b; ++buck)
					bucket_counts[buck][proc] = bucket_counts_aux[buck];
			}
		}

		// SEND KEYS
		int max		= arr.size();	// max number of elems to copy to each destination
		int current	= 0;			// current destination proc
		int left	= max;			// how many elements left to send to this proc
		int dest	= 0;			// current destination
		int off		= 0;			// number of elems already copied from current bucket to the previous proc
		for(unsigned int buck = 0; buck < b; ++buck) {
			for(unsigned int proc = 0; proc < p; ++proc) {
	
				if (left == -1) left = max;

				unsigned int size = bucket_counts[buck][proc] - off;// number of elems left to copy from this buck/proc
				if (size > left) { // if there are more elements to copy than those who are left, limit size and adjust off for next copy
					size = left;
				}
				
				if (size > 0) {
					if (proc == dest) {	// if destination is the same as source, then that proc simply copy values
						if (id == dest)	{
							memcpy(&(arr[current]), &(buckets[buck][off]), size * sizeof(int));
						}
					} else {
						if (id == proc) {			// if its sender
							MPI_Isend(&(buckets[buck][off]), size, MPI_INT, dest, TAG_KEY_SEND, MPI_COMM_WORLD, &request);
						} else if (id == dest) {	// if its receiver
							MPI_Recv(&(arr[current]), 		size, MPI_INT, proc, TAG_KEY_SEND, MPI_COMM_WORLD, &status);
						}
					}
				}

				current += size;
				off  	+= size;
				left 	-= size;	// subtract size from elements left to copy to this proc

				// if no more items left no this proc, move to the next
				if (left == 0) {
					dest++;
					current = 0;
					//cout << "current " << dest  << " buck " << buck << ", proc " << proc << endl;
					left = max;
				}
				
				// if it not the end of this buck/proc pair
				if (off != bucket_counts[buck][proc]) {
					proc--;
				} else {
					off = 0;
				}
			}
		}
	}
}

int check_array_order(vector<int> &arr, int id, int size) {

	// check local array order
	for(unsigned int i = 1; i < arr.size(); ++i)
		if (arr[i - 1] > arr[i])
			return i;

	int is_ordered = 1, reduce_val;
	int next_val;
	MPI_Request request;
	MPI_Status status;

	// all processes but first send first element to previous process
	if (id > 0) {
		// if local array is size 0, send max_int
		int val_to_send = (arr.size() == 0) ? numeric_limits<int>::max() : arr[0];
		MPI_Isend(&val_to_send, 1, MPI_INT, id - 1, TAG_CHECK_ORDER, MPI_COMM_WORLD, &request);
	}

	// all processes but last receive element from next process and compare it to their last one
	if (id < size - 1) {
		MPI_Recv(&next_val, 1, MPI_INT, id + 1, TAG_CHECK_ORDER, MPI_COMM_WORLD, &status);

		// this link is ordered if last local value is <= than received value, or if local size is 0
		is_ordered = (arr.size() == 0 || arr.back() <= next_val);
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

void ordered_print(string str, int id, int size) {
	int buff_size;
	// if master, receive data and print it
	if (id == 0) {
		MPI_Status status;
		char *buff;
		for(unsigned int i = 0; i < size; ++i) {
			if (i != id) {
				MPI_Recv(&buff_size, 1, MPI_INT, i, TAG_COUT_SIZE, MPI_COMM_WORLD, &status);
				buff = new char[buff_size+1];
				buff[buff_size] = '\0';
				MPI_Recv(&buff, buff_size, MPI_BYTE, i, TAG_COUT_DATA, MPI_COMM_WORLD, &status);
				cout << buff;
			} else {
				cout << str;
			}
		}
	}

	// else send it to master
	else {
		buff_size = str.size();
		MPI_Send(&buff_size,  1,          MPI_INT,  0, TAG_COUT_SIZE, MPI_COMM_WORLD);
		MPI_Send(&buff_size, str.size(), MPI_BYTE, 0, TAG_COUT_DATA, MPI_COMM_WORLD);
	}
}

int main(int argc, char **argv) {
	
	int g = 4;

	int id, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	Timer timer;
	int len;
	stringstream ss;

	if (argc > 1)	len = atoi(argv[1]);
	else			len = LEN;

	if (argc > 2)	g = atoi(argv[1]);
	else			cout << "defaulting to mask size g=" << g << endl;

	vector<int> arr(len / size);// = new vector<int>(len / size);
	// generate test data
	test_data(arr, id, size);

	ss << "[" << id << "] initial array: " << " " << arr_str(arr) << endl;
	//ordered_print(ss.str(), id, size);
	cout << ss.str();
	ss.str("");

	MPI_Barrier(MPI_COMM_WORLD);

	timer.start();
	radix_mpi(arr, id, size, g);
	timer.stop();

	MPI_Barrier(MPI_COMM_WORLD);

	ss << "[" << id << "]";
	if (arr.size() > 0)
		ss << arr_str(arr) << endl;
	//ordered_print(ss.str(), id, size);
	cout << ss.str();
	ss.str("");

	MPI_Barrier(MPI_COMM_WORLD);

	/*int order = check_array_order(arr, id, size);
	int order = ORDER_CORRECT;

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
	cout << id << endl;
	MPI_Barrier(MPI_COMM_WORLD);*/

	ss << "id: " << id << " - solution took: " << timer.get() * 1.0e-3 << " usec" << endl;
	//ordered_print(ss.str(), id, size);
	cout << ss.str();

	MPI_Finalize();
}
