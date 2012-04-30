/**
 * \file common.cpp
 */

#include "common.h"
#include <time.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string>
#include <sstream>
using namespace std;

vector<int> def_arr;

void Timer::start() {
	clock_gettime(DEF_CLOCK, &t_start);
}

void Timer::stop() {
	clock_gettime(DEF_CLOCK, &t_end);
}

long Timer::get() {
	return (t_end.tv_sec - t_start.tv_sec) * 1e9 + (t_end.tv_nsec - t_start.tv_nsec);
}

void dump_arr(vector<int> arr) {
	cout << "[" << arr[0];
	for(unsigned int i = 1; i < arr.size(); ++i)
		cout << ", " << arr[i];
	cout << "]" << endl;
}

string arr_str(vector<int> arr) {
	stringstream ss;
	ss << "[" << arr[0];
	for(unsigned int i = 1; i < arr.size(); ++i)
		ss << ", " << arr[i];
	ss << "]";

	return ss.str();
}

int check_array_order(vector<int> arr) {
	for(unsigned int i = 1; i < arr.size(); ++i)
		if (arr[i-1] > arr[i])
			return i;
	return 0;
}

vector<int> get_test_array(int n) {
	srand(time(NULL));
	while(n-- > 0) {
		def_arr.push_back(rand() % 100000);
	}
	/*def_arr.push_back(1);
	def_arr.push_back(2);
	def_arr.push_back(4);
	def_arr.push_back(8);
	def_arr.push_back(16);
	def_arr.push_back(32);
	def_arr.push_back(64);
	def_arr.push_back(128);*/
	//for(int i = 6; i >= 0; --i)
	//	def_arr.push_back(1 << i);


	return def_arr;
}
