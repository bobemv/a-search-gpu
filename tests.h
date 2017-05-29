#pragma once
#include "OCLW.h"

class Tests
{
public:
	Tests() {

	}
	int standard_single(cl_ulong nnodos, cl_float sparsefactor, bool debug);
	int standard_sparsefactor_warmups(cl_ulong nnodos, bool debug);
	int standard_sparsefactor(cl_ulong nnodos, int reps, bool debug);
	int grid_single(cl_ulong dim, cl_float blockfactor, bool debug);
	int grid_blockfactor(cl_ulong dim, int reps, bool debug);
	double median(double *elems, int numElems);
	double median_and_sort(double **elems, int numElems);
	double average(double *elems, int numElems);
};

