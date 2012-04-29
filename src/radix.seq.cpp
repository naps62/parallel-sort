/**
 * \file sort.seq.cpp
 */

#include "sort.h"
#include <algorithm>
#include <vector>
#include "common.h"
#include <iostream>
using namespace std;

void sort(int argc, char *argv[], int len, Timer timer) {
	timer.start();
	vector<int> arr = get_test_array(len);

	unsigned int p = 1;					 // num of processors
	unsigned int n = arr.size();		 // problem size
	unsigned int g = 4; 				 // num bits for each pass
	unsigned int num_buckets = (1 << g); // num of buckets (2^g)
	unsigned int mask = ((1 << g) - 1);	 // initial mask to get key

	// create buckets
	vector<vector<int> > buckets(num_buckets);

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

	timer.stop();
}

/*
void sort(vector<int> &arr, int argc, char *argv[]) {
	unsigned int p = 1;					 // num of processors
	unsigned int n = arr.size();		 // problem size
	unsigned int g = 4; 				 // num bits for each pass
	unsigned int num_buckets = (2 << g); // num of buckets (2^g)

	// create buckets
	vector<vector<int> > buckets(num_buckets);

	// find max element
	int max   = *max_element(arr.begin(), arr.end());
	//int pow10 = 1;

	// begin radix sort
	for(; max != 0; max /= 10, pow10*=10) {

		// determine which bucket each element should enter
		for(vector<int>::iterator elem = arr.begin(); elem != arr.end(); ++elem) {

			// calculate bucket number
			size_t const bucket_num = ( *elem / pow10) % 10;
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
}
*/
