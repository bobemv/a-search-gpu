/**********************************************************************
Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#define SUCCESS 0
#define FAILURE 1

using namespace std;

/* convert the kernel file into a string */
int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if(f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size+1];
		if(!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout<<"Error: failed to open file\n:"<<filename<<endl;
	return FAILURE;
}

void mostrarArray(int* numeros, int lengtharray) {
	char space = 32;

	int i;

	for (i = 0; i < lengtharray; i++) {
		cout << numeros[i] << space;
	}
	cout << "\n";
}

void mostrarArray(long* numeros, int lengtharray) {
	char space = 32;

	int i;

	for (i = 0; i < lengtharray; i++) {
		cout << numeros[i] << space;
	}
	cout << "\n";
}

long long sumarArrayLong(long* numeros, int lengtharray) {
	int i;
	long long res = 0;

	for (i = 0; i < lengtharray; i++) {
		res += numeros[i] ;
	}
	
	return res;
}

/*Funcion que se encargara de correr el algoritmo en CPU*/
void* funcionCPU(void** args) {
	/*Obtenemos los parametros*/
	int* firstm = (int*)(args[0]);
	int* secondm = (int*)(args[1]);
	int firstlength = firstm[0];
	int secondlength = secondm[0];


	int multiplier = 1;
	int i;
	/*Pasamos el primer multiplo a numero normal long para facilitar la multiplicacion*/
	long first = 0;
	for (i = firstlength; i > 0; i--) {
		first += firstm[i] * multiplier;
		multiplier *= 10;
	}
	cout << first << endl;
	multiplier = 1;

	/*Multiplicacion de numeros por el metodo normal. O(N^2)*/
	long long res = 0;
	for (i = secondlength; i > 0; i--) {
		res += first * secondm[i] * multiplier;
		multiplier *= 10;
	}
	cout << res << endl;
	return &res;
}

int main_main(int argc, char* argv[])
{
	/*Variables de medicion*/
	clock_t start;
	double tiempo;

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
	if(numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);	
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		cout << "Choose CPU as default device." << endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);	
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	}
	

	/*Step 3: Create context.*/
	cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
	
	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	/*Step 5: Create program object */
	const char *filename = "HelloWorld_Kernel.cl";
	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
	
	/*Step 6: Build program. */
	status=clBuildProgram(program, 1,devices,NULL,NULL,NULL);

	/*Step 7: Initial input,output for the host and create memory objects for the kernel*/
	const int firstmultiple [] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	const int secondmultiple [] = { 10, 11, 12, 13, 14, 15, 16, 17, 18 };
	int firstlength = sizeof(firstmultiple) / sizeof(int);
	int secondlength = sizeof(secondmultiple) / sizeof(int);
	
	int* input1 = (int*)malloc((firstlength+1) * sizeof(int));
	int* input2 = (int*)malloc((secondlength+1) * sizeof(int));
	memcpy(input1, &firstlength, sizeof(int)); /*Almacenamos las longitudes de los arrays al principio*/
	memcpy(input2, &secondlength, sizeof(int));
	memcpy(input1+1, firstmultiple, firstlength*sizeof(int));
	memcpy(input2+1, secondmultiple, secondlength*sizeof(int));
	cout << "Multiplo 1: ";
	mostrarArray(input1, firstlength+1);
	cout << "Multiplo 2: ";
	mostrarArray(input2, secondlength+1);

	long *output = (long*)malloc(firstlength*sizeof(long));

	cl_mem input1Buffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, (firstlength + 1) * sizeof(int),(void *) input1, NULL);
	cl_mem input2Buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (secondlength+ 1) * sizeof(int), (void *)input2, NULL);
	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY , firstlength * sizeof(long), NULL, NULL);

	/*Step 8: Create kernel object */
	cl_kernel kernel = clCreateKernel(program,"helloworld", NULL);

	/*Step 9: Sets Kernel arguments.*/
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&input1Buffer);
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&input2Buffer);
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&outputBuffer);
	
	/*Tiempo en CPU del algoritmo*/
	int numargs = 2;
	void** argsCPU = (void**)malloc(numargs * sizeof(void*));
	long long* res;
	argsCPU[0] = input1;
	argsCPU[1] = input2;


	start = clock();
	res = (long long*) funcionCPU(argsCPU);
	cout << "Tiempo tardado CPU: " << ((float)(clock() - start)) / CLOCKS_PER_SEC << endl;
	cout << "Resultado: " << *res << endl;

	/*Step 10: Running the kernel.*/
	size_t global_work_size[1] = {firstlength};

	start = clock();
	status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
	

	/*Step 11: Read the cout put back to host memory.*/
	status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, sizeof(long)*firstlength, output, 0, NULL, NULL);
	cout << "Tiempo tardado GPU: " << ((float)(clock() - start)) / CLOCKS_PER_SEC << endl;
	cout << "Array resultado: ";
	mostrarArray(output, firstlength );
	cout << "Resultado: " << sumarArrayLong(output, firstlength) << endl;


	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(kernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseMemObject(input1Buffer);		//Release mem object.
	status = clReleaseMemObject(input2Buffer);		//Release mem object.
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.

	if (output != NULL)
	{
		free(output);
		output = NULL;
	}

	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}

	std::cout<<"Passed!\n";
	return SUCCESS;
}

int main_step1(int argc, char* argv[]) {
	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	cout << "Numero de Plataformas: " << numPlatforms << endl;

	/*For clarity, choose the first available platform. */
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		cl_int i = 0;
		/*Mostramos la informacion de cada plataforma*/
		cout << "Informacion plataformas:" << endl;
		cout << endl;
		const int tamInfo = 100;/*100 para ir sobre seguro*/
		char info[tamInfo]; 
		for (i = 0; i < numPlatforms; i++) {
			cout << "Plataforma id: " << platforms[i] << endl;
			clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE ,tamInfo, info, NULL);
			cout << "" << info << endl;
			memset(info, 0, sizeof(info));

			clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION , tamInfo, info, NULL);
			cout << "" << info << endl;
			memset(info, 0, sizeof(info));

			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, tamInfo, info, NULL);
			cout << "" << info << endl;
			memset(info, 0, sizeof(info));

			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, tamInfo, info, NULL);
			cout << "" << info << endl;
			memset(info, 0, sizeof(info));

			clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, tamInfo, info, NULL);
			cout << "" << info << endl;
			memset(info, 0, sizeof(info));

			cout << endl;
		}

		platform = platforms[0];
		free(platforms);
	}

	
}
int main_step2(int argc, char* argv[])
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
		status= clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_GLOBAL_MEM_SIZE: " << *((cl_ulong*)inf) << " bytes" << endl;
		cout << "CL_DEVICE_GLOBAL_MEM_SIZE: " << (*((cl_ulong*)inf))/(1024.0*1024.0*1024.0) << " Gbytes" << endl;
		
		status = clGetDeviceInfo(devices[i], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_LOCAL_MEM_SIZE: " << *((cl_ulong*)inf) << " bytes" << endl;
		cout << "CL_DEVICE_LOCAL_MEM_SIZE: " << (*((cl_ulong*)inf)) / (1024.0*1024.0*1024.0) << " Gbytes" << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_CLOCK_FREQUENCY: " << *((cl_ulong*)inf) << " MHz" << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_COMPUTE_UNITS: " << *((cl_ulong*)inf)  << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_PARAMETER_SIZE: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_WORK_GROUP_SIZE: " << *((cl_ulong*)inf) << endl;

		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_ulong), inf, NULL);
		cout << "CL_DEVICE_MAX_WORK_ITEM_ DIMENSIONS: " << *((cl_ulong*)inf) << endl;

		

		cout << endl;
	}
}

