#pragma once

//OCLW stands for OpenCL Wrappings

#include<CL/cl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<iostream>
#include<fstream>

#define FAILURE 1

using namespace std;

class OCLW
{
public:
	char *filename;
	char *fun;
	cl_device_id *devices;
	cl_context context;
	cl_command_queue commandQueue;
	cl_program program;
	cl_mem *buffers;
	cl_uint nbuffers;
	cl_kernel kernel;
	cl_uint narguments;
	cl_int* output = NULL;


	cl_ulong maxComputeUnits;
	cl_ulong maxWorkGroupSize;
	cl_uint workDim;
	size_t* globalWorkSize = NULL;
	size_t* localWorkSize = NULL;
	cl_ulong totalSize;

	static enum DeviceType { CPU = 1, GPU = 2 };
	DeviceType flagDeviceType = GPU;

	/*----- CONSTRUCTOR ------- */
	OCLW() {
		nbuffers = 0;
		buffers = NULL;
	}

	/*----- MAIN FUNCTIONS ------- */
	cl_int GPU_setup();
	cl_int GPU_program(char *filename);
	cl_int GPU_buffer(size_t sizeBuffer, void *data, cl_mem_flags flags);
	cl_int GPU_buffer_input(size_t sizeBuffer, void *data);
	cl_int GPU_buffer_output(size_t sizeBuffer);
	cl_int GPU_buffer_input_output(size_t sizeBuffer, void *data);
	cl_int GPU_buffer_update(cl_uint posBuffer, size_t sizeToWrite, void *data);
	cl_int GPU_buffer_input_empty(size_t sizeBuffer);
	cl_int GPU_buffer_output_empty(size_t sizeBuffer);
	cl_int GPU_buffer_input_output_empty(size_t sizeBuffer);
	cl_int GPU_kernel(char *fun);
	cl_int GPU_argument(size_t sizeVariable, void* variable, cl_uint position);
	cl_int GPU_argument_buffers();
	cl_int GPU_work_sizes_optimal(cl_ulong threadsWanted);
	cl_int GPU_work_sizes(cl_uint workDim, size_t *globalWorkSize, size_t *localWorkSize);
	cl_int GPU_work_sizes_optimal_allcores(cl_ulong threadsWanted);
	cl_int GPU_work_sizes_optimal_onecore();
	cl_int GPU_run();
	cl_int GPU_run_several(int numInstances);
	cl_int GPU_buffer_read_host(cl_uint numBuffer, size_t sizeToRead, void *ptr);
	cl_int GPU_clear_kernel();
	cl_int GPU_clear();
	cl_int GPU_clear_buffer(cl_uint numBuffer);

	cl_int GPU_set_device_type(cl_int flag);
	DeviceType GPU_get_device_type();

	/*----- UTILITY ------- */
	int convertToString(const char *filename, string& s);

	/*----- ERRORS------- */
	void debug_GPU_errors(cl_int status);

};