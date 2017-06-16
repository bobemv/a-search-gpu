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
			tCPU[j] = (*clSearch).time_CPU_inside_instances_search_A_star(20);


			cout << "GPU" << endl;
			//tGPU[j] = (*clSearch).time_CPU_inside_instances_search_A_star(100);
			tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*CPU - GPU_inside*/
int Tests::test_A(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_Inside.cl";
	Search_AStar *clSearch;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_A/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);


	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}


	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_search_A_star();
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel , "searchastar", opencl);
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU_inside" << endl;
			tGPU[j] = (*clSearch).time_GPU_inside_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*CPU - GPU_v1*/
int Tests::test_B1(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_v1.cl";
	Search_AStar *clSearch;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_B1/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);


	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_search_A_star();
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU_v1" << endl;
			tGPU[j] = (*clSearch).time_GPU_v1_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*CPU - GPU_v2*/
int Tests::test_B2(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_v2.cl";
	Search_AStar *clSearch;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_B2/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);


	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}


	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_search_A_star();
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU_v2" << endl;
			tGPU[j] = (*clSearch).time_GPU_v2_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*CPU - GPU_v3*/
int Tests::test_B3(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_v3.cl";
	Search_AStar *clSearch;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_B3/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}


	myfile << "SPARSE CPU_MED GPU_MED CPU_AVG GPU_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	for (i = 0; i < reps; i++) {
		clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
		if (debug) cout << "GPU (WARMUP)" << endl;
		(*clSearch).time_GPU_v3_search_A_star();
	}
	
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();


			cout << "GPU_v3" << endl;
			tGPU[j] = (*clSearch).time_GPU_v3_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*GPU_v1 - GPU_v2 - GPU_v3*/
int Tests::test_C(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl_v1;
	OCLW opencl_v2;
	OCLW opencl_v3;
	char *fileKernel_v1 = "Kernel_v1.cl";
	char *fileKernel_v2 = "Kernel_v2.cl";
	char *fileKernel_v3 = "Kernel_v3.cl";
	Search_AStar *clSearch_v1;
	Search_AStar *clSearch_v2;
	Search_AStar *clSearch_v3;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_C/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	/*Creating context, command queue and program for our kernel V1*/
	status = opencl_v1.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v1.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_v1.GPU_program(fileKernel_v1);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v1.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	/*Creating context, command queue and program for our kernel V1*/
	status = opencl_v2.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v2.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_v2.GPU_program(fileKernel_v2);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v2.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	/*Creating context, command queue and program for our kernel V1*/
	status = opencl_v3.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v3.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_v3.GPU_program(fileKernel_v3);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v3.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	myfile << "SPARSE GPU_V1_MED GPU_V2_MED GPU_V3_MED GPU_V1_AVG GPU_V2_AVG GPU_V3_AVG\n";

	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch_v1 = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_v1, "searchastar", opencl_v1);
	clSearch_v2 = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_v2, "searchastar", opencl_v2);
	clSearch_v3 = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_v3, "searchastar", opencl_v3);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch_v1).time_GPU_v1_search_A_star();
	(*clSearch_v2).time_GPU_v2_search_A_star();
	(*clSearch_v3).time_GPU_v3_search_A_star();
	delete clSearch_v1;
	delete clSearch_v2;
	delete clSearch_v3;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch_v1 = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel_v1, "searchastar", opencl_v1);
		clSearch_v2 = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel_v2, "searchastar", opencl_v2);
		clSearch_v3 = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel_v3, "searchastar", opencl_v3);
		double* tGPUv1 = (double*)malloc(reps * sizeof(double));
		double* tGPUv2 = (double*)malloc(reps * sizeof(double));
		double* tGPUv3 = (double*)malloc(reps * sizeof(double));


		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "GPUv1" << endl;
			tGPUv1[j] = (*clSearch_v1).time_GPU_v1_search_A_star();

			cout << "GPUv2" << endl;
			tGPUv2[j] = (*clSearch_v2).time_GPU_v2_search_A_star();

			cout << "GPUv3" << endl;
			tGPUv3[j] = (*clSearch_v3).time_GPU_v3_search_A_star();
			//tCPU[j] = tGPU[j];
			
			(*clSearch_v1).random_start_end(j);
			(*clSearch_v2).set_start_end((*clSearch_v1).get_start(), (*clSearch_v1).get_end());
			(*clSearch_v3).set_start_end((*clSearch_v1).get_start(), (*clSearch_v1).get_end());

		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tGPUv1, reps)) + " ";
		myfile << to_string(median(tGPUv2, reps)) + " ";
		myfile << to_string(median(tGPUv3, reps)) + " ";
		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tGPUv1, reps)) + " ";
		myfile << to_string(average(tGPUv2, reps)) + " ";
		myfile << to_string(average(tGPUv3, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tGPUv1);
		tGPUv1 = NULL;
		free(tGPUv2);
		tGPUv2 = NULL;
		free(tGPUv3);
		tGPUv3 = NULL;

		delete clSearch_v1;
		delete clSearch_v2;
		delete clSearch_v3;
	}

	myfile.close();
	return 0;
}

/*GPU_inside - GPU_inside_parallel*/
int Tests::test_D(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl_inside;
	OCLW opencl_inside_parallel;
	char *fileKernel_inside = "Kernel_Inside.cl";
	char *fileKernel_inside_parallel = "Kernel_Inside_Parallel.cl";
	Search_AStar *clSearch_inside;
	Search_AStar *clSearch_inside_parallel;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_D/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);


	/*Creating context, command queue and program for our kernel*/
	status = opencl_inside.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_inside.GPU_program(fileKernel_inside);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	status = opencl_inside_parallel.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside_parallel.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_inside_parallel.GPU_program(fileKernel_inside_parallel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside_parallel.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}


	myfile << "SPARSE GPU_INSIDE_MED GPU_INSIDE_PARALLEL_MED GPU_INSIDE_AVG GPU_INSIDE_PARALLEL_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch_inside = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_inside, "searchastar", opencl_inside);
	clSearch_inside_parallel = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_inside_parallel, "searchastar", opencl_inside_parallel);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch_inside).time_GPU_inside_search_A_star();
	(*clSearch_inside_parallel).time_GPU_inside_parallel_search_A_star();
	delete clSearch_inside;
	delete clSearch_inside_parallel;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch_inside = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel_inside, "searchastar", opencl_inside);
		clSearch_inside_parallel = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel_inside, "searchastar", opencl_inside_parallel);
		double* tGPU_inside = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside_parallel = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "GPU_inside" << endl;
			tGPU_inside[j] = (*clSearch_inside).time_GPU_inside_search_A_star();


			cout << "GPU_inside_parallel" << endl;
			tGPU_inside_parallel[j] = (*clSearch_inside_parallel).time_GPU_inside_parallel_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch_inside).random_start_end(j);
			(*clSearch_inside_parallel).set_start_end((*clSearch_inside).get_start(), (*clSearch_inside).get_end());
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tGPU_inside, reps)) + " ";
		myfile << to_string(median(tGPU_inside_parallel, reps)) + " ";
		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tGPU_inside, reps)) + " ";
		myfile << to_string(average(tGPU_inside_parallel, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tGPU_inside);
		tGPU_inside = NULL;
		free(tGPU_inside_parallel);
		tGPU_inside_parallel = NULL;

		delete clSearch_inside;
		delete clSearch_inside_parallel;
	}

	myfile.close();
	return 0;
}

/*GPU_v1 - GPU_v2 - GPU_v3 - GPU_inside - GPU_inside_parallel - NOT UPDATED YET*/
int Tests::test_E(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_E/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	myfile << "SPARSE GPU_V1_MED GPU_V2_MED GPU_V3_MED GPU_INSIDE_MED GPU_INSIDE_PARALLEL_MED GPU_V1_AVG GPU_V2_AVG GPU_V3_AVG GPU_INSIDE_AVG GPU_INSIDE_PARALLEL_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_v1_search_A_star();
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double* tGPUv1 = (double*)malloc(reps * sizeof(double));
		double* tGPUv2 = (double*)malloc(reps * sizeof(double));
		double* tGPUv3 = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside_parallel = (double*)malloc(reps * sizeof(double));



		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "GPUv1" << endl;
			tGPUv1[j] = (*clSearch).time_GPU_v1_search_A_star();

			cout << "GPUv2" << endl;
			tGPUv2[j] = (*clSearch).time_GPU_v2_search_A_star();

			cout << "GPUv3" << endl;
			tGPUv3[j] = (*clSearch).time_GPU_v3_search_A_star();

			cout << "GPU_inside" << endl;
			tGPU_inside[j] = (*clSearch).time_GPU_inside_search_A_star();

			cout << "GPU_inside_parallel" << endl;
			tGPU_inside_parallel[j] = (*clSearch).time_GPU_inside_parallel_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tGPUv1, reps)) + " ";
		myfile << to_string(median(tGPUv2, reps)) + " ";
		myfile << to_string(median(tGPUv3, reps)) + " ";
		myfile << to_string(median(tGPU_inside, reps)) + " ";
		myfile << to_string(median(tGPU_inside_parallel, reps)) + " ";

		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tGPUv1, reps)) + " ";
		myfile << to_string(average(tGPUv2, reps)) + " ";
		myfile << to_string(average(tGPUv3, reps)) + "";
		myfile << to_string(average(tGPU_inside, reps)) + " ";
		myfile << to_string(average(tGPU_inside_parallel, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tGPUv1);
		tGPUv1 = NULL;
		free(tGPUv2);
		tGPUv2 = NULL;
		free(tGPUv3);
		tGPUv3 = NULL;
		free(tGPU_inside);
		tGPU_inside = NULL;
		free(tGPU_inside_parallel);
		tGPU_inside_parallel = NULL;

		delete clSearch;
	}

	myfile.close();
	return 0;
}

/*CPU_inside_instances - GPU_inside_instances*/
int Tests::test_F(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, int numInstances, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_Inside_Instances.cl";

	int i = 0, j = 0;

	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_F/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	myfile << "SPARSE CPU_INSTANCES_MED GPU_INSIDE_INSTANCES_MED CPU_INSTANCES_AVG GPU_INSIDE_INSTANCES_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_inside_instances_search_A_star(5);
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPU = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU_instances" << endl;
			tCPU[j] = (*clSearch).time_CPU_instances_search_A_star(numInstances);


			cout << "GPU_inside_instances" << endl;
			tGPU[j] = (*clSearch).time_GPU_inside_instances_search_A_star(numInstances);
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
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

/*CPU GPU_v1 - GPU_v2 - GPU_v3 - GPU_inside - GPU_inside_parallel - NOT UPDATED YET*/
int Tests::test_all(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_all/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	myfile << "SPARSE GPU_V1_MED GPU_V2_MED GPU_V3_MED GPU_INSIDE_MED GPU_INSIDE_PARALLEL_MED GPU_V1_AVG GPU_V2_AVG GPU_V3_AVG GPU_INSIDE_AVG GPU_INSIDE_PARALLEL_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch).time_GPU_v1_search_A_star();
	delete clSearch;

	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		Search_AStar *clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, "HelloWorld_Kernel.cl", "searchastar");
		double* tCPU = (double*)malloc(reps * sizeof(double));
		double* tGPUv1 = (double*)malloc(reps * sizeof(double));
		double* tGPUv2 = (double*)malloc(reps * sizeof(double));
		double* tGPUv3 = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside_parallel = (double*)malloc(reps * sizeof(double));



		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "CPU" << endl;
			tCPU[j] = (*clSearch).time_CPU_search_A_star();

			cout << "GPUv1" << endl;
			tGPUv1[j] = (*clSearch).time_GPU_v1_search_A_star();

			cout << "GPUv2" << endl;
			tGPUv2[j] = (*clSearch).time_GPU_v2_search_A_star();

			cout << "GPUv3" << endl;
			tGPUv3[j] = (*clSearch).time_GPU_v3_search_A_star();

			cout << "GPU_inside" << endl;
			tGPU_inside[j] = (*clSearch).time_GPU_inside_search_A_star();

			cout << "GPU_inside_parallel" << endl;
			tGPU_inside_parallel[j] = (*clSearch).time_GPU_inside_parallel_search_A_star();
			//tCPU[j] = tGPU[j];

			(*clSearch).random_start_end(j);
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tCPU, reps)) + " ";
		myfile << to_string(median(tGPUv1, reps)) + " ";
		myfile << to_string(median(tGPUv2, reps)) + " ";
		myfile << to_string(median(tGPUv3, reps)) + " ";
		myfile << to_string(median(tGPU_inside, reps)) + " ";
		myfile << to_string(median(tGPU_inside_parallel, reps)) + " ";

		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tCPU, reps)) + " ";
		myfile << to_string(average(tGPUv1, reps)) + " ";
		myfile << to_string(average(tGPUv2, reps)) + " ";
		myfile << to_string(average(tGPUv3, reps)) + "";
		myfile << to_string(average(tGPU_inside, reps)) + " ";
		myfile << to_string(average(tGPU_inside_parallel, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tCPU);
		tCPU = NULL;
		free(tGPUv1);
		tGPUv1 = NULL;
		free(tGPUv2);
		tGPUv2 = NULL;
		free(tGPUv3);
		tGPUv3 = NULL;
		free(tGPU_inside);
		tGPU_inside = NULL;
		free(tGPU_inside_parallel);
		tGPU_inside_parallel = NULL;

		delete clSearch;
	}

	myfile.close();
	return 0;
}

/*GPU_v3 - GPU_inside_parallel*/
int Tests::test_G(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl_v3;
	OCLW opencl_inside_parallel;
	char *fileKernel_v3 = "Kernel_v3.cl";
	char *fileKernel_inside_parallel = "Kernel_Inside_Parallel.cl";
	Search_AStar *clSearch_v3;
	Search_AStar *clSearch_inside_parallel;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_G/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);



	/*Creating context, command queue and program for our kernel*/
	status = opencl_v3.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v3.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_v3.GPU_program(fileKernel_v3);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_v3.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	status = opencl_inside_parallel.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside_parallel.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl_inside_parallel.GPU_program(fileKernel_inside_parallel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl_inside_parallel.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}

	myfile << "SPARSE GPU_V3_MED GPU_INSIDE_PARALLEL_MED GPU_V3_AVG GPU_INSIDE_PARALLEL_AVG\n";
	if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
	clSearch_v3 = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_v3, "searchastar", opencl_v3);
	clSearch_inside_parallel = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_inside_parallel, "searchastar", opencl_inside_parallel);
	if (debug) cout << "GPU (WARMUP)" << endl;
	(*clSearch_v3).time_GPU_v3_search_A_star();
	(*clSearch_inside_parallel).time_GPU_inside_parallel_search_A_star();
	delete clSearch_v3;
	delete clSearch_inside_parallel;
	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch_v3 = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_v3, "searchastar", opencl_v3);
		clSearch_inside_parallel = new Search_AStar(nnodos, sparsefactor[0], maxcoste, maxdistance, ini, fin, fileKernel_inside_parallel, "searchastar", opencl_inside_parallel);
		double* tGPU_v3 = (double*)malloc(reps * sizeof(double));
		double* tGPU_inside_parallel = (double*)malloc(reps * sizeof(double));

		myfile << to_string(sparsefactor[i]) + " ";
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			cout << "GPU_v3" << endl;
			tGPU_v3[j] = (*clSearch_v3).time_GPU_v3_search_A_star();


			cout << "GPU_inside_parallel" << endl;
			tGPU_inside_parallel[j] = (*clSearch_inside_parallel).time_GPU_inside_parallel_search_A_star();
			//tCPU[j] = tGPU[j];

			//(*clSearch).random_start_end(j);
			(*clSearch_v3).random_start_end(j);
			(*clSearch_inside_parallel).set_start_end((*clSearch_v3).get_start(), (*clSearch_v3).get_end());
		}

		//if (debug) cout << "Writing file " << endl;
		//if (debug) cout << "Medians " << endl;
		myfile << to_string(median(tGPU_v3, reps)) + " ";
		myfile << to_string(median(tGPU_inside_parallel, reps)) + " ";
		//if (debug) cout << "Averages " << endl;
		myfile << to_string(average(tGPU_v3, reps)) + " ";
		myfile << to_string(average(tGPU_inside_parallel, reps)) + "\n";
		//if (debug) cout << "Finished Writing file " << endl;

		free(tGPU_v3);
		tGPU_v3 = NULL;
		free(tGPU_inside_parallel);
		tGPU_inside_parallel = NULL;

		delete clSearch_v3;
		delete clSearch_inside_parallel;
	}

	myfile.close();
	return 0;
}


/*Return data about path lengths*/
int Tests::test_lengthPaths(cl_ulong nnodos, int reps, cl_float sparsefactor[], int numElems, bool debug)
{
	/*--- VARIABLES ---*/
	cl_ulong ini = 0;
	cl_ulong fin = nnodos - 1;
	cl_uint maxdistance = 300;
	cl_uint maxcoste = 50;
	cl_int status;
	OCLW opencl;
	char *fileKernel = "Kernel_v3.cl";
	Search_AStar *clSearch;

	int i = 0, j = 0;
	/*--- END ---*/
	ofstream myfile;
	unsigned long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	string filename = "graphs/test_lengthPaths/" + to_string(startTime) + "standard_sparsefactor-nnodos" + to_string(nnodos) + ".txt";
	myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}
	status = opencl.GPU_program(fileKernel);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return FAILURE;
	}


	myfile << "SPARSE LENGTHPATH\n";


	for (i = 0; i < numElems; i++) {
		if (debug) cout << "Let's instantiate a Search_AStar object with nnodos = " << nnodos << " and sparsefactor = " << sparsefactor[i] << endl;
		clSearch = new Search_AStar(nnodos, sparsefactor[i], maxcoste, maxdistance, ini, fin, fileKernel, "searchastar", opencl);

		
		//clSearch.debug_print_connections();

		for (j = 0; j < reps; j++) {
			myfile << to_string(sparsefactor[i]) + " ";
			cout << "GPU_v3" << endl;
			(*clSearch).time_GPU_v3_search_A_star();

			myfile << to_string((*clSearch).get_length_path()) + "\n";
			(*clSearch).random_start_end(j);

		}


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