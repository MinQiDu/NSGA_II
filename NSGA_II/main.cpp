#include "nsgaii.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	int run, func_id, mnfes, dim, pop_size;
	double CR, MR;

	run = atoi(argv[1]);
	func_id = atoi(argv[2]);
	mnfes = atoi(argv[3]);
	dim = atoi(argv[4]);
	pop_size = atoi(argv[5]);
	CR = atof(argv[6]);
	MR = atof(argv[7]);

	NSGAII nsgaii;
	nsgaii.RunALG(run, func_id, mnfes, dim, pop_size, CR, MR);
	return 0;
}