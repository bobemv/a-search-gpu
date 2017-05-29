/*Function provided from AMD inside their SDK. (helloworld sample)*/
/*Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "searchastar.h"

/*----- STANDARD SORTING AND SEARCHING------- */
int SearchAStar::swap(node** numero1, node** numero2) {
	node* aux;

	if (numero1 == NULL || numero2 == NULL) {
		return FAILURE;
	}

	aux = *numero1;
	*numero1 = *numero2;
	*numero2 = aux;

	return SUCCESS;
}
int SearchAStar::partir(node** tabla, cl_ulong ip, cl_ulong iu) {

	cl_ulong m, i;
	cl_float k;

	m = (ip + iu) / 2;
	k = tabla[m]->f;
	swap(&tabla[ip], &tabla[m]);
	m = ip;
	for (i = ip + 1; i <= iu; i++) {
		if (tabla[i]->f>k) {
			m++;
			swap(&tabla[i], &tabla[m]);
		}
	}
	swap(&tabla[ip], &tabla[m]);
	return m;
}
int SearchAStar::quicksort(node** tabla, cl_ulong ip, cl_ulong iu) {
	cl_ulong m, check = 0;

	if (ip == iu + 1) {
		return SUCCESS;
	}
	if (ip>iu) { return FAILURE; }


	if (ip == iu) {
		return SUCCESS;
	}
	else {
		m = partir(tabla, ip, iu);
		if (ip<(m - 1)) {
			check = quicksort(tabla, ip, m - 1);
			if (check == FAILURE) {
				return FAILURE;
			}
		}
		if ((m + 1)<iu) {
			check = quicksort(tabla, m + 1, iu);
			if (check == FAILURE) {
				return FAILURE;
			}
		}
	}

	return SUCCESS;

}


/*----- GENERAL GRAPHS ------- */

/*Se crea una tabla parecida a un map. Cada nodo tiene un array de ints -> Longitud | (ID, Coste) | (ID, Coste) | ...
El posicionamiento de la informacion del nodo en el array determina su ID.*/
int** SearchAStar::create_undirected_graph(int nnodos, int maxcoste, float sparsefactor) {
	int** matriz = (int**)malloc(nnodos * sizeof(int*));
	int* mem_aux = NULL;
	int i, j, n, c;
	float p;
	for (i = 0; i < nnodos; i++) {
		matriz[i] = (int*)malloc(1 * sizeof(int));
		matriz[i][0] = 0; /*Number of nodes*/
	}
	srand(time(NULL));

	for (i = 0; i < nnodos; i++) {
		for (j = i; j < nnodos; j++) {
			if (i != j) {
				c = (rand() % maxcoste) + 1;
				p = (float)rand() / (float)RAND_MAX;
				if (sparsefactor > p) {
					/*Rearranging array to store the info related to i -c-> j*/
					matriz[i][0]++;
					n = 2*matriz[i][0] + 1;
					mem_aux = (int*)realloc(matriz[i], n*sizeof(int));
					if (mem_aux == NULL) {
						/*TODO free memory management*/
						return NULL;
					}
					matriz[i] = mem_aux;
					mem_aux = NULL;

					/*Storing info*/
					matriz[i][n-2] = j;
					matriz[i][n-1] = c;

					/*Rearranging array to store the info related to j -c-> i*/
					matriz[j][0]++;
					n = 2*matriz[j][0] + 1;
					mem_aux = (int*)realloc(matriz[j], n * sizeof(int));
					if (mem_aux == NULL) {
						/*TODO free memory management*/
						return NULL;
					}
					matriz[j] = mem_aux;
					mem_aux = NULL;

					/*Storing info*/
					matriz[j][n-2] = i;
					matriz[j][n-1] = c;
				}

			}
		}
	}

	return matriz;

}

/*Returns value's position*/
/*Pseudocode used: https://en.wikipedia.org/wiki/Binary_search_algorithm*/
int SearchAStar::binary_search(int* nums, int n, int e) {
	/*1*/
	int l = 0;
	int r = n - 1;
	/*2*/
	while (l <= r) {
		/*3*/
		int m = (l + r) / 2;
		/*4*/
		if (nums[m] < e) {
			l = m + 1;
			continue;
		}
		/*5*/
		if (nums[m] > e) {
			r = m - 1;
		}
		/*6*/
		return m;
	}

	return -1;
}

int SearchAStar::binary_search_odd(int* nums, int n, int e) {
	/*1*/
	int l = 0;
	int r = n - 1;
	/*2*/
	while (l <= r) {
		/*3*/
		int m = (l + r) / 2;
		/*4*/
		if (nums[m] < e) {
			l = m + 2;
			continue;
		}
		/*5*/
		if (nums[m] > e) {
			r = m - 2;
		}
		/*6*/
		return m;
	}

	return -1;
}

/*Returns value if found.
Returns -1 if not.*/
int SearchAStar::search_cost_node_2_node(int** conexiones, int from, int to) {
	int pos = binary_search_odd(conexiones[from] + sizeof(int), conexiones[from][0] * 2, to);
	if (pos != -1) {
		return conexiones[from][pos + 2];
	}
	else {
		return -1;
	}
}

node** SearchAStar::genera_sucesores(node* nodo, int** conexiones, int nnodos) {
	int i;
	int l = conexiones[nodo->id][0];
	node** sucesores = (node**)malloc(l * sizeof(node*));

	for (i = 0; i < l; i++) {
		node* n = (node*)malloc(sizeof(node));
		n->parent = nodo;
		n->id = conexiones[nodo->id][1 + i*2];
		sucesores[l] = n;
	}

	return sucesores;
}


/*-----GRIDLIKE GRAPHS------- */
int SearchAStar::create_undirected_graph_grid() {
	cl_ulong* mem_aux = NULL;
	cl_long nodes[4] = { -1, -1, -1, -1 };
	cl_long i, j, x, y, n, counter;
	cl_float p;

	conexiones = (cl_ulong**)malloc(nnodos * sizeof(cl_ulong*));
	srand(time(NULL));

	for (i = 0; i < nnodos; i++) {
		conexiones[i] = (cl_ulong*)malloc(1 * sizeof(cl_ulong));
		p = (float)rand() / (float)RAND_MAX;
		/*Number of nodes. -1: still not generated its nodes. 0: it's a block. >0: # of nodes it can go to.*/
		if (blockfactor > p) {
			conexiones[i][0] = 0;
		}
		else {
			conexiones[i][0] = -1;
		}
	}

	for (n = 0; n < nnodos; n++) {
		
		if (conexiones[n][0] == 0)
			continue;

		y = n / dim;
		x = n % dim;
		counter = 0;

		/*Up square*/
		if (y - 1 >= 0 && conexiones[n - dim][0] != 0) {
			nodes[0] = n - dim;
			counter++;
		}

		/*Down square*/
		if (y + 1 < dim && conexiones[n + dim][0] != 0) {
			nodes[1] = n + dim;
			counter++;
		}
		/*Left square*/
		if (x - 1 >= 0 && conexiones[n - 1][0] != 0) {
			nodes[2] = n - 1;
			counter++;
		}
		/*Right square*/
		if (x + 1 < dim && conexiones[n + 1][0] != 0) {
			nodes[3] = n + 1;
			counter++;
		}

		mem_aux = (cl_ulong*)realloc(conexiones[n], (counter + 1) * sizeof(cl_ulong));
		if (mem_aux == NULL) {
			/*TODO free memory management*/
			return FAILURE;
		}
		conexiones[n] = mem_aux;
		mem_aux = NULL;
		/*Storing info*/
		for (i = 0, j = 0; i < counter; i++) {
			while (nodes[j] == -1)
				j++;
			conexiones[n][i + 1] = nodes[j];
			nodes[j] = -1;
		}
		conexiones[n][0] = counter;

	}

	return SUCCESS;

}

int SearchAStar::genera_sucesores_grid(node* nodo) {
	cl_ulong i;
	cl_ulong l = conexiones[nodo->id][0];
	nsucesores = l;
	if (l > 0) {
		sucesores = (node**)malloc(l * sizeof(node*));
		for (i = 0; i < l; i++) {
			node* n = (node*)malloc(sizeof(node));
			n->parent = nodo;
			n->id = conexiones[nodo->id][1 + i];
			sucesores[i] = n;
		}
	}

	return SUCCESS;
}

int SearchAStar::genera_sucesores_grid_GPU() {
	cl_ulong i = 0;
	cl_ulong l = 0;
	node **mem_aux = NULL;
	node *actual = NULL, *sucesor = NULL;

	nsucesores = 0;
	expand = 0;
	insert = 0;
	sucesores = NULL;

	/*We first introduce successors generated from the open list*/
	if (DEBUG) cout << "First, expanding nodes from the open list" << endl;
	while (nsucesores <= totalSize && nabiertos > 0) {
		l = conexiones[abiertos[nabiertos - 1]->id][0];
		if (l + nsucesores > totalSize) {
			break;
		}
		else {
			actual = pop_open_list();
			nsucesores += l;
			mem_aux = (node**)realloc(sucesores, nsucesores * sizeof(node*));
			if (mem_aux == NULL) {
				/*Free memory*/
				return FAILURE;
			}
			sucesores = mem_aux;
			mem_aux = NULL;
			for (i = 0; i < l; i++) {
				sucesor = (node*)malloc(sizeof(node));
				sucesor->parent = actual;
				sucesor->g = actual->g + 1.0;
				sucesor->f = sucesor->g;
				//suc->f = 0;
				sucesor->id = conexiones[actual->id][1 + i];
				sucesores[insert] = sucesor;
				insert++;
				sucesor = NULL;
			}

			/*We consider the actual node as expanded and append it to the closed nodes list. */
			if (append_closed_list(actual) == NULL) {
				return FAILURE;
			}
			actual = NULL;
			l = 0;
		}
	}


	/*If there is space left, we expand nodes already in the successors list (however it does not update f,g and h values of course).*/
	if (DEBUG) cout << "Second, if there is space left, expanding the first nodes from the successors list" << endl;
	while (nsucesores <= totalSize && expand < nsucesores) {
		actual = sucesores[expand];
		expand++;
		l = conexiones[actual->id][0];
		if (l + nsucesores > totalSize) {
			expand--;
			break;
		}
		else {
			nsucesores += l;
			mem_aux = (node**)realloc(sucesores, nsucesores * sizeof(node*));
			if (mem_aux == NULL) {
				/*Free memory*/
				return FAILURE;
			}
			sucesores = mem_aux;
			mem_aux = NULL;
			for (i = 0; i < l; i++) {
				sucesor = (node*)malloc(sizeof(node));
				sucesor->parent = actual;
				sucesor->g = actual->g + 1.0;
				sucesor->f = sucesor->g;
				//suc->f = 0;
				sucesor->id = conexiones[actual->id][1 + i];
				sucesores[insert] = sucesor;
				insert++;
				sucesor = NULL;
			}
			if (expand >= insert) {
				break;
			}

			actual = NULL;
			l = 0;
		}
	}
	
	/*sucesores = (node**)malloc(l * sizeof(node*));

	while ((nsucesores + l) <= totalSize) {
		if (l != 0) {
			mem_aux = (node**)realloc(sucesores, (nsucesores + l) * sizeof(node*));
			if (mem_aux == NULL) {
				//Free memory
				return FAILURE;
			}
			sucesores = mem_aux;
			mem_aux = NULL;
			for (i = 0; i < l; i++) {
				node* n = (node*)malloc(sizeof(node));
				n->parent = nodo;
				n->g = nodo->g + 1.0;
				n->f = n->g;
				//n->f = 0;
				n->id = conexiones[nodo->id][1 + i];
				sucesores[insert] = n;
				insert++;
			}
		}
		if (expand >= insert){
			break;
		}
		
		nodo = sucesores[expand];
		expand++;
		nsucesores += l;

		l = conexiones[nodo->id][0];
	}
	expand--;*/

	return SUCCESS;

}

cl_float SearchAStar::heuristic_grid(cl_ulong idStart, cl_ulong idEnd) {
	cl_float xStart, yStart, xEnd, yEnd;

	xStart = idStart % dim;
	yStart = idStart / dim;
	xEnd = idEnd % dim;
	yEnd = idEnd / dim;

	float leg1 = fabs(xStart - xEnd);
	float leg2 = fabs(yStart - yEnd);
	leg1 = pow(leg1, 2);
	leg2 = pow(leg2, 2);
	return sqrt(leg1 + leg2);
}

int SearchAStar::create_undirected_graph_grid_v2() {
	cl_float p;
	cl_ulong i, n;
	bool* isblock = (bool*)malloc(nnodos * sizeof(bool));


	edges = NULL;
	nedges = 0;
	edge *mem_aux = NULL;

	srand(time(NULL));

	for (i = 0; i < nnodos; i++) {
		p = (float)rand() / (float)RAND_MAX;
		blockfactor > p ? isblock[i] = true : isblock[i] = false;
	}

	for (i = 0; i < nnodos; i++) {

		if (isblock[i]) continue;

		/*Down square*/
		if(i + dim < nnodos && !isblock[i+dim]) {
			nedges++;
			mem_aux = (edge*)realloc(edges, nedges * sizeof(edge));
			if (mem_aux == NULL) {
				if (nedges > 0 && edges != NULL) {
					free(edges);
					nedges = 0;
					edges = NULL;
				}
				return FAILURE;
			}
			edges = mem_aux;
			mem_aux = NULL;

			edges[nedges - 1].start = i;
			edges[nedges - 1].start = i+dim;
		}
		/*Right square*/
		if (i + 1 < dim && !isblock[i + 1]) {
			nedges++;
			mem_aux = (edge*)realloc(edges, nedges * sizeof(edge));
			if (mem_aux == NULL) {
				if (nedges > 0 && edges != NULL) {
					free(edges);
					nedges = 0;
					edges = NULL;
				}
				return FAILURE;
			}
			edges = mem_aux;
			mem_aux = NULL;

			edges[nedges - 1].start = i;
			edges[nedges - 1].start = i + 1;
		}

	}

	return SUCCESS;

}


/*----- GPU------- */
int SearchAStar::convertToString(const char *filename, string& s)
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

int SearchAStar::setup_GPU_variables() {

	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	cl_uint	numDevices = 0;


	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*For clarity, choose the first available platform. */
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) return FAILURE;
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/

	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		return FAILURE;
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) return FAILURE;
	}


	/*Step 3: Create context.*/
	context = clCreateContext(NULL, 1, devices, NULL, NULL, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Step 4: Creating command queue associate with the context.*/
	commandQueue = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	return SUCCESS;
}

int SearchAStar::create_GPU_program() {
	/*Step 5: Create program object */
	string sourceStr;
	cl_int status;

	status = convertToString(filename, sourceStr);
	if (status == FAILURE) {
		return FAILURE;
	}
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	program = clCreateProgramWithSource(context, 1, &source, sourceSize, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Step 6: Build program. */
	status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) {
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char *log = (char *)malloc(log_size*sizeof(char));

		// Get the log
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		cout << "log: " << endl;
		cout << log << endl;

		return FAILURE;
	}

	return SUCCESS;
}

int SearchAStar::create_GPU_kernel() {
	/*Create the necessary buffers*/
	if (DEBUG) cout << "-- Creating buffers --" << endl;
	cl_ulong i;
	cl_int status;
	output = (cl_int*)malloc(nsucesores * sizeof(cl_int));
	buffers = (cl_mem*)malloc(nbuffers * sizeof(cl_mem));
	nodeMin **aux = (nodeMin**)malloc(3 * sizeof(nodeMin*));
	nodeMin *dummy = (nodeMin*)malloc(sizeof(nodeMin));
	/*TODO get rid of the following conversions...*/
	/*Converting sucesores from pointer to a pointers to pointer to structures.*/


	aux[0] = (nodeMin*)malloc(nsucesores * sizeof(nodeMin));
	for (i = 0; i < nsucesores; i++) {
		aux[0][i].id = sucesores[i]->id;
		aux[0][i].f = sucesores[i]->f;
	}
	buffers[0] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, nsucesores * sizeof(nodeMin), (void *)aux[0], &status);
	
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Converting abiertos from pointer to a pointers to pointer to structures.*/
	if (nabiertos == 0) { /*TODO This is a dummy buffer and it is a bad practice too.*/
		aux[1] = (nodeMin*)malloc(1 * sizeof(nodeMin));
		memcpy(&(aux[1][0]), dummy, sizeof(nodeMin));
		buffers[1] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(nodeMin), (void *)aux[1], &status);
	}
	else {
		aux[1] = (nodeMin*)malloc(nabiertos * sizeof(nodeMin));
		for (i = 0; i < nabiertos; i++) {
			aux[1][i].id = abiertos[i]->id;
			aux[1][i].f = abiertos[i]->f;
		}
		buffers[1] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, nabiertos * sizeof(nodeMin), (void *)aux[1], &status);
	}if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Converting sucesores from pointer to a pointers to pointer to structures.*/
	if (ncerrados == 0) { /*TODO This is a dummy buffer and it is a bad practice too.*/
		aux[2] = (nodeMin*)malloc(1 * sizeof(nodeMin));
		memcpy(&(aux[2][0]), dummy, sizeof(nodeMin));
		buffers[2] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(nodeMin), (void *)aux[2], &status);
	}
	else {
		aux[2] = (nodeMin*)malloc(ncerrados * sizeof(nodeMin));
		for (i = 0; i < ncerrados; i++) {
			aux[2][i].id = cerrados[i]->id;
			aux[2][i].f = cerrados[i]->f;
		}
		buffers[2] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, ncerrados * sizeof(nodeMin), (void *)aux[2], &status);
	}if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	buffers[3] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), (void *)&nsucesores, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	buffers[4] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), (void *)&nabiertos, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	buffers[5] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), (void *)&ncerrados, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	buffers[6] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), (void *)&fin, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	buffers[7] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_ulong), (void *)&dim, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	if (nsucesores == 0) {
		buffers[8] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 1 * sizeof(cl_int), NULL, &status);
	}
	else {
		buffers[8] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, nsucesores * sizeof(cl_int), NULL, &status);
	}

	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Create kernel*/
	if (DEBUG) cout << "-- Creating kernel. --" << endl;

	kernel = clCreateKernel(program, fun, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Setting kernel parameters. TODO change it. Dont use for's.*/
	for (i = 0; i < 3; i++) {
		status = clSetKernelArg(kernel, i, sizeof(cl_mem), (void *)&(buffers[i]));
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) return FAILURE;
	}

	status = clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void *)&nsucesores);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 4, sizeof(cl_ulong), (void *)&nabiertos);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 5, sizeof(cl_ulong), (void *)&ncerrados);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 6, sizeof(cl_ulong), (void *)&fin);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 7, sizeof(cl_ulong), (void *)&dim);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	i = 8;
	status = clSetKernelArg(kernel, i, sizeof(cl_mem), (void *)&(buffers[i]));

	if (aux != NULL) {
		if (aux[0] != NULL) free(aux[0]);
		if (aux[1] != NULL) free(aux[1]);
		if (aux[2] != NULL) free(aux[2]);
		free(aux);
	}

	return SUCCESS;
}

int SearchAStar::create_GPU_kernel_v2() {
	/*Create the necessary buffers*/
	if (DEBUG) cout << "-- Creating buffers --" << endl;
	cl_ulong i;
	cl_int status;
	nbuffers = 2;
	output2 = (nodeGPU*)malloc(nedges * sizeof(nodeGPU));
	buffers = (cl_mem*)malloc(nbuffers * sizeof(cl_mem));

	buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nedges * sizeof(edge), (void *)edges, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	buffers[1] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, nedges * sizeof(nodeGPU), NULL, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;


	/*Create kernel*/
	if (DEBUG) cout << "-- Creating kernel. --" << endl;

	kernel = clCreateKernel(program, fun, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Setting kernel parameters. TODO change it. Dont use for's.*/
	
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&(buffers[0]));
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 1, sizeof(cl_ulong), (void *)&nedges);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 2, sizeof(cl_ulong), (void *)&ini);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void *)&fin);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 4, sizeof(cl_ulong), (void *)&dim);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&(buffers[1]));
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	return SUCCESS;
}

int SearchAStar::create_GPU_kernel_v3() {
	/*Create the necessary buffers*/
	if (DEBUG) cout << "-- Creating buffers --" << endl;
	cl_ulong i;
	cl_int status;
	nbuffers = 2;
	output2 = (nodeGPU*)malloc(nedges * sizeof(nodeGPU));
	buffers = (cl_mem*)malloc(nbuffers * sizeof(cl_mem));

	buffers[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nedges * sizeof(edge), (void *)edges, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	buffers[1] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, nedges * sizeof(nodeGPU), NULL, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;


	/*Create kernel*/
	if (DEBUG) cout << "-- Creating kernel. --" << endl;

	kernel = clCreateKernel(program, fun, &status);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	/*Setting kernel parameters. TODO change it. Dont use for's.*/

	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&(buffers[0]));
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 1, sizeof(cl_ulong), (void *)&nedges);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 2, sizeof(cl_ulong), (void *)&ini);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void *)&fin);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 4, sizeof(cl_ulong), (void *)&dim);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;
	status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&(buffers[1]));
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) return FAILURE;

	return SUCCESS;
}


/*-----A* SEARCH------- */
cl_ulong* SearchAStar::get_path_A_star(node* result) {
	cl_ulong i = 0, j;
	cl_ulong* inversed = NULL;
	cl_ulong* path = NULL;
	cl_ulong* mem_aux = NULL;


	/*First we get all nodes which form the path*/
	while (result != NULL) {
		i++;
		mem_aux = (cl_ulong*)realloc(inversed, i * sizeof(cl_ulong));
		if (mem_aux == NULL) {
			return NULL;
		}
		inversed = mem_aux;
		mem_aux = NULL;
		inversed[i - 1] = result->id;
		result = result->parent;
	}

	/*Now we will reverse it and put its length as first element*/
	path = (cl_ulong*)malloc((i + 1) * sizeof(cl_ulong));
	path[0] = i;

	for (j = 1; j < i+1; j++) {
		path[j] = inversed[i-j];
	}

	return path;
}
cl_ulong* SearchAStar::search_A_star() {
	node** mem_aux = NULL;
	cl_ulong* path;
	bool found = false;
	node* actual = NULL;
	node* sucesor = NULL;
	node* inicial = (node*)malloc(sizeof(node));
	cl_ulong i, j;

	clear_search_variables();

	inicial->id = ini;
	inicial->g = 0;
	inicial->parent = NULL;

	nabiertos++;
	abiertos = (node**)malloc(nabiertos*sizeof(node*));
	abiertos[nabiertos - 1] = inicial;
	
	while (nabiertos > 0 && !found) {
		/*We obtain the node with smallest f value (located at the back of the list thanks to our quicksort).*/
		actual = abiertos[nabiertos-1];
		if(DEBUG) cout << "Actual node: " << actual->id << endl;
		nabiertos--;

		mem_aux = (node**)realloc(abiertos, nabiertos*sizeof(node*));
		if (mem_aux == NULL && nabiertos > 0) {
			cout << "[1]There has been some kind of problem with memory allocation. Not your fault!" << endl;
			clear_search_variables();
			return NULL;
		}
		abiertos = mem_aux;
		mem_aux = NULL;


		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores_grid(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}
		/* Expandimos cada sucesor*/
		i = 0;
		if (DEBUG) cout << "Expanding each successor node." << endl;
		while(i < nsucesores){

			if (found) {
				free(sucesores[i]);
				i++;
				continue;
			}
			sucesor = sucesores[i];

			if (DEBUG) cout << "Successor node: " << sucesor->id << endl;

			/*Si es el nodo final, terminamos.*/
			if (sucesor->id == fin) {
				if (DEBUG) cout << "Goal node (" << sucesor->id << ") found " << endl;
				found = true;
				i++;
				continue;
			}

			/*Calculamos, f, g y h.*/
			if (DEBUG) cout << "Computing f, g and h." << endl;
			sucesor->g = actual->g + 1;
			sucesor->h = heuristic_grid(sucesor->id, fin);
			sucesor->f = sucesor->g + sucesor->h;

			/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the open nodes list." << endl;
			bool flagSkip = false;
			j = 0;
			while (j < nabiertos) {
				if (abiertos[j]->id == sucesor->id && abiertos[j]->f <= sucesor->f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				i++;
				free(sucesor);
				continue;
			}

			/*Buscamos si hay un nodos con el mismo id en cerrados. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the closed nodes list." << endl;
			j = 0;
			while (j < ncerrados) {
				if (cerrados[j]->id == sucesor->id && cerrados[j]->f <= sucesor->f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				/*Free node*/
				free(sucesor);
			}
			else {
				if (DEBUG) cout << "Adding it to the open list." << endl;
				nabiertos++;
				mem_aux = (node**)realloc(abiertos, nabiertos * sizeof(node*));
				if (mem_aux == NULL) {
					cout << "[2]There has been some kind of problem with memory allocation. Not your fault!" << endl;
					/*TODO free memory*/
					clear_search_variables();
					return NULL;
				}
				abiertos = mem_aux;
				mem_aux = NULL;

				abiertos[nabiertos - 1] = sucesor;
			}

			i++;
			
		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;

		if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		ncerrados++;
		mem_aux = (node**)realloc(cerrados, ncerrados*sizeof(node*));
		if (mem_aux == NULL) {
			cout << "[3]There has been some kind of problem with memory allocation. Not your fault!" << endl;
			/*TODO free memory*/
			clear_search_variables();
			return NULL;
		}
		cerrados = mem_aux;
		mem_aux = NULL;

		cerrados[ncerrados - 1] = actual;

		if (DEBUG) cout << "Quicksorting the open nodes list." << endl;
		quicksort(abiertos, 0, nabiertos - 1);

	}


	if (found) {
		if (DEBUG) cout << "Retrieving the generated path." << endl;
		path = get_path_A_star(sucesor);
	}
	else {
		path = NULL;
	}
	
	/*Free memory*/
	if (DEBUG) cout << "Freeing memory and resetting variables." << endl;
	clear_search_variables();

	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}

cl_ulong* SearchAStar::search_A_star_GPU() {
	/*A* search variables*/
	node** mem_aux = NULL;
	node** aux = NULL;
	cl_ulong* path;
	bool found = false;
	node* actual = NULL;
	node* sucesor = NULL;
	node* goal = NULL;
	node* inicial = (node*)malloc(sizeof(node));
	cl_ulong i, j;
	

	inicial->id = ini;
	inicial->g = 0;
	inicial->f = heuristic_grid(ini, fin);
	inicial->parent = NULL;

	/*GPU setup variables*/

	char inf[100];
	nbuffers = 9;
	cl_int status;

	/*End variables*/

	/*Setting up GPU environment*/
	if (DEBUG) cout << "----- GPU context -----" << endl;
	if (setup_GPU_variables()) {
		cout << "There was an error setting up the GPU variables. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "----- GPU program -----" << endl;
	if (create_GPU_program()) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Getting max compute units and max work group size values." << endl;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}
	maxComputeUnits = *((cl_ulong*)inf);


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}
	maxWorkGroupSize = *((cl_ulong*)inf);


	if (DEBUG) cout << "Computing optimal sizes to global_work_size and local_work_size." << endl;
	workDim = 1;
	int numLocalWorkSizes = 1;
	globalWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));
	globalWorkSize[0] = maxComputeUnits;
	localWorkSize = (size_t*)malloc((numLocalWorkSizes) * sizeof(size_t));
	localWorkSize[0] = 1;
	totalSize = 0;
	for (i = 0; i < workDim; i++) {
		totalSize += globalWorkSize[i];
	}
	for (i = 0; i < numLocalWorkSizes; i++) {
		totalSize *= localWorkSize[i];
	}

	/*End setting up GPU*/

	if (DEBUG) cout << "Clearing searching variables before starting our algorithm in GPU." << endl;
	clear_search_variables();

	nabiertos++;
	abiertos = (node**)malloc(nabiertos * sizeof(node*));
	abiertos[nabiertos - 1] = inicial;

	while (nabiertos > 0 && !found) {
		/*Obtenemos el elemento con menor f.*/
		/*actual = pop_open_list();
		if (actual == NULL) {
			cout << "There was some kind of error. Not your fault!" << endl;
			return NULL;
		}
		if (DEBUG) cout << "----- Actual node: (" << actual->id <<","<<actual->f<< ") -----" << endl;*/

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating successors." << endl;
		nsucesores = 0;
		if (genera_sucesores_grid_GPU() == FAILURE) {
			cout << "There has been an error generating the successors. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}

		if (nsucesores == 0) {
			if(DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}

		/*We create the kernel*/
		if (create_GPU_kernel() == FAILURE) {
			cout << "There has been an error creating the kernel. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]),NULL);
		if (DEBUG) debug_GPU_errors(status);
		cl_int multiplier = 1;
		globalWorkSize[0] = maxComputeUnits;
		localWorkSize[0] = 1;

		/* Execute kernel. We look at each node (compute its h and f and look up if it has already been seen).*/
		if (DEBUG) {
			cout << "Executing kernel with the following parameters:" << endl;
			cout << "nsucesores: " << nsucesores << endl;
			for (i = 0; i < nsucesores; i++) {
				cout << "("<< sucesores[i]->id<< ",f:"<< sucesores[i]->f << ")|";
			}
			cout << endl;
			cout << "nabiertos: " << nabiertos << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "(" << abiertos[i]->id << ",f:" << abiertos[i]->f << ")|";
			}
			cout << endl;
			cout << "ncerrados: " << ncerrados << endl;
			for (i = 0; i < ncerrados; i++) {
				cout << "(" << cerrados[i]->id << ",f:" << cerrados[i]->f << ")|";
			}
			cout << endl;
		}

		status = clEnqueueNDRangeKernel(commandQueue, kernel, workDim, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if (status != CL_SUCCESS) {
			cout << "There has been an error executing the kernel. Exiting function..." << endl;
			if (DEBUG) {
				debug_GPU_errors(status);
			}
			/*TODO free memory*/
			return NULL;
		}
		/*We wait for it to finish*/
		status = clFinish(commandQueue);
		if (DEBUG) debug_GPU_errors(status);

		/*Read kernel's output*/
		if (DEBUG) cout << "Reading kernel's output." << endl;
		status = clEnqueueReadBuffer(commandQueue, buffers[nbuffers - 1], CL_TRUE, 0, sizeof(cl_int)*nsucesores, output, 0, NULL, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) {
			cout << "There has been an error reading the kernel's output. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		/*Read kernel's successors list*/
		nodeMin* aux2 = (nodeMin*)malloc(sizeof(nodeMin)*nsucesores);
		if (DEBUG) cout << "Reading kernel's updated successors list." << endl;
		status = clEnqueueReadBuffer(commandQueue, buffers[0], CL_TRUE, 0, sizeof(nodeMin)*nsucesores, aux2, 0, NULL, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) {
			cout << "There has been an error reading the kernel's output. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		for (i = 0; i < nsucesores; i++) {
			sucesores[i]->f = aux2[i].f;
		}

		if (nsucesores == 0) { /*TODO should this be in another place?*/
			/*TODO free memory*/
			break;
		}
		if (DEBUG) {
			cout << "Output de la GPU:" << endl;
			for (i = 0; i < nsucesores; i++) {
				cout << output[i] << " ";
			}
			cout << endl;
		}

		/*if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		if (append_closed_list(actual) == NULL) {
			cout << "There was some kind of error. Not your fault!" << endl;
			return NULL;
		}*/

		/*Watch which successors, already expanded, are going to be listed in the closed list.*/
		found = false;
		if (DEBUG) {
			cout << "Checking which successors are going to be included in the closed list." << endl;
			cout << "Expand index: " << expand << endl;
		}
		for (i = 0; i < expand; i++) {
			if (sucesores[i] == NULL) {
				continue;
			}
			if (DEBUG) cout << "i: " << i << " Nodo: " << sucesores[i]->id << ",f: " << sucesores[i]->f << ".Out: " << output[i] << endl;
			if (found) {
				free(sucesores[i]);
				continue;
			}

			if (output[i] == 2) {
				found = true;
				goal = sucesores[i];
			}
			else if (output[i] == 1) {
				if (append_closed_list(sucesores[i]) == NULL) {
					cout << "There was some kind of error. Not your fault!" << endl;
					return NULL;
				}
			}
			else {
				
				/*we first check if there is no conflict with the rest of the expanded nodes.*/
				bool flagAdd = true;

				for (j = i + 1; j < expand; j++) {
					if (sucesores[j] != NULL && sucesores[j]->id == sucesores[i]->id) {
						if (sucesores[j]->f <= sucesores[i]->f) {
							flagAdd = false;
							//free(sucesores[i]);
							break;
						}
						else {
							//free(sucesores[j]);
							//sucesores[j] = NULL;
						}
					}
				}
				/*then, if there is no conflict it is added*/
				if (flagAdd) {
					if (DEBUG) cout << "Added." << endl;
					if (append_closed_list(sucesores[i]) == NULL) {
						cout << "There was some kind of error. Not your fault!" << endl;
						return NULL;
					}
				}
				else {
					if (DEBUG) cout << "Not added." << endl;
				}
			}
		}
	
		/*Watch which successors, not expanded yet, are going to be listed in the open list.*/
		if (DEBUG) cout << "Checking which successors are going to be included in the open list." << endl;
		for (i = expand; i < nsucesores; i++) {
			if (sucesores[i] == NULL) {
				continue;
			}
			if (DEBUG) cout << "i: " << i << " Nodo: " << sucesores[i]->id << ",f: " << sucesores[i]->f << ".Out: " << output[i] << endl;
			if (found) {
				free(sucesores[i]);
				continue;
			}

			if (output[i] == 2) {
				found = true;
				goal = sucesores[i];
			}
			else if (output[i] == 1) {
				free(sucesores[i]);
			}
			else {
				/*we first check if there is no conflict with the rest of the nodes.*/
				bool flagAdd = true;
				for (j = i + 1; j < nsucesores; j++) {
					if (sucesores[j] != NULL && sucesores[j]->id == sucesores[i]->id) {
						if (sucesores[j]->f <= sucesores[i]->f) {
							free(sucesores[i]);
							flagAdd = false;
							break;
						}
						else {
							free(sucesores[j]);
							sucesores[j] = NULL;
						}
					}
				}

				/*then, if there is no conflict it is added*/
				if (flagAdd) {
					if (DEBUG) cout << "Added." << endl;
					if (append_open_list(sucesores[i]) == NULL) {
						cout << "There was some kind of error. Not your fault!" << endl;
						return NULL;
					}
				}
				else {
					if (DEBUG) cout << "Not added." << endl;
				}
			}
		}

		if (DEBUG) {
			cout << "We order our open list with quicksort." << endl;
			cout << "Before:" << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "("<<abiertos[i]->id <<","<<abiertos[i]->f << ")-";
			}
			cout << endl;
		}
		quicksort(abiertos, 0, nabiertos - 1);
		if (DEBUG) {
			cout << "Afterwards:" << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "(" << abiertos[i]->id << "," << abiertos[i]->f << ")-";
			}
			cout << endl;
		}

		/*Free resources allocated for the kernel*/
		if(DEBUG) cout << "Freeing resources allocated for the kernel:" << endl;
		status = clReleaseKernel(kernel);
		i = 0;
		for (i = 0; i < nbuffers; i++) {
			clReleaseMemObject(buffers[i]);
		}
		/*TODO Free memory*/
		if(DEBUG) cout << "Freeing resources allocated for the search:" << endl;
		if (sucesores != NULL) {
			free(sucesores);
		}
		nsucesores = 0;
		sucesores = NULL;
		if (buffers != NULL) {
			free(buffers);
		}
		buffers = NULL;
		if(DEBUG) cout << endl << endl;
	}


	if (found) {
		path = get_path_A_star(goal);
	}
	else {
		path = NULL;
	}

	/*Free more allocated memory*/
	clear_search_variables();

	return path;

}

cl_ulong* SearchAStar::search_A_star_GPU_v2() {
	/*A* search variables*/
	node** mem_aux = NULL;
	node** aux = NULL;
	cl_ulong* path;
	bool found = false;
	cl_ulong i, j;


	/*GPU setup variables*/

	char inf[100];
	cl_int status;

	/*End variables*/

	/*Setting up GPU environment*/
	if (DEBUG) cout << "----- GPU context -----" << endl;
	if (setup_GPU_variables()) {
		cout << "There was an error setting up the GPU variables. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "----- GPU program -----" << endl;
	if (create_GPU_program()) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	/*We create the kernel*/
	if (create_GPU_kernel_v2() == FAILURE) {
		cout << "There has been an error creating the kernel. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}
	/*
	if (DEBUG) cout << "Getting max compute units and max work group size values." << endl;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory
		return NULL;
	}
	maxComputeUnits = *((cl_ulong*)inf);
	*/

	if (DEBUG) cout << "Computing optimal sizes to global_work_size and local_work_size." << endl;
	localWorkSize = (size_t*)malloc(sizeof(size_t));
	status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]), NULL);
	localWorkSize[0] = 1;

	workDim = 1;
	int profundidad = 1;
	globalWorkSize = (size_t*)malloc(sizeof(size_t));
	globalWorkSize[0] = pow(4, profundidad);
	

	/* Execute kernel.*/
	if (DEBUG) cout << "Executing kernel." << endl;
	status = clEnqueueNDRangeKernel(commandQueue, kernel, workDim, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (status != CL_SUCCESS) {
		cout << "There has been an error executing the kernel. Exiting function..." << endl;
		if (DEBUG) {
			debug_GPU_errors(status);
		}
		/*TODO free memory*/
		return NULL;
	}
	/*We wait for it to finish*/
	status = clFinish(commandQueue);
	if (DEBUG) debug_GPU_errors(status);

	/*Read kernel's output*/
	if (DEBUG) cout << "Reading kernel's output." << endl;
	status = clEnqueueReadBuffer(commandQueue, buffers[nbuffers - 1], CL_TRUE, 0, sizeof(nodeGPU)*nedges, output2, 0, NULL, NULL);
	if (DEBUG) debug_GPU_errors(status);
	if (status != CL_SUCCESS) {
		cout << "There has been an error reading the kernel's output. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) {
		cout << "Output de la GPU:" << endl;
		for (i = 0; i < nedges; i++) {
			cout << output2[i].id << " ";
		}
		cout << endl;
	}


	/*Free resources allocated for the kernel*/
	if (DEBUG) cout << "Freeing resources allocated for the kernel:" << endl;
	status = clReleaseKernel(kernel);
	i = 0;
	for (i = 0; i < nbuffers; i++) {
		clReleaseMemObject(buffers[i]);
	}
	/*TODO Free memory*/
	if (DEBUG) cout << "Freeing resources allocated for the search:" << endl;

	if (buffers != NULL) {
		free(buffers);
	}
	buffers = NULL;

	/*
	if (found) {
		path = get_path_A_star(goal);
	}
	else {
		path = NULL;
	}
	*/
	path = NULL;

	return path;

}

cl_ulong* SearchAStar::search_A_star_GPU_v3() {
	/*A* search variables*/
	node** mem_aux = NULL;
	node** aux = NULL;
	cl_ulong* path;
	bool found = false;
	node* actual = NULL;
	node* sucesor = NULL;
	node* goal = NULL;
	node* inicial = (node*)malloc(sizeof(node));
	cl_ulong i, j;


	inicial->id = ini;
	inicial->g = 0;
	inicial->f = heuristic_grid(ini, fin);
	inicial->parent = NULL;

	/*GPU setup variables*/

	char inf[100];
	nbuffers = 9;
	cl_int status;

	/*End variables*/

	/*Setting up GPU environment*/
	if (DEBUG) cout << "----- GPU context -----" << endl;
	if (setup_GPU_variables()) {
		cout << "There was an error setting up the GPU variables. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "----- GPU program -----" << endl;
	if (create_GPU_program()) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Getting max compute units and max work group size values." << endl;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}
	maxComputeUnits = *((cl_ulong*)inf);


	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), inf, NULL);
	if (status != CL_SUCCESS) {
		cout << "There was an error creating the GPU program. Exiting function..." << endl;
		/*TODO free memory*/
		return NULL;
	}
	maxWorkGroupSize = *((cl_ulong*)inf);


	if (DEBUG) cout << "Computing optimal sizes to global_work_size and local_work_size." << endl;
	workDim = 1;
	int numLocalWorkSizes = 1;
	globalWorkSize = (size_t*)malloc((workDim) * sizeof(size_t));
	globalWorkSize[0] = maxComputeUnits;
	localWorkSize = (size_t*)malloc((numLocalWorkSizes) * sizeof(size_t));
	localWorkSize[0] = 1;
	totalSize = 0;
	for (i = 0; i < workDim; i++) {
		totalSize += globalWorkSize[i];
	}
	for (i = 0; i < numLocalWorkSizes; i++) {
		totalSize *= localWorkSize[i];
	}

	/*End setting up GPU*/

	if (DEBUG) cout << "Clearing searching variables before starting our algorithm in GPU." << endl;
	clear_search_variables();

	nabiertos++;
	abiertos = (node**)malloc(nabiertos * sizeof(node*));
	abiertos[nabiertos - 1] = inicial;

	while (nabiertos > 0 && !found) {
		/*Obtenemos el elemento con menor f.*/
		actual = pop_open_list();
		if (actual == NULL) {
		cout << "There was some kind of error. Not your fault!" << endl;
		return NULL;
		}
		if (DEBUG) cout << "----- Actual node: (" << actual->id <<","<<actual->f<< ") -----" << endl;

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores_grid(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}

		/* Expandimos cada sucesor*/
		i = 0;
		if (DEBUG) cout << "Expanding each successor node." << endl;
		while (i < nsucesores) {

			if (found) {
				free(sucesores[i]);
				i++;
				continue;
			}
			sucesor = sucesores[i];

			if (DEBUG) cout << "Successor node: " << sucesor->id << endl;

			/*Si es el nodo final, terminamos.*/
			if (sucesor->id == fin) {
				if (DEBUG) cout << "Goal node (" << sucesor->id << ") found " << endl;
				found = true;
				i++;
				continue;
			}

			/*Calculamos, f, g y h.*/
			if (DEBUG) cout << "Computing f, g and h." << endl;
			sucesor->g = actual->g + 1;
			sucesor->h = heuristic_grid(sucesor->id, fin);
			sucesor->f = sucesor->g + sucesor->h;

			/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the open nodes list." << endl;
			bool flagSkip = false;
			j = 0;
			while (j < nabiertos) {
				if (abiertos[j]->id == sucesor->id && abiertos[j]->f <= sucesor->f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				i++;
				free(sucesor);
				continue;
			}

			/*Buscamos si hay un nodos con el mismo id en cerrados. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the closed nodes list." << endl;
			j = 0;
			while (j < ncerrados) {
				if (cerrados[j]->id == sucesor->id && cerrados[j]->f <= sucesor->f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				/*Free node*/
				free(sucesor);
			}
			else {
				if (DEBUG) cout << "Adding it to the open list." << endl;
				nabiertos++;
				mem_aux = (node**)realloc(abiertos, nabiertos * sizeof(node*));
				if (mem_aux == NULL) {
					cout << "[2]There has been some kind of problem with memory allocation. Not your fault!" << endl;
					/*TODO free memory*/
					clear_search_variables();
					return NULL;
				}
				abiertos = mem_aux;
				mem_aux = NULL;

				abiertos[nabiertos - 1] = sucesor;
			}

			i++;

		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;



		/*We create the kernel*/
		if (create_GPU_kernel_v3() == FAILURE) {
			cout << "There has been an error creating the kernel. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		status = clGetKernelWorkGroupInfo(kernel, devices[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(localWorkSize[0]), NULL);
		if (DEBUG) debug_GPU_errors(status);
		cl_int multiplier = 1;
		globalWorkSize[0] = maxComputeUnits;
		localWorkSize[0] = 1;

		/* Execute kernel. We look at each node (compute its h and f and look up if it has already been seen).*/
		if (DEBUG) {
			cout << "Executing kernel with the following parameters:" << endl;
			cout << "nsucesores: " << nsucesores << endl;
			for (i = 0; i < nsucesores; i++) {
				cout << "(" << sucesores[i]->id << ",f:" << sucesores[i]->f << ")|";
			}
			cout << endl;
			cout << "nabiertos: " << nabiertos << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "(" << abiertos[i]->id << ",f:" << abiertos[i]->f << ")|";
			}
			cout << endl;
			cout << "ncerrados: " << ncerrados << endl;
			for (i = 0; i < ncerrados; i++) {
				cout << "(" << cerrados[i]->id << ",f:" << cerrados[i]->f << ")|";
			}
			cout << endl;
		}

		status = clEnqueueNDRangeKernel(commandQueue, kernel, workDim, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if (status != CL_SUCCESS) {
			cout << "There has been an error executing the kernel. Exiting function..." << endl;
			if (DEBUG) {
				debug_GPU_errors(status);
			}
			/*TODO free memory*/
			return NULL;
		}
		/*We wait for it to finish*/
		status = clFinish(commandQueue);
		if (DEBUG) debug_GPU_errors(status);

		/*Read kernel's output*/
		if (DEBUG) cout << "Reading kernel's output." << endl;
		status = clEnqueueReadBuffer(commandQueue, buffers[nbuffers - 1], CL_TRUE, 0, sizeof(cl_int)*nsucesores, output, 0, NULL, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) {
			cout << "There has been an error reading the kernel's output. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		/*Read kernel's successors list*/
		nodeMin* aux2 = (nodeMin*)malloc(sizeof(nodeMin)*nsucesores);
		if (DEBUG) cout << "Reading kernel's updated successors list." << endl;
		status = clEnqueueReadBuffer(commandQueue, buffers[0], CL_TRUE, 0, sizeof(nodeMin)*nsucesores, aux2, 0, NULL, NULL);
		if (DEBUG) debug_GPU_errors(status);
		if (status != CL_SUCCESS) {
			cout << "There has been an error reading the kernel's output. Exiting function..." << endl;
			/*TODO free memory*/
			return NULL;
		}
		for (i = 0; i < nsucesores; i++) {
			sucesores[i]->f = aux2[i].f;
		}

		if (nsucesores == 0) { /*TODO should this be in another place?*/
							   /*TODO free memory*/
			break;
		}
		if (DEBUG) {
			cout << "Output de la GPU:" << endl;
			for (i = 0; i < nsucesores; i++) {
				cout << output[i] << " ";
			}
			cout << endl;
		}

		/*if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		if (append_closed_list(actual) == NULL) {
		cout << "There was some kind of error. Not your fault!" << endl;
		return NULL;
		}*/

		/*Watch which successors, already expanded, are going to be listed in the closed list.*/
		found = false;
		if (DEBUG) {
			cout << "Checking which successors are going to be included in the closed list." << endl;
			cout << "Expand index: " << expand << endl;
		}
		for (i = 0; i < expand; i++) {
			if (sucesores[i] == NULL) {
				continue;
			}
			if (DEBUG) cout << "i: " << i << " Nodo: " << sucesores[i]->id << ",f: " << sucesores[i]->f << ".Out: " << output[i] << endl;
			if (found) {
				free(sucesores[i]);
				continue;
			}

			if (output[i] == 2) {
				found = true;
				goal = sucesores[i];
			}
			else if (output[i] == 1) {
				if (append_closed_list(sucesores[i]) == NULL) {
					cout << "There was some kind of error. Not your fault!" << endl;
					return NULL;
				}
			}
			else {

				/*we first check if there is no conflict with the rest of the expanded nodes.*/
				bool flagAdd = true;

				for (j = i + 1; j < expand; j++) {
					if (sucesores[j] != NULL && sucesores[j]->id == sucesores[i]->id) {
						if (sucesores[j]->f <= sucesores[i]->f) {
							flagAdd = false;
							//free(sucesores[i]);
							break;
						}
						else {
							//free(sucesores[j]);
							//sucesores[j] = NULL;
						}
					}
				}
				/*then, if there is no conflict it is added*/
				if (flagAdd) {
					if (DEBUG) cout << "Added." << endl;
					if (append_closed_list(sucesores[i]) == NULL) {
						cout << "There was some kind of error. Not your fault!" << endl;
						return NULL;
					}
				}
				else {
					if (DEBUG) cout << "Not added." << endl;
				}
			}
		}

		/*Watch which successors, not expanded yet, are going to be listed in the open list.*/
		if (DEBUG) cout << "Checking which successors are going to be included in the open list." << endl;
		for (i = expand; i < nsucesores; i++) {
			if (sucesores[i] == NULL) {
				continue;
			}
			if (DEBUG) cout << "i: " << i << " Nodo: " << sucesores[i]->id << ",f: " << sucesores[i]->f << ".Out: " << output[i] << endl;
			if (found) {
				free(sucesores[i]);
				continue;
			}

			if (output[i] == 2) {
				found = true;
				goal = sucesores[i];
			}
			else if (output[i] == 1) {
				free(sucesores[i]);
			}
			else {
				/*we first check if there is no conflict with the rest of the nodes.*/
				bool flagAdd = true;
				for (j = i + 1; j < nsucesores; j++) {
					if (sucesores[j] != NULL && sucesores[j]->id == sucesores[i]->id) {
						if (sucesores[j]->f <= sucesores[i]->f) {
							free(sucesores[i]);
							flagAdd = false;
							break;
						}
						else {
							free(sucesores[j]);
							sucesores[j] = NULL;
						}
					}
				}

				/*then, if there is no conflict it is added*/
				if (flagAdd) {
					if (DEBUG) cout << "Added." << endl;
					if (append_open_list(sucesores[i]) == NULL) {
						cout << "There was some kind of error. Not your fault!" << endl;
						return NULL;
					}
				}
				else {
					if (DEBUG) cout << "Not added." << endl;
				}
			}
		}

		if (DEBUG) {
			cout << "We order our open list with quicksort." << endl;
			cout << "Before:" << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "(" << abiertos[i]->id << "," << abiertos[i]->f << ")-";
			}
			cout << endl;
		}
		quicksort(abiertos, 0, nabiertos - 1);
		if (DEBUG) {
			cout << "Afterwards:" << endl;
			for (i = 0; i < nabiertos; i++) {
				cout << "(" << abiertos[i]->id << "," << abiertos[i]->f << ")-";
			}
			cout << endl;
		}

		/*Free resources allocated for the kernel*/
		if (DEBUG) cout << "Freeing resources allocated for the kernel:" << endl;
		status = clReleaseKernel(kernel);
		i = 0;
		for (i = 0; i < nbuffers; i++) {
			clReleaseMemObject(buffers[i]);
		}
		/*TODO Free memory*/
		if (DEBUG) cout << "Freeing resources allocated for the search:" << endl;
		if (sucesores != NULL) {
			free(sucesores);
		}
		nsucesores = 0;
		sucesores = NULL;
		if (buffers != NULL) {
			free(buffers);
		}
		buffers = NULL;
		if (DEBUG) cout << endl << endl;
	}


	if (found) {
		path = get_path_A_star(goal);
	}
	else {
		path = NULL;
	}

	/*Free more allocated memory*/
	clear_search_variables();

	return path;

}

/*----- MEASURE ELAPSED TIME------- */
double SearchAStar::time_CPU_search_A_star() {

	clock_t start, end;
	double elapsed;
	cl_ulong* resultSearch;
	cl_ulong i;

 	if(DEBUG) cout << "time_CPU_search_A_star(...) starts." << endl;

	start = clock();
	resultSearch = search_A_star();
	end = clock();
	elapsed = double(end - start) / CLOCKS_PER_SEC;

	if (resultSearch == NULL) {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "There is no path between " << ini << " and " << fin << " nodes." << endl;
	}
	else {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "Path found with " << resultSearch[0] << " nodes! It is:";
		for (i = 0; i < resultSearch[0]; i++) {
			cout << " " << resultSearch[1 + i];
		}
		cout << endl;
	}

	return elapsed;
}

double SearchAStar::time_GPU_search_A_star() {

	clock_t start, end;
	double elapsed;
	cl_ulong* resultSearch;
	cl_ulong i;

	if (DEBUG) cout << "time_GPU_search_A_star(...) starts." << endl;

	start = clock();
	resultSearch = search_A_star_GPU();
	end = clock();
	elapsed = double(end - start) / CLOCKS_PER_SEC;

	if (resultSearch == NULL) {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "There is no path between " << ini << " and " << fin << " nodes." << endl;
	}
	else {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "Path found with " << resultSearch[0] << " nodes! It is:";
		for (i = 0; i < resultSearch[0]; i++) {
			cout << " " << resultSearch[1 + i];
		}
		cout << endl;
	}

	return elapsed;
}

double SearchAStar::time_GPU_search_A_star_v2() {

	clock_t start, end;
	double elapsed;
	cl_ulong* resultSearch;
	cl_ulong i;

	if (DEBUG) cout << "time_GPU_search_A_star_v2(...) starts." << endl;

	start = clock();
	resultSearch = search_A_star_GPU_v2();
	end = clock();
	elapsed = double(end - start) / CLOCKS_PER_SEC;

	if (resultSearch == NULL) {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "There is no path between " << ini << " and " << fin << " nodes." << endl;
	}
	else {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "Path found with " << resultSearch[0] << " nodes! It is:";
		for (i = 0; i < resultSearch[0]; i++) {
			cout << " " << resultSearch[1 + i];
		}
		cout << endl;
	}

	return elapsed;
}

/*----- ERRORS------- */
void SearchAStar::debug_GPU_errors(cl_int status) {
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
void SearchAStar::debug_print_connections() {
	cl_ulong i, j;
	cout << "The graph created is:" << endl;
	for (i = 0; i < nnodos; i++) {
		cout << i << ": ";
		for (j = 0; j < conexiones[i][0]; j++) {
			cout << conexiones[i][1 + j] << " ";
		}
		cout << endl;
	}

}

/*----- UTILITY ------- */
cl_bool SearchAStar::compareNodes(node* const &n1, node* const &n2) {
	return n1->f > n2->f;
}
void SearchAStar::clear_search_variables() {
	cl_ulong i = 0;
	if (DEBUG) cout << "Clearing successors list (" << nsucesores << " nodes)." << endl;
	for (i = 0; i < nsucesores; i++) {

		if (sucesores[i] != NULL) {
			free(sucesores[i]);
		}
	}
	if (sucesores != NULL) {
		free(sucesores);
	}

	nsucesores = 0;
	sucesores = NULL;

	if (DEBUG) cout << "Clearing open list (" << nsucesores << " nodes)." << endl;
	for (i = 0; i < nabiertos; i++) {
		if (abiertos[i] != NULL) {
			free(abiertos[i]);
		}
	}
	if (abiertos != NULL) {
		free(abiertos);
	}
	nabiertos = 0;
	abiertos = NULL;

	if (DEBUG) cout << "Clearing closed list (" << nsucesores << " nodes)." << endl;
	for (i = 0; i < ncerrados; i++) {
		if (cerrados[i] != NULL) {
			free(cerrados[i]);
		}
	}
	if (cerrados != NULL) {
		free(cerrados);
	}
	ncerrados = 0;
	cerrados = NULL;

	expand = 0;
	insert = 0;

}
node* SearchAStar::pop_open_list() {
	node *nodo = NULL;
	node **mem_aux = NULL;
	if (nabiertos == 0) {
		return NULL;
	}

	nodo = abiertos[nabiertos - 1];
	nabiertos--;
	mem_aux = (node**)realloc(abiertos, nabiertos * sizeof(node*));
	if (mem_aux == NULL && nabiertos > 0) {
			return NULL;
	}
	abiertos = mem_aux;
	mem_aux = NULL;

	return nodo;
}
node* SearchAStar::append_open_list(node* nodo) {
	node **mem_aux = NULL;
	nabiertos++;
	mem_aux = (node**)realloc(abiertos, nabiertos * sizeof(node*));
	if (mem_aux == NULL) {
		return NULL;
	}
	abiertos = mem_aux;
	mem_aux = NULL;
	abiertos[nabiertos - 1] = nodo;

	return nodo;
}
node* SearchAStar::pop_closed_list() {
	node *nodo = NULL;
	node **mem_aux = NULL;
	if (ncerrados == 0) {
		return NULL;
	}

	nodo = cerrados[ncerrados - 1];
	ncerrados--;
	mem_aux = (node**)realloc(cerrados, ncerrados * sizeof(node*));
	if (mem_aux == NULL && ncerrados > 0) {
		return NULL;
	}
	cerrados = mem_aux;
	mem_aux = NULL;

	return nodo;
}
node* SearchAStar::append_closed_list(node* nodo) {
	node **mem_aux = NULL;
	ncerrados++;
	mem_aux = (node**)realloc(cerrados, ncerrados * sizeof(node*));
	if (mem_aux == NULL) {
		return NULL;
	}
	cerrados = mem_aux;
	mem_aux = NULL;
	cerrados[ncerrados - 1] = nodo;

	return nodo;
}