/**
 * \file sort.seq.cpp
 */

#include "common.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <iomanip>
using namespace std;

void radix(vector<unsigned int> &arr, unsigned int g) {

	unsigned int num_buckets = (1 << g); // num of buckets (2^g)
	unsigned int mask = ((1 << g) - 1);	 // initial mask to get key

	// create buckets
	vector<vector<unsigned int> > buckets(num_buckets);

	// begin radix sort
	for(unsigned int i = 0; mask != 0; mask <<= g, ++i) {

		// determine which bucket each element should enter
		for(vector<unsigned int>::iterator elem = arr.begin(); elem != arr.end(); ++elem) {

			// calculate bucket number
			size_t const bucket_num = (*elem & mask) >> (g * i);
			// add the element to the list in the bucket
			buckets[ bucket_num ].push_back(*elem);
		}

		// transfer results of buckets back int main array
		vector<unsigned int>::iterator store_pos = arr.begin();

		for(vector<vector<unsigned int> >::iterator bucket = buckets.begin(); bucket != buckets.end(); ++bucket) {

			// for each element in this bucket
			for(vector<unsigned int>::iterator bucket_elem = bucket->begin(); bucket_elem != bucket->end(); ++bucket_elem) {
				*store_pos++ = *bucket_elem;
			}

			// clear the current bucket
			bucket->clear();
		}
	}
}

int check_array_order(vector<unsigned int> &arr) {
	for (unsigned int i = 1; i < arr.size(); ++i)
		if (arr[i - 1] > arr[i])
			return i;
	return 0;
}


int main(int argc, char **argv) {

	Timer timer;
	int len;
	unsigned int g;

	if (argc > 1)	len = atoi(argv[1]);
	else			len = LEN;

	if (argc > 2)   g = atoi(argv[2]);
	else			g = 4;

	vector<unsigned int> arr(len);
	read_arr(arr, 0);
	//cerr << "starting test with array of size " << arr.size() << endl;
	//cerr << "initial data: " << arr_str(arr) << endl;

	timer.start();
	radix(arr, g);
	timer.stop();

	int order = check_array_order(arr);
	if (order == 0)
		cerr << "CORRECT! Result is ordered" << endl;
	else
		cerr << "WRONG! Order fails at index " << order << endl;

	cout << timer.get() * 1.0e-3 << endl;
}

