#include "common.h"
#include <iostream>
using namespace std;

void timer_test() {
	cout << " --- timer test started" << endl;
	Timer timer;
	timer.start();
	sleep(2);
	timer.stop();

	cout << " --- result:    " << timer.get() << endl;
	cout << " --- excpected: " << 2000000000 << endl;
}

int main() {
	cout << "running timer test" << endl;
	timer_test();
}
