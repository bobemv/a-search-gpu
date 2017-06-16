#include "tests.h"

int main(int argc, char* argv[]) {

	Tests tests;
	cl_float sparsefactor[] = { 0.001, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6 };
	int numInstances = 500;
	int reps = 50;
	int nsparse = sizeof(sparsefactor) / sizeof(cl_float);
	//tests.standard_single(200, 0.8, true);
	//tests.grid_single(50, 0.65, false);

	
	cl_ulong nnodosarray[] = {50, 100, 200};
	int numElems = sizeof(nnodosarray) / sizeof(cl_ulong);

	for (int i = 0; i < numElems; i++) {
		cout << i << endl;
		tests.test_A(nnodosarray[i], reps, sparsefactor, nsparse, true);
	}

	cl_ulong nnodosarray1[] = { 50, 100, 200, 500 };
	int numElems1 = sizeof(nnodosarray1) / sizeof(cl_ulong);

	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_B1(nnodosarray1[i], reps, sparsefactor, nsparse, true);
	}

	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_B3(nnodosarray1[i], reps, sparsefactor, nsparse, true);
	}

	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_B3(nnodosarray1[i], reps, sparsefactor, nsparse, true);
	}

	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_C(nnodosarray1[i], reps, sparsefactor, nsparse, true);
	}

	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_G(nnodosarray1[i], reps, sparsefactor, nsparse, true);
	}


	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_D(nnodosarray[i], reps, sparsefactor, nsparse, true);
	}

	reps = 20;
	for (int i = 0; i < numElems1; i++) {
		cout << i << endl;
		tests.test_F(nnodosarray[i], reps, sparsefactor, nsparse, numInstances, true);
	}

	/*
	cl_ulong nnodosarray[] = { 10, 20, 40, 100, 200, 400};
	int numElems = sizeof(nnodosarray) / sizeof(cl_ulong);

	for (int i = 0; i < numElems; i++) {
		cout << i << endl;
		tests.grid_blockfactor(nnodosarray[i], 1, false);
	}
	*/
	
	//tests.standard_sparsefactor(400, 5, false);



	return 0;

}

int main_in(int argc, char* argv[])
{
	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	/*For clarity, choose the first available platform. */
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

	devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

	/*Mostramos la informacion de cada plataforma*/
	cout << "Informacion devices:" << endl;
	cout << endl;
	const int tamInfo = 100;/*100 para ir sobre seguro*/
	char info[tamInfo];
	void *inf;
	cl_int i = 0;
	for (i = 0; i < numDevices; i++) {
		cout << "Devices id: " << devices[i] << endl;
		clGetDeviceInfo(devices[i], CL_DEVICE_PROFILE, tamInfo, info, NULL);
		cout << "" << info << endl;
		memset(info, 0, sizeof(info));

		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, tamInfo, info, NULL);
		cout << "" << info << endl;
		memset(info, 0, sizeof(info));

		clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, tamInfo, info, NULL);
		cout << "" << info << endl;
		memset(info, 0, sizeof(info));

		inf = (void*)malloc(sizeof(cl_ulong));
		status = clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_GLOBAL_MEM_SIZE: " << *((cl_ulong*)inf) << " bytes" << endl;
		cout << "CL_DEVICE_GLOBAL_MEM_SIZE: " << (*((cl_ulong*)inf)) / (1024.0*1024.0*1024.0) << " Gbytes" << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_LOCAL_MEM_SIZE: " << *((cl_ulong*)inf) << " bytes" << endl;
		cout << "CL_DEVICE_LOCAL_MEM_SIZE: " << (*((cl_ulong*)inf)) / (1024.0*1024.0*1024.0) << " Gbytes" << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_CLOCK_FREQUENCY: " << *((cl_ulong*)inf) << " MHz" << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_COMPUTE_UNITS: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_PARAMETER_SIZE: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_WORK_GROUP_SIZE: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_WORK_ITEM_SIZES: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_WORK_ITEM_ DIMENSIONS: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_GLOBAL_MEM_SIZE DIMENSIONS: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_MEM_ALLOC_SIZE DIMENSIONS: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), inf, NULL);
		cout << "CL_DEVICE_MAX_PARAMETER_SIZE DIMENSIONS: " << *((size_t*)inf) << endl;



		cout << endl;
	}
}

