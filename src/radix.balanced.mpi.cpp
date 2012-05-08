/**
 * \file sort.seq.cpp
 */

#include "common.h"
#include <stdlib.h>
#include <vector>
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <mpi.h>
using namespace std;


// given a key and a mask, calcs the bucket to where the key belongs

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
		arr[i] = rand();
}

// count number of bits set to 1 in a number
unsigned int popcount(unsigned int x) {
	unsigned int c;
	for(c = 0; x; ++c) x &= x-1;
	return c;
}

#define GET_BUCKET_NUM(elem, mask, g, i) (((elem) & (mask)) >> ((g) * (i)))

/**
 * it is assumed that B = P, only one bucket per processor
 *
 * \param arr Where to stop the ordered array (one local array per processor)
 * \param id  Id of this processs
 * \param p   Number of processor (equivalent to MPI_size)
 * \param g   Number of bits to use for each mask (must be power of 2)
 */
void radix_mpi(vector<int> &arr, const unsigned int id, const unsigned int p, const unsigned int g) {

	const unsigned int b	= (1 << g);			// num of buckets (2^g)
	unsigned int mask 		= ((1 << g) - 1);	// initial mask to get key


	vector<vector<int> > buckets(b);				// the buckets
	vector<vector<unsigned int> > bucket_counts;	// bin counts for each processor
	for(unsigned int i = 0; i < b; ++i)
		bucket_counts.push_back(vector<unsigned int>(p));

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
		unsigned int max		= arr.size();	// max number of elems to copy to each destination
		unsigned int current	= 0;			// current destination proc
		unsigned int left	= max;			// how many elements left to send to this proc
		unsigned int dest	= 0;			// current destination
		unsigned int off		= 0;			// number of elems already copied from current bucket to the previous proc
		for(unsigned int buck = 0; buck < b; ++buck) {
			for(unsigned int proc = 0; proc < p; ++proc) {

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

int check_array_order(vector<int> &arr, unsigned int id, unsigned int size) {

	// master receives full array
	if (id == 0) {
		MPI_Status status;
		vector<int> full_arr(arr.size() * size);

		memcpy(&full_arr[0], &arr[0], arr.size() * sizeof(int));
		for(unsigned int i = 1; i < size; ++i) {
			MPI_Recv(&full_arr[arr.size() * i], arr.size(), MPI_INT, i, TAG_CHECK_ORDER, MPI_COMM_WORLD, &status);
		}

		cout << "final result: " << arr_str(full_arr) << endl;

		// check local array order
		for(unsigned int i = 1; i < full_arr.size(); ++i)
			if (full_arr[i - 1] > full_arr[i])
				return i;

		return 0;

	// others only send their array to master
	} else {
		MPI_Send(&arr[0], arr.size(), MPI_INT, 0, TAG_CHECK_ORDER, MPI_COMM_WORLD);
		return ORDER_ONLY_MASTER;
	}
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

	if (argc > 2)	g = atoi(argv[1]);

	if (id == 0) cout << "mask size = " << g << endl << endl;

	// initialize data for this proc
	vector<int> arr(len / size);
	test_data(arr, id, size);

	// the real stuff
	if (id == 0) cout << "starting radix sort...";
	MPI_Barrier(MPI_COMM_WORLD);
	timer.start();
	radix_mpi(arr, id, size, g);
	timer.stop();
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 0) cout << "finished" << endl << endl;

	// check array order
	int order = check_array_order(arr, id, size);
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
