/**
 * \file sort.seq.cpp
 */

#include <vector>
#include <algorithm>
#include "seeds.h"
#include "common.h"
using namespace std;

void radix_sort(vector<int> &arr) {

	// create buckets
	vector<vector<int> > buckets(10);

	// find max element
	int max   = *max_element(arr.begin(), arr.end());
	int pow10 = 1;

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

int main(int argc, char **argv) {

	int len;
	if (argc > 1)
		len = atoi(argv[1]);
	else {
		cout << "using default array length: " << LEN << endl << endl;
		len = LEN;
	}

	vector<int> arr = get_test_array(len);
	Timer timer;

	cout << "Sequential Radix sort" << endl;
	//cout << "input:  "; dump_arr(arr);

	timer.start();
	radix_sort(arr);
	timer.stop();

	//cout << "output: "; dump_arr(arr);
	if (check_array_order(arr))
		cout << "CORRECT! output is ordered" << endl;
	else
		cout << "WRONG! output array is not ordered" << endl;

	cout << "solution took: " << timer.get() * 1.0e-3 << " usec" << endl;
	cout << endl << "exiting" << endl;
}
