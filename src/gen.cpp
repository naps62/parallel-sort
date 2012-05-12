#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

int main(int argc, char **argv) {
	srand(time(NULL));

	if (argc != 2) {
		cout << "error" << endl;
		exit(-1);
	}

	for(int i = 0; i < atoi(argv[1]); ++i)
		cout << rand() << endl;
}
