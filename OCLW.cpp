#include"OCLW.h"

int OCLW::convertToString(const char *filename, string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
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
	cout << "Error: failed to open file\n:" << filename << endl;
	return FAILURE;
}

cl_int OCLW::GPU_setup() {

	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	cl_uint	numDevices = 0;


	if (status != CL_SUCCESS) return status;

	/*For clarity, choose the first available platform. */
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if (status != CL_SUCCESS) return status;
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/

	if (OCLW::flagDeviceType == CPU) {
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
	}
	else if (OCLW::flagDeviceType == GPU) {
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	}
	
	if (status != CL_SUCCESS) return status;
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		return FAILURE;
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		if (OCLW::flagDeviceType == CPU) {
			status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
		}
		else if (OCLW::flagDeviceType == GPU) {
			status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
		}
		if (status != CL_SUCCESS) return status;
	}


	/*Step 3: Create context.*/
	context = clCreateContext(NULL, 1, devices, NULL, NULL, &status);
	if (status != CL_SUCCESS) return status;

	/*Step 4: Creating command queue associate with the context.*/
	commandQueue = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status);
	if (status != CL_SUCCESS) return status;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_program(char *filename) {
	/*Step 5: Create program object */
	string sourceStr;
	cl_int status;
	char *options = (char*)calloc(100, sizeof(char));

	strcpy(options, "-cl-opt-disable -Werror");
	//options = NULL;

	status = convertToString(filename, sourceStr);
	if (status == FAILURE) {
		return FAILURE;
	}
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	program = clCreateProgramWithSource(context, 1, &source, sourceSize, &status);
	if (status != CL_SUCCESS) return status;

	/*Step 6: Build program. */
	status = clBuildProgram(program, 1, devices, options, NULL, NULL);
	if (status != CL_SUCCESS) {
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char *log = (char *)malloc(log_size * sizeof(char));

		// Get the log
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		cout << "log: " << endl;
		cout << log << endl;

		return status;
	}

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer(size_t sizeBuffer, void *data, cl_mem_flags flags) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL){
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, flags, sizeBuffer, data, &status);

	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_input(size_t sizeBuffer, void *data) {
	cl_int status;
	cl_mem *mem_aux = NULL;


	if (sizeBuffer < 1 || data == NULL) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeBuffer, data, &status);

	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_output(size_t sizeBuffer) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeBuffer, NULL, &status);
	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_input_output(size_t sizeBuffer, void *data) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeBuffer, data, &status);
	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_input_empty(size_t sizeBuffer) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeBuffer, NULL, &status);

	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_output_empty(size_t sizeBuffer) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeBuffer, NULL, &status);

	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_input_output_empty(size_t sizeBuffer) {
	cl_int status;
	cl_mem *mem_aux = NULL;

	if (sizeBuffer < 1) {
		return FAILURE;
	}

	mem_aux = (cl_mem*)realloc(buffers, (nbuffers + 1) * sizeof(cl_mem));
	if (mem_aux == NULL) {
		return FAILURE;
	}
	buffers = mem_aux;
	mem_aux = NULL;


	buffers[nbuffers] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeBuffer, NULL, &status);

	if (status != CL_SUCCESS) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return status;
		}
		buffers = mem_aux;
		mem_aux = NULL;
		return status;
	}

	nbuffers++;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_update(cl_uint posBuffer, size_t sizeToWrite, void *data) {
	cl_int status;

	if (posBuffer >= nbuffers) {
		return FAILURE;
	}

	status = clEnqueueWriteBuffer(commandQueue, buffers[posBuffer], true, 0, sizeToWrite, data, 0, NULL, NULL);

	return status;
}


cl_int OCLW::GPU_kernel(char *fun) {
	cl_uint i; 
	cl_int status;


	kernel = clCreateKernel(program, fun, &status);
	if (status != CL_SUCCESS) return status;

	narguments = 0;
	return CL_SUCCESS;
}


cl_int OCLW::GPU_argument(size_t sizeVariable, void* variable, cl_uint position) {
	cl_int status;

	status = clSetKernelArg(kernel, position, sizeVariable, variable);
	if (status != CL_SUCCESS) return status;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_argument_buffers() {
	cl_int status;
	cl_int i;

	for (i = 0; i < nbuffers; i++) {
		status = clSetKernelArg(kernel, i, sizeof(cl_mem), (void *)&(buffers[i]));
		if (status != CL_SUCCESS) return status;
	}

	return CL_SUCCESS;
}

cl_int OCLW::GPU_work_sizes_optimal(cl_ulong threadsWanted) {
	char inf[100];
	cl_int status;
	int i;
	cl_int multiplier = 1;
	workDim = 1;
	globalWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));
	localWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxComputeUnits = *((cl_ulong*)inf);


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxWorkGroupSize = *((cl_ulong*)inf);

	/*Assigning proper values to localworksizes and globalworksizes.*/
	status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]), NULL);
	if (status != CL_SUCCESS) return status;
	
	
	cout << "maxComputeUnits: " << maxComputeUnits << endl;
	cout << "maxWorkGroupSize: " << maxWorkGroupSize << endl;
	cout << "CL_KERNEL_WORK_GROUP_SIZE: " << localWorkSize[0] << endl;
	globalWorkSize[0] = ((threadsWanted/localWorkSize[0])+1)*localWorkSize[0];
	


	totalSize = 0;
	for (i = 0; i < workDim; i++) {
		totalSize += globalWorkSize[i];
	}
	for (i = 0; i < workDim; i++) {
		totalSize *= localWorkSize[i];
	}

	return CL_SUCCESS;
}

cl_int OCLW::GPU_work_sizes_optimal_allcores(cl_ulong threadsWanted) {
	char inf[100];
	cl_int status;
	int i;
	cl_int multiplier = 1;
	workDim = 1;
	globalWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));
	localWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxComputeUnits = *((cl_ulong*)inf);


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxWorkGroupSize = *((cl_ulong*)inf);

	/*Assigning proper values to localworksizes and globalworksizes.*/
	status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]), NULL);
	if (status != CL_SUCCESS) return status;

	cl_ulong workItemsperLocalWorkGroup = (threadsWanted / maxComputeUnits);
	if (workItemsperLocalWorkGroup == 0) {
		workItemsperLocalWorkGroup++;
	}

	if (workItemsperLocalWorkGroup > CL_DEVICE_MAX_WORK_GROUP_SIZE) {
		localWorkSize[0] = CL_DEVICE_MAX_WORK_GROUP_SIZE;
	}

	globalWorkSize[0] = localWorkSize[0] * maxComputeUnits;

	totalSize = 0;
	for (i = 0; i < workDim; i++) {
		totalSize += globalWorkSize[i];
	}
	for (i = 0; i < workDim; i++) {
		totalSize *= localWorkSize[i];
	}

	return CL_SUCCESS;
}

cl_int OCLW::GPU_work_sizes_optimal_onecore() {
	char inf[100];
	cl_int status;
	int i;
	cl_int multiplier = 1;
	workDim = 1;
	globalWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));
	localWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxComputeUnits = *((cl_ulong*)inf);


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) return status;

	maxWorkGroupSize = *((cl_ulong*)inf);

	/*Assigning proper values to localworksizes and globalworksizes.*/
	status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]), NULL);
	if (status != CL_SUCCESS) return status;


	localWorkSize[0] = maxWorkGroupSize;
	

	globalWorkSize[0] = localWorkSize[0];

	totalSize = 0;
	for (i = 0; i < workDim; i++) {
		totalSize += globalWorkSize[i];
	}
	for (i = 0; i < workDim; i++) {
		totalSize *= localWorkSize[i];
	}

	return CL_SUCCESS;
}


cl_int OCLW::GPU_work_sizes(cl_uint workDim, size_t *globalWorkSize, size_t *localWorkSize) {
	this->workDim = workDim;
	this->globalWorkSize = globalWorkSize;
	this->localWorkSize = localWorkSize;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_run() {
	cl_int status;

	status = clEnqueueNDRangeKernel(commandQueue, kernel, workDim, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (status != CL_SUCCESS) return status;
	/*We wait for it to finish*/
	status = clFinish(commandQueue);
	if (status != CL_SUCCESS) return status;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_buffer_read_host(cl_uint numBuffer, size_t sizeToRead, void *ptr) {
	cl_int status;

	if (numBuffer >= nbuffers) {
		return FAILURE;
	}

	status = clEnqueueReadBuffer(commandQueue, buffers[numBuffer], CL_TRUE, 0, sizeToRead, ptr, 0, NULL, NULL);
	if (status != CL_SUCCESS) return status;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_clear_kernel() {
	cl_int status;
	cl_uint i = 0;

	status = clReleaseKernel(kernel);
	if (status != CL_SUCCESS) return status;

	for (i = 0; i < nbuffers; i++) {
		status = clReleaseMemObject(buffers[i]);
		if (status != CL_SUCCESS) return status;
	}
	nbuffers = 0;

	return CL_SUCCESS;
}

cl_int OCLW::GPU_clear() {
	cl_int status;
	cl_uint i = 0;

	if (kernel != NULL) {
		status = clReleaseKernel(kernel);
		if (status != CL_SUCCESS) return status;
		kernel = NULL;
	}

	for (i = 0; i < nbuffers; i++) {
		status = clReleaseMemObject(buffers[i]);
		if (status != CL_SUCCESS) return status;
	}
	nbuffers = 0;

	if (program != NULL) {
		status = clReleaseProgram(program);
		if (status != CL_SUCCESS) return status;
		program = NULL;
	}

	if (commandQueue != NULL) {
		status = clReleaseCommandQueue(commandQueue);
		if (status != CL_SUCCESS) return status;
		commandQueue = NULL;
	}

	if (context != NULL) {
		status = clReleaseContext(context);
		if (status != CL_SUCCESS) return status;
		context = NULL;
	}

	return CL_SUCCESS;

}

cl_int OCLW::GPU_clear_buffer(cl_uint numBuffer) {
	cl_int status;
	cl_mem* mem_aux = NULL;

	if (numBuffer >= nbuffers) {
		return FAILURE;
	}
	status = clReleaseMemObject(buffers[numBuffer]);
	if (status != CL_SUCCESS) return status;

	nbuffers--;

	if (nbuffers > 0) {
		mem_aux = (cl_mem*)realloc(buffers, nbuffers  * sizeof(cl_mem));
		if (mem_aux == NULL) {
			return FAILURE;
		}
		buffers = mem_aux;
		mem_aux = NULL;
	}

	return CL_SUCCESS;
}

cl_int OCLW::GPU_set_device_type(cl_int flag) {
	OCLW::flagDeviceType = OCLW::GPU;
	if (flag == CL_DEVICE_TYPE_GPU) {
		OCLW::flagDeviceType = OCLW::GPU;
	}
	else if (flag == CL_DEVICE_TYPE_CPU) {
		OCLW::flagDeviceType = OCLW::CPU;
	}

	return CL_SUCCESS;
}

OCLW::DeviceType OCLW::GPU_get_device_type() {
	return OCLW::flagDeviceType;
}

void OCLW::debug_GPU_errors(cl_int status) {
	float version = 2.0;
	if (status == CL_SUCCESS) {
		cout << "CL_SUCCESS" << endl;
	}
	else if (status == CL_INVALID_PROGRAM_EXECUTABLE) {
		cout << "Error: CL_INVALID_PROGRAM_EXECUTABLE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is no successfully built program executable available for device associated with command_queue. " << endl;
	}
	else if (status == CL_INVALID_COMMAND_QUEUE) {
		cout << "Error: CL_INVALID_COMMAND_QUEUE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] command_queue is not a valid host command-queue.  " << endl;
	}
	else if (status == CL_INVALID_KERNEL) {
		cout << "Error: CL_INVALID_KERNEL" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] kernel is not a valid kernel object. " << endl;
	}
	else if (status == CL_INVALID_CONTEXT) {
		cout << "Error: CL_INVALID_CONTEXT" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same. " << endl;
	}
	else if (status == CL_INVALID_KERNEL_ARGS) {
		cout << "Error: CL_INVALID_KERNEL_ARGS" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the kernel argument values have not been specified or if a kernel argument declared to be a pointer to a type does not point to a named address space. " << endl;
	}
	else if (status == CL_INVALID_WORK_DIMENSION) {
		cout << "Error: CL_INVALID_WORK_DIMENSION" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] work_dim is not a valid value (i.e. a value between 1 and 3). " << endl;
	}
	else if (status == CL_INVALID_GLOBAL_WORK_SIZE) {
		cout << "Error: CL_INVALID_GLOBAL_WORK_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...]  global_work_size is NULL, or if any of the values specified in global_work_size[0], ...global_work_size [work_dim - 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued. " << endl;
	}
	else if (status == CL_INVALID_GLOBAL_OFFSET) {
		cout << "Error: CL_INVALID_GLOBAL_OFFSET" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the sizeof(size_t) for the device on which the kernel execution will be enqueued. " << endl;
	}
	else if (status == CL_INVALID_WORK_GROUP_SIZE) {
		cout << "Error: CL_INVALID_WORK_GROUP_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[1] - [...] local_work_size is specified and does not match the work-group size for kernel in the program source given by the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier. " << endl;
		cout << "[2] - [...] local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] * … local_work_size[work_dim – 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo. " << endl;
		cout << "[3] - [...] local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source. " << endl;
	}
	else if (status == CL_INVALID_WORK_ITEM_SIZE) {
		cout << "Error: CL_INVALID_WORK_ITEM_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1]. " << endl;
	}
	else if (status == CL_INVALID_WORK_GROUP_SIZE) {
		cout << "Error: CL_INVALID_WORK_GROUP_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the program was compiled with –cl-uniform-work-group-size and the number of work-items specified by global_work_size is not evenly divisible by size of work-group given by local_work_size. " << endl;
	}
	else if (status == CL_INVALID_WORK_ITEM_SIZE) {
		cout << "Error: CL_INVALID_WORK_ITEM_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim – 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], ... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1]. " << endl;
	}
	else if (status == CL_MISALIGNED_SUB_BUFFER_OFFSET) {
		cout << "Error: CL_MISALIGNED_SUB_BUFFER_OFFSET" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue. " << endl;
	}
	else if (status == CL_INVALID_IMAGE_SIZE) {
		cout << "Error: CL_INVALID_IMAGE_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue. " << endl;
	}
	else if (status == CL_IMAGE_FORMAT_NOT_SUPPORTED) {
		cout << "Error: CL_IMAGE_FORMAT_NOT_SUPPORTED" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...]  an image object is specified as an argument value and the image format (image channel order and data type) is not supported by device associated with queue. " << endl;
	}
	else if (status == CL_OUT_OF_RESOURCES) {
		cout << "Error: CL_OUT_OF_RESOURCES" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[1] - [...]  if there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device. " << endl;
		cout << "[2] - [...]  there is a failure to allocate resources required by the OpenCL implementation on the device. " << endl;
	}
	else if (status == CL_MEM_OBJECT_ALLOCATION_FAILURE) {
		cout << "Error: CL_MEM_OBJECT_ALLOCATION_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel. " << endl;
	}
	else if (status == CL_INVALID_EVENT_WAIT_LIST) {
		cout << "Error: CL_INVALID_EVENT_WAIT_LIST " << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events. " << endl;
	}
	else if (status == CL_INVALID_OPERATION) {
		cout << "Error: CL_INVALID_OPERATION " << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] SVM pointers are passed as arguments to a kernel and the device does not support SVM or if system pointers are passed as arguments to a kernel and/or stored inside SVM allocations passed as kernel arguments and the device does not support fine grain system SVM allocations. " << endl;
	}
	else if (status == CL_OUT_OF_HOST_MEMORY) {
		cout << "Error: CL_OUT_OF_HOST_MEMORY" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to allocate resources required by the OpenCL implementation on the host. " << endl;
	}

	/*The info shown in the errors below has been obtained from the following source:
	https://streamcomputing.eu/blog/2013-04-28/opencl-error-codes/ */

	else if (status == CL_DEVICE_NOT_FOUND) {
		cout << "Error: CL_DEVICE_NOT_FOUND" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] no OpenCL devices that matched device_type were found. " << endl;
	}
	else if (status == CL_DEVICE_NOT_AVAILABLE) {
		cout << "Error: CL_DEVICE_NOT_AVAILABLE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...]  a device in devices is currently not available even though the device was returned by clGetDeviceIDs. " << endl;
	}
	else if (status == CL_COMPILER_NOT_AVAILABLE) {
		cout << "Error: CL_COMPILER_NOT_AVAILABLE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] program is created with clCreateProgramWithSource and a compiler is not available i.e. CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device Queries for clGetDeviceInfo is set to CL_FALSE." << endl;
	}
	else if (status == CL_MEM_OBJECT_ALLOCATION_FAILURE) {
		cout << "Error: CL_MEM_OBJECT_ALLOCATION_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to allocate memory for buffer object. " << endl;
	}
	else if (status == CL_PROFILING_INFO_NOT_AVAILABLE) {
		cout << "Error: CL_PROFILING_INFO_NOT_AVAILABLE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue, if the execution status of the command identified by event is not CL_COMPLETE or if event is a user event object. " << endl;
	}
	else if (status == CL_MEM_COPY_OVERLAP) {
		cout << "Error: CL_MEM_COPY_OVERLAP" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] src_buffer and dst_buffer are the same buffer or subbuffer object and the source and destination regions overlap or if src_buffer and dst_buffer are different sub-buffers of the same associated buffer object and they overlap. The regions overlap if src_offset ≤ to dst_offset ≤ to src_offset + size – 1, or if dst_offset ≤ to src_offset ≤ to dst_offset + size – 1. " << endl;
	}
	else if (status == CL_MEM_OBJECT_ALLOCATION_FAILURE) {
		cout << "Error: CL_MEM_OBJECT_ALLOCATION_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to allocate memory for buffer object. " << endl;
	}
	else if (status == CL_IMAGE_FORMAT_MISMATCH) {
		cout << "Error: CL_IMAGE_FORMAT_MISMATCH" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] src_image and dst_image do not use the same image format. " << endl;
	}
	else if (status == CL_IMAGE_FORMAT_NOT_SUPPORTED) {
		cout << "Error: CL_IMAGE_FORMAT_NOT_SUPPORTED" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the image_format is not supported." << endl;
	}
	else if (status == CL_BUILD_PROGRAM_FAILURE) {
		cout << "Error: CL_BUILD_PROGRAM_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to build the program executable. This error will be returned if clBuildProgram does not return until the build has completed." << endl;
	}
	else if (status == CL_MAP_FAILURE) {
		cout << "Error: CL_MAP_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to map the requested region into the host address space. This error cannot occur for image objects created with CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR." << endl;
	}
	else if (status == CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) {
		cout << "Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the execution status of any of the events in event_list is a negative integer value." << endl;
	}
	else if (status == CL_COMPILE_PROGRAM_FAILURE) {
		cout << "Error: CL_COMPILE_PROGRAM_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] there is a failure to compile the program source. This error will be returned if clCompileProgram does not return until the compile has completed." << endl;
	}
	else if (status == CL_LINKER_NOT_AVAILABLE) {
		cout << "Error: CL_LINKER_NOT_AVAILABLE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] a linker is not available i.e. CL_DEVICE_LINKER_AVAILABLE specified in the table of allowed values for param_name for clGetDeviceInfo is set to CL_FALSE." << endl;
	}
	else if (status == CL_LINK_PROGRAM_FAILURE) {
		cout << "Error: CL_LINK_PROGRAM_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...]there is a failure to link the compiled binaries and/or libraries." << endl;
	}
	else if (status == CL_DEVICE_PARTITION_FAILED) {
		cout << "Error: CL_DEVICE_PARTITION_FAILED" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the partition name is supported by the implementation but in_device could not be further partitioned." << endl;
	}
	else if (status == CL_KERNEL_ARG_INFO_NOT_AVAILABLE) {
		cout << "Error: CL_LINK_PROGRAM_FAILURE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the argument information is not available for kernel." << endl;
	}
	else if (status == CL_INVALID_VALUE) {
		cout << "Error: CL_INVALID_VALUE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] This depends on the function: two or more coupled parameters had errors." << endl;
	}
	else if (status == CL_INVALID_DEVICE_TYPE) {
		cout << "Error: CL_INVALID_DEVICE_TYPE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] an invalid device_type is given" << endl;
	}
	else if (status == CL_INVALID_PLATFORM) {
		cout << "Error: CL_INVALID_PLATFORM" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] an invalid platform was given" << endl;
	}
	else if (status == CL_INVALID_DEVICE) {
		cout << "Error: CL_INVALID_DEVICE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] devices contains an invalid device or are not associated with the specified platform." << endl;
	}
	else if (status == CL_INVALID_QUEUE_PROPERTIES) {
		cout << "Error: CL_INVALID_QUEUE_PROPERTIES" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] specified command-queue-properties are valid but are not supported by the device." << endl;
	}
	else if (status == CL_INVALID_HOST_PTR) {
		cout << "Error: CL_INVALID_HOST_PTR" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] This flag is valid only if host_ptr is not NULL. If specified, it indicates that the application wants the OpenCL implementation to allocate memory for the memory object and copy the data from memory referenced by host_ptr.CL_MEM_COPY_HOST_PTR and CL_MEM_USE_HOST_PTR are mutually exclusive.CL_MEM_COPY_HOST_PTR can be used with CL_MEM_ALLOC_HOST_PTR to initialize the contents of the cl_mem object allocated using host-accessible (e.g. PCIe) memory." << endl;
	}
	else if (status == CL_INVALID_MEM_OBJECT) {
		cout << "Error: CL_INVALID_MEM_OBJECT" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] memobj is not a valid OpenCL memory object." << endl;
	}
	else if (status == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) {
		cout << "Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the OpenGL/DirectX texture internal format does not map to a supported OpenCL image format." << endl;
	}
	else if (status == CL_INVALID_SAMPLER) {
		cout << "Error: CL_INVALID_SAMPLER" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] sampler is not a valid sampler object." << endl;
	}
	else if (status == CL_INVALID_SAMPLER) {
		cout << "Error: CL_INVALID_SAMPLER" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] sampler is not a valid sampler object." << endl;
	}
	else if (status == CL_INVALID_BINARY) {
		cout << "Error: CL_INVALID_BINARY" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] The provided binary is unfit for the selected device. if program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded." << endl;
	}
	else if (status == CL_INVALID_BUILD_OPTIONS) {
		cout << "Error: CL_INVALID_BUILD_OPTIONS" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the build options specified by options are invalid." << endl;
	}
	else if (status == CL_INVALID_PROGRAM) {
		cout << "Error: CL_INVALID_PROGRAM" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] program is a not a valid program object." << endl;
	}
	else if (status == CL_INVALID_KERNEL_NAME) {
		cout << "Error: CL_INVALID_KERNEL_NAME" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] kernel_name is not found in program." << endl;
	}
	else if (status == CL_INVALID_KERNEL_NAME) {
		cout << "Error: CL_INVALID_KERNEL_NAME" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] kernel_name is not found in program." << endl;
	}
	else if (status == CL_INVALID_KERNEL_DEFINITION) {
		cout << "Error: CL_INVALID_KERNEL_DEFINITION" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built." << endl;
	}
	else if (status == CL_INVALID_ARG_INDEX) {
		cout << "Error: CL_INVALID_ARG_INDEX" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] arg_index is not a valid argument index." << endl;
	}
	else if (status == CL_INVALID_ARG_VALUE) {
		cout << "Error: CL_INVALID_ARG_VALUE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] arg_value specified is not a valid value." << endl;
	}
	else if (status == CL_INVALID_ARG_SIZE) {
		cout << "Error: CL_INVALID_ARG_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size != sizeof(cl_sampler)." << endl;
	}
	else if (status == CL_INVALID_EVENT) {
		cout << "Error: CL_INVALID_EVENT" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] event objects specified in event_list are not valid event objects." << endl;
	}
	else if (status == CL_INVALID_GL_OBJECT) {
		cout << "Error: CL_INVALID_GL_OBJECT" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] texture is not a GL texture object whose type matches texture_target, if the specified miplevel of texture is not defined, or if the width or height of the specified miplevel is zero." << endl;
	}
	else if (status == CL_INVALID_BUFFER_SIZE) {
		cout << "Error: CL_INVALID_BUFFER_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] size is 0.Implementations may return CL_INVALID_BUFFER_SIZE if size is greater than the CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in the table of allowed values for param_name for clGetDeviceInfo for all devices in context." << endl;
	}
	else if (status == CL_INVALID_MIP_LEVEL) {
		cout << "Error: CL_INVALID_MIP_LEVEL" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] miplevel is greater than zero and the OpenGL implementation does not support creating from non-zero mipmap levels." << endl;
	}
	else if (status == CL_INVALID_PROPERTY) {
		cout << "Error: CL_INVALID_PROPERTY" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] Vague error, depends on the function" << endl;
	}
	else if (status == CL_INVALID_IMAGE_DESCRIPTOR) {
		cout << "Error: CL_INVALID_IMAGE_DESCRIPTOR" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] values specified in image_desc are not valid or if image_desc is NULL." << endl;
	}
	else if (status == CL_INVALID_COMPILER_OPTIONS) {
		cout << "Error: CL_INVALID_COMPILER_OPTIONS" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the compiler options specified by options are invalid." << endl;
	}
	else if (status == CL_INVALID_LINKER_OPTIONS) {
		cout << "Error: CL_INVALID_LINKER_OPTIONS" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the linker options specified by options are invalid." << endl;
	}
	else if (status == CL_INVALID_DEVICE_PARTITION_COUNT) {
		cout << "Error: CL_INVALID_DEVICE_PARTITION_COUNT" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] the partition name specified in properties is CL_DEVICE_PARTITION_BY_COUNTS and the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_SUB_DEVICES or the total number of compute units requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device, or the number of compute units requested for one or more sub-devices is less than zero or the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device." << endl;
	}
	else if (status == CL_INVALID_PIPE_SIZE) {
		cout << "Error: CL_INVALID_PIPE_SIZE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] pipe_packet_size is 0 or the pipe_packet_size exceeds CL_DEVICE_PIPE_MAX_PACKET_SIZE value for all devices in context or if pipe_max_packets is 0." << endl;
	}
	else if (status == CL_INVALID_DEVICE_QUEUE) {
		cout << "Error: CL_INVALID_DEVICE_QUEUE" << endl;
		cout << "From the OpenCL " << version << " Reference Pages:" << endl;
		cout << "[...] when an argument is of type queue_t when it’s not a valid device queue object." << endl;
	}
	else {
		cout << "Error: undefined." << endl;
	}
} 