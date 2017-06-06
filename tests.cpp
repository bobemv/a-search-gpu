#include "tests.h"
#include "Search_AStar.h"
#include <fstream>
#include <string>

using namespace std;

int Tests::standard_single(cl_ulong nnodos, cl_float sparsefactor, bool debug) {
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	/*--- END ---*/

	if(debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor << endl;
	Search_AStar clSearch (nnodos, sparsefactor, maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");

	//clSearch.debug_print_connections();

	if (debug) cout << "CPU" << endl;
	clSearch.time_CPU_search_A_star();

	if (debug) cout << "GPU (WARMUP)" << endl;
	clSearch.time_GPU_v1_search_A_star();

	if (debug) cout << "GPU" << endl;
	clSearch.time_GPU_v1_search_A_star();

	return 0;
}

int Tests::standard_sparsefactor_warmups(cl_ulong nnodos, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_float sparsefactor[] = {0.0001, 0.001, 0.01, 0.02, 0.04, 0.08, 0.1, 0.2, 0.4, 0.8, 1};
	int numElems = sizeof(sparsefactor) / sizeof(cl_float);
	int i = 0;
	/*--- END ---*/
	ofstream myfile;
	string filename = "graphs/test2_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar clSearch (nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double t = 0.0;

		myfile << to_string(sparsefactor[i])+" ";
		//clSearch.debug_print_connections();

		if (debug) cout << "CPU" << endl;
		t = clSearch.time_CPU_search_A_star();
		myfile << to_string(t) + " ";

		if (debug) cout << "GPU (WARMUP 1)" << endl;
		t = clSearch.time_GPU_v1_search_A_star();
		myfile << to_string(t) + " ";

		if (debug) cout << "GPU (WARMUP 2)" << endl;
		t = clSearch.time_GPU_v1_search_A_star();
		myfile << to_string(t) + " ";

		if (debug) cout << "GPU (WARMUP 3)" << endl;
		t = clSearch.time_GPU_v1_search_A_star();
		myfile << to_string(t) + " ";

		if (debug) cout << "GPU" << endl;
		t = clSearch.time_GPU_v1_search_A_star();
		myfile << to_string(t) + "\n";
	}

	myfile.close();
	return 0;
}

int Tests::standard_sparsefactor(cl_ulong nnodos, int reps, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_float sparsefactor[] = { 0.001, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };
	int numElems = sizeof(sparsefactor) / sizeof(cl_float);
	int i = 0, j= 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/"+ to_string(startTime) +"standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	Search_AStar *clSearch = new Search_AStar (nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_instances_search_A_star(20);
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU" << endl;
			tGPU[j] = (*clSearch).time_GPU_inside_parallel_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end();
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tCPU, reps)) + " ";
		myfile << to_string(median(tGPU, reps)) + " ";
		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tCPU, reps)) + " ";
		myfile << to_string(average(tGPU, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;
		
		free(tCPU);
		tCPU = NULL;
		free(tGPU);
		tGPU = NULL;
		
		delete clSearch;
	}

	myfile.close();
	return 0;
}


int Tests::test_CPU_GPU_v1(cl_ulong nnodos, int reps, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_float sparsefactor[] = { 0.001, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };
	int numElems = sizeof(sparsefactor) / sizeof(cl_float);
	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_instances_search_A_star(20);
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU" << endl;
			tGPU[j] = (*clSearch).time_GPU_v1_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end();
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tCPU, reps)) + " ";
		myfile << to_string(median(tGPU, reps)) + " ";
		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tCPU, reps)) + " ";
		myfile << to_string(average(tGPU, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tCPU);
		tCPU = NULL;
		free(tGPU);
		tGPU = NULL;

		delete clSearch;
	}

	myfile.close();
	return 0;
}


int Tests::grid_single(cl_ulong dim, cl_float blockfactor, bool debug) {
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = (dim*dim) - 1;
	/*--- END ---*/

	if (debug) cout << "Let's instantiate a Search_AStar object with dim = " << dim << " and blockfactor = " << blockfactor << endl;
	Search_AStar clSearch (dim, blockfactor, ini, fin, "HelloWorld_Kernel.cl", "searchastar");

	//clSearch.debug_print_connections();

	if (debug) cout << "CPU" << endl;
	clSearch.time_CPU_search_A_star();

	if (debug) cout << "GPU (WARMUP)" << endl;
	clSearch.time_GPU_v1_search_A_star();

	if (debug) cout << "GPU" << endl;
	clSearch.time_GPU_v1_search_A_star();

	return 0;
}
int Tests::grid_blockfactor(cl_ulong dim, int reps, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = dim+1;
	cl_ulong fin = dim*(dim-1) - 2;
	cl_ulong nnodos = dim * dim;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_float blockfactor[] = { 0.1, 0.4, 0.6, 0.8, 0.85, 0.9, 0.95, 1 };
	int numElems = sizeof(blockfactor) / sizeof(cl_float);
	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/" + to_string(startTime) + "grid_blockfactor-dim" + to_string(dim) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	myfile << "SPARSE CPU GPU\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with dim = " << dim << " and blockfactor = " << blockfactor << endl;
	Search_AStar clSearch (dim, blockfactor[0], ini, fin, "HelloWorld_Kernel.cl", "searchastar");
	if (debug) cout << "GPU (WARMUP)" << endl;
	clSearch.time_GPU_v1_search_A_star();

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << blockfactor[i] << endl;
		Search_AStar clSearch (dim, blockfactor[i], ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(blockfactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			if (debug) cout << "CPU" << endl;
			tCPU[i] = clSearch.time_CPU_search_A_star();


			if (debug) cout << "GPU" << endl;
			tGPU[i] = clSearch.time_GPU_v1_search_A_star();
		}

		//TODO should use median instead
		myfile << to_string(median(tCPU, reps)) + " ";
		myfile << to_string(median(tGPU, reps)) + "\n";

		free(tCPU);
		tCPU = NULL;
		free(tGPU);
		tGPU = NULL;
	}

	myfile.close();
	return 0;
}




/* AUXILIARES */
double Tests::median_and_sort(double **elems, int numElems) {
	int i, j;
	double res;

	for (i = 0; i < (numElems - 1); i++) {
		for (j = 0; j < (numElems-1); j++) {
			if (*(elems[j + 1]) < *(elems[j])) {
				double aux = *(elems[j]);
				*(elems[j]) = *(elems[j + 1]);
				*(elems[j + 1]) = aux;
			}
		}
	}
	if (numElems % 2) {
		res = *(elems[numElems / 2]);
	}
	else {
		res = (*(elems[numElems / 2]) + *(elems[(numElems / 2) - 1])) / 2;
	}

	return res;
}

double Tests::median(double *elems, int numElems) {
	int i, j;
	double res = 0.0;

	for (i = 0; i < (numElems - 1); i++) {
		for (j = 0; j < (numElems - 1); j++) {
			if (elems[j + 1] < elems[j]) {
				double aux = elems[j];
				elems[j] = elems[j + 1];
				elems[j + 1] = aux;
			}
		}
	}

	if (numElems % 2) {
		res = elems[numElems / 2];
	}
	else {
		res = (elems[numElems / 2] + elems[(numElems / 2) - 1]) / 2;
	}

	return res;
}

double Tests::average(double *elems, int numElems) {
	int i;
	double res = 0.0;

	for (i = 0; i < numElems; i++) {
		res += elems[i];
	}

	return res / numElems;
}