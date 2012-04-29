/**
 * \file sort.seq.cpp
 */

#include "sort.h"
#include <algorithm>
#include <vector>
#include "common.h"
#include <iostream>
using namespace std;

inline int swap(int &x, int &y) {
	int tmp = x;
	x = y;
	y = tmp;
}

int partition(vector<int> &arr, int left, int right, int pivot_index) {
	int pivot = arr[pivot_index];

	while(left <= right) {

		while(arr[left] < pivot)  ++left;
		while(arr[right] > pivot) --right;

		if (left < right) {
			swap(arr[left], arr[right]);
		} else {
			return right;
		}
	}

	return right;
}

void quicksort(vector<int> &arr, int left, int right) {
	// if array is of length 1, the job is done
	if (right <= left)
		return;

	// make the pivot be the first element
	int pivot = arr[(left + right) / 2];
	int i = left, j = right;

	while(i <= j) {
		while(arr[i] < pivot) ++i;
		while(arr[j] > pivot) --j;

		if (i <= j) {
			swap(arr[i], arr[j]);
			++i; --j;
		}
	}
	
	quicksort(arr, left, j);
	quicksort(arr, i, right);
}

void sort(int argc, char *argv[], int len, Timer timer) {
	vector<int> arr = get_test_array(len);

	cout << "output: "; dump_arr(arr);

	timer.start();
	quicksort(arr, 0, arr.size() - 1);
	timer.stop();

	cout << "output: "; dump_arr(arr);

	int order = check_array_order(arr);
	if (order == 0)
		cout << "CORRECT! output is ordered" << endl;
	else
		cout << "WRONG! output array is not ordered, failed at " << order <<  endl;
}
