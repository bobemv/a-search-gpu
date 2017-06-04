#include "Search_AStar.h"

/*----- STANDARD SORTING AND SEARCHING------- */
int Search_AStar::swap(node* numero1, node* numero2) {
	node aux;

	if (numero1 == NULL || numero2 == NULL) {
		return FAILURE;
	}

	aux = *numero1;
	*numero1 = *numero2;
	*numero2 = aux;

	return SUCCESS;
}
int Search_AStar::partir(node* tabla, cl_ulong ip, cl_ulong iu) {

	cl_ulong m, i;
	cl_float k;

	m = (ip + iu) / 2;
	k = tabla[m].f;
	swap(&tabla[ip], &tabla[m]);
	m = ip;
	for (i = ip + 1; i <= iu; i++) {
		if (tabla[i].f>k) {
			m++;
			swap(&tabla[i], &tabla[m]);
		}
	}
	swap(&tabla[ip], &tabla[m]);
	return m;
}
int Search_AStar::quicksort(node* tabla, cl_ulong ip, cl_ulong iu) {
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

int Search_AStar::create_undirected_graph() {
	cl_ulong estimated = (((nnodos*(nnodos - 1)) / 2)*sparsefactor)+1;
	edge *edges = (edge*)malloc(estimated * sizeof(edge));
	edge *mem_aux = NULL;
	cl_ulong i, j, n, c;
	cl_float p;
	cl_float increment = 1.5;

	srand(time(NULL));

	nedges = 0;
	for (i = 0; i < nnodos; i++) {
		for (j = i; j < nnodos; j++) {
			if (i != j) {
				c = (rand() % maxcoste) + 1;
				p = (cl_float)rand() / (cl_float)RAND_MAX;
				if (sparsefactor > p) {
					nedges++;

					while(nedges > estimated){ /*We expand our array in case we run out of*/
						estimated *= increment;
						mem_aux = (edge*)realloc(edges, estimated * sizeof(edge));
						if (mem_aux == NULL) {
							/*TODO free memory management*/
							return FAILURE;
						}
						edges = mem_aux;
						mem_aux = NULL;
					}

					edges[nedges - 1].from = i;
					edges[nedges - 1].to = j;
					edges[nedges - 1].cost = c;
				}

			}
		}
	}


	conexiones = edges;

	return SUCCESS;

}

int Search_AStar::create_undirected_graph_grid()
{
	cl_ulong estimated = (cl_ulong)(dim*dim*sparsefactor) + 1;
	cl_ulong i, j, n, c;
	cl_float p;
	cl_float increment = 2.0;
	bool *isBlock = (bool*)malloc(nnodos*sizeof(bool));
	conexiones = (edge*)malloc(estimated * sizeof(edge));
	edge *mem_aux = NULL;

	srand(time(NULL));

	if (TOFILE) {
		unsigned long passTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		string filename = "times/" + to_string(passTime) + "_GRID-nnodos" + to_string(nnodos) + ".txt";
		myfile.open(filename, std::ofstream::out | std::ofstream::trunc);
	}

	for (i = 0; i < dim; i++)
	{
		for (j = 0; j < dim; j++)
		{
		    p = (cl_float)rand() / (cl_float)RAND_MAX;
			if (sparsefactor > p)
			{
				if (TOFILE) myfile << 0 << " ";
				
				isBlock[j + (i * dim)] = false;
			}
			else
			{
				if (TOFILE) myfile << 1 << " ";
				isBlock[j + (i * dim)] = true;
			}

		}
	}

	if (TOFILE) {
		myfile << endl;
		
		myfile.close();
	}

	nedges = 0;
	for (i = 0; i < dim; i++)
	{
		for (j = 0; j < dim; j++)
		{
			cl_ulong pos = j + (i * dim);
			if (!isBlock[pos])
			{

				cl_ulong derecha = j + 1;
				cl_ulong abajo = i + 1;

				if (derecha < dim)
				{
					derecha = pos + 1;
					if (!isBlock[derecha])
					{
						nedges++;

						while (nedges > estimated) { /*We expand our array in case we run out of*/
							estimated *= increment;
							mem_aux = (edge*)realloc(conexiones, estimated * sizeof(edge));
							if (mem_aux == NULL) {
								/*TODO free memory management*/
								return FAILURE;
							}
							conexiones = mem_aux;
							mem_aux = NULL;
						}

						conexiones[nedges - 1].from = pos;
						conexiones[nedges - 1].to = derecha;
						conexiones[nedges - 1].cost = 1;
					}
				}

				if (abajo < dim)
				{
					abajo = pos + dim;
					if (!isBlock[abajo])
					{

						nedges++;

						while (nedges > estimated) { /*We expand our array in case we run out of*/
							estimated *= increment;
							mem_aux = (edge*)realloc(conexiones, estimated * sizeof(edge));
							if (mem_aux == NULL) {
								/*TODO free memory management*/
								return FAILURE;
							}
							conexiones = mem_aux;
							mem_aux = NULL;
						}

						conexiones[nedges - 1].from = pos;
						conexiones[nedges - 1].to = abajo;
						conexiones[nedges - 1].cost = 1;
					}
				}
			}

		}
	}



	return SUCCESS;

}


int Search_AStar::binary_search(int* nums, int n, int e) {
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

cl_ulong Search_AStar::binary_search_odd(cl_ulong* nums, cl_ulong n, cl_ulong e) {
	/*1*/
	cl_ulong l = 0;
	cl_ulong r = n - 1;
	/*2*/
	while (l <= r) {
		/*3*/
		cl_ulong m = (l + r) / 2;
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

cl_ulong Search_AStar::search_cost_node_2_node(cl_ulong from, cl_ulong to) {
	cl_ulong i;
	cl_ulong res = 0;

	for (i = 0; i < nedges; i++) {
		if (conexiones[i].from == from && conexiones[i].to == to) {
			res = conexiones[i].cost;
		}
		if (conexiones[i].from == to && conexiones[i].to == from) {
			res = conexiones[i].cost;
		}
	}

	return res;
}

node* Search_AStar::genera_sucesores(node nodo) {
	cl_ulong i, j;
	cl_ulong estimated;
	node n;
	node *mem_aux = NULL;
	cl_float multiplier = 1.5;
	estimated = (((nnodos - 1) / 2)*sparsefactor) + 1;
	nsucesores = 0;
	sucesores = (node*)malloc(estimated * sizeof(node));
	for (i = 0; i < nedges; i++) {
		if (conexiones[i].from == nodo.type) {
			nsucesores++;
			while (nsucesores > estimated) {
				if (estimated == 1) {
					estimated = 2;
				}
				else {
					estimated *= multiplier;
				}
				
				mem_aux = (node*)realloc(sucesores, estimated*sizeof(node));
				if (mem_aux == NULL) {
					free(sucesores);
					sucesores = NULL;
					nsucesores = 0;
					return NULL;
				}
				sucesores = mem_aux;
				mem_aux = NULL;
			}
			n.parent = nodo.id;
			indexnodes++;
			n.id = indexnodes;
			n.type = conexiones[i].to;
			n.g = 0;
			n.h = 0;
			n.f = 0;
			sucesores[nsucesores-1] = n;
		}
		else if (conexiones[i].to == nodo.type) {
			nsucesores++;
			while (nsucesores > estimated) {
				
				if (estimated == 1) {
					estimated = 2;
				}
				else {
					estimated *= multiplier;
				}
				mem_aux = (node*)realloc(sucesores, estimated * sizeof(node));
				if (mem_aux == NULL) {
					free(sucesores);
					sucesores = NULL;
					nsucesores = 0;
					return NULL;
				}
				sucesores = mem_aux;
				mem_aux = NULL;
			}
			n.parent = nodo.id;
			indexnodes++;
			n.id = indexnodes;
			n.type = conexiones[i].from;
			n.g = 0;
			n.h = 0;
			n.f = 0;
			sucesores[nsucesores-1] = n;
		}
		
	}

	if (nsucesores > 0) {
		mem_aux = (node*)realloc(sucesores, nsucesores * sizeof(node));
		if (mem_aux == NULL) {
			free(sucesores);
			sucesores = NULL;
			nsucesores = 0;
			return NULL;
		}
		sucesores = mem_aux;
		mem_aux = NULL;
	}

	return sucesores;
}

/*-----A* SEARCH------- */
cl_ulong* Search_AStar::get_path_A_star(node result) {
	cl_ulong i = 0, j;
	cl_ulong* inversed = NULL;
	cl_ulong* path = NULL;
	cl_ulong* mem_aux = NULL;


	/*First we get all nodes which form the path*/
	while (result.parent != 0) {
		i++;

		mem_aux = (cl_ulong*)realloc(inversed, i * sizeof(cl_ulong));
		if (mem_aux == NULL) {
			return NULL;
		}
		inversed = mem_aux;
		mem_aux = NULL;
		inversed[i - 1] = result.type;

		for (j = 0; j < ncerrados; j++) {
			if (DEBUG) cout << "Cerrados: Id(" << cerrados[j].id << "), type(" << cerrados[j].type << ")" << endl;
			if (cerrados[j].id == result.parent) {
				if (DEBUG) cout << "CAMBIO" << endl;
				result = cerrados[j];
				break;
			}
		}
		/*
		cout << "--Id--: " << result.type << endl;
		cout << "G: " << result.g << endl;
		cout << "F: " << result.f << endl;
		cout << "----" << endl;*/
	}

	i++;
	mem_aux = (cl_ulong*)realloc(inversed, i * sizeof(cl_ulong));
	if (mem_aux == NULL) {
		return NULL;
	}
	inversed = mem_aux;
	mem_aux = NULL;
	inversed[i - 1] = result.type;

	/*Now we will reverse it and put its length as first element*/
	path = (cl_ulong*)malloc((i + 1) * sizeof(cl_ulong));
	path[0] = i;

	for (j = 1; j < i + 1; j++) {
		path[j] = inversed[i - j];
		if (TOFILE) {
			myfile << path[j] << " ";
		}
	}

	return path;
}
cl_ulong* Search_AStar::search_A_star() {
	node* mem_aux = NULL;
	cl_ulong* path;
	bool found = false;
	node actual;
	node sucesor;
	node inicial;
	cl_ulong i, j;

	clear_search_variables();

	inicial.type = ini;
	indexnodes++;
	inicial.id = indexnodes;
	inicial.g = 0;
	inicial.parent = 0;

	append_open_list(inicial);
	
	while (nabiertos > 0 && !found) {
		/*We obtain the node with smallest f value (located at the back of the list thanks to our quicksort).*/
		actual = pop_open_list();
		if (DEBUG) cout << "Actual node: " << actual.type << endl;


		if (TOFILE) {
			unsigned long passTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			myfile << to_string(passTime-startTime) << " " << to_string(actual.type) << " ";
		}

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}
		/* Expandimos cada sucesor*/
		i = 0;
		if (DEBUG) cout << nsucesores << " node(s) to be expanded. Expanding each successor node." << endl;
		while (i < nsucesores) {

			sucesor = sucesores[i];

			if (TOFILE) {
				myfile << to_string(sucesor.type) << " ";
			}

			if (DEBUG) cout << "Successor node: " << sucesor.type << endl;

			/*Si es el nodo final, terminamos.*/
			if (sucesor.type == fin) {
				if (DEBUG) cout << "Goal node (" << sucesor.type << ") found " << endl;
				found = true;
				break;
			}

			/*Calculamos, f, g y h.*/
			if (DEBUG) cout << "Computing f, g and h." << endl;
			sucesor.g = actual.g + search_cost_node_2_node(actual.type, sucesor.type);
			sucesor.h = heuristic_distance(sucesor.type, fin);
			if (DEBUG) cout << "H: "<<sucesor.h << endl;
			sucesor.f = sucesor.g + sucesor.h;

			/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the open nodes list." << endl;
			bool flagSkip = false;
			j = 0;
			while (j < nabiertos) {
				if (abiertos[j].type == sucesor.type && abiertos[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				i++;
				continue;
			}

			/*Buscamos si hay un nodos con el mismo id en cerrados. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the closed nodes list." << endl;
			j = 0;
			while (j < ncerrados) {
				if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (!flagSkip) {
				if (DEBUG) cout << "Adding it to the open list." << endl;
				append_open_list(sucesor);
			}

			i++;

		}

		if (TOFILE) {
			myfile << endl;
		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;

		if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		append_closed_list(actual);

		if (DEBUG) cout << "Quicksorting the open nodes list." << endl;
		/*printf("Antes bubblesort\n");
		printf("nabiertos: %ld\n", nabiertos);
		for (cl_ulong k = 0; k<nabiertos; k++) {
			cout << abiertos[k].id << "(" << abiertos[k].type << "):" << abiertos[k].f << " ";
		}
		cout << endl;*/
		quicksort(abiertos, 0, nabiertos - 1); 
		/*printf("Despues bubblesort\n");
		printf("nabiertos: %ld\n", nabiertos);
		for (cl_ulong k = 0; k<nabiertos; k++) {
			cout << abiertos[k].id << "(" << abiertos[k].type << "):" << abiertos[k].f << " ";
		}
		cout << endl;*/

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
cl_ulong* Search_AStar::search_A_star_GPU_v1() {
	node* mem_aux = NULL;
	cl_ulong* path;
	bool found = false;
	node actual;
	node sucesor;
	node inicial;
	cl_ulong i, j;

	/*-------- GPU ----------*/
	/*Extra variables necessary because of our GPU kernel*/
	cl_int status;
	size_t sizeAux;
	OCLW opencl;

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	status = opencl.GPU_program(filename);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Creating our kernel" << endl;
	status = opencl.GPU_kernel(fun);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Creating necessary buffers and immutable arguments" << endl;
	if (DEBUG) cout << "Buffer I: infonodes" << endl;
	status = opencl.GPU_buffer_input(nnodos * sizeof(infonode), infonodes);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	if (DEBUG) cout << "Creating output buffers for our kernel" << endl;
	if (DEBUG) cout << "Buffer I/O: sucesores" << endl;
	status = opencl.GPU_buffer_input_output_empty((nnodos - 1) * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: buffers" << endl;
	status = opencl.GPU_argument_buffers();

	if (DEBUG) cout << "Argument: idEnd" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&fin, 3);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	if (DEBUG) cout << "Argument: nnodos" << endl;
	status = opencl.GPU_argument(sizeof(nnodos), (void*)&nnodos, 4);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	/*-------- END GPU ----------*/

	clear_search_variables();

	inicial.type = ini;
	indexnodes++;
	inicial.id = indexnodes;
	inicial.g = 0;
	inicial.h = 0;
	inicial.f = 0;
	inicial.parent = 0;

	append_open_list(inicial);

	while (nabiertos > 0 && !found) {
		/*We obtain the node with smallest f value (located at the back of the list thanks to our quicksort).*/
		actual = pop_open_list();
		if (DEBUG) cout << "Actual node: " << actual.type << endl;

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}
		/* Expandimos cada sucesor*/
		if (DEBUG) cout << nsucesores << " node(s) to be expanded. Expanding each successor node." << endl;

		/*-------- GPU ----------*/
		/*We compute each successor heuristic in our kernel*/

		if (DEBUG) cout << "Argument variable: nsucesores" << endl;
		status = opencl.GPU_argument(sizeof(nsucesores), (void*)&nsucesores, 2);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			
			return NULL;
		}

		if (DEBUG) cout << "Updating successors buffer" << endl;
		status = opencl.GPU_buffer_update(1, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Computing optimal work sizes" << endl;
		status = opencl.GPU_work_sizes_optimal(nsucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*End setting up GPU variables*/

		/*Executing kernel*/
		if (DEBUG) cout << "Executing kernel" << endl;
		status = opencl.GPU_run();
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*Updating our succesor nodes' H value*/
		if (DEBUG) cout << "Reading the kernel's output" << endl;
		status = opencl.GPU_buffer_read_host(1, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		/*-------- END GPU ----------*/


		i = 0;

		while (i < nsucesores) {

			if (found) {
				break;
			}
			sucesor = sucesores[i];

			if (DEBUG) cout << "Successor node: " << sucesor.type << endl;

			/*Si es el nodo final, terminamos.*/
			if (sucesor.type == fin) {
				if (DEBUG) cout << "Goal node (" << sucesor.type << ") found " << endl;
				found = true;
				i++;
				continue;
			}

			/*Calculamos, f, g y h.*/
			if (DEBUG) cout << "Computing f and g in CPU." << endl;
			sucesor.g = actual.g + search_cost_node_2_node(actual.type, sucesor.type);
			sucesor.f = sucesor.g + sucesor.h;

			/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the open nodes list." << endl;
			bool flagSkip = false;
			j = 0;
			while (j < nabiertos) {
				if (abiertos[j].type == sucesor.type && abiertos[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				i++;
				continue;
			}

			/*Buscamos si hay un nodos con el mismo id en cerrados. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the closed nodes list." << endl;
			j = 0;
			while (j < ncerrados) {
				if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (!flagSkip) {
				if (DEBUG) cout << "Adding it to the open list." << endl;
				append_open_list(sucesor);
			}

			i++;

		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;

		if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		append_closed_list(actual);

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

	opencl.GPU_clear();
	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}
cl_ulong* Search_AStar::search_A_star_GPU_v2() {
	node* mem_aux = NULL;
	cl_ulong* path;
	bool found = false;
	node actual;
	node sucesor;
	node inicial;
	cl_ulong i, j;

	/*-------- GPU ----------*/
	/*Extra variables necessary because of our GPU kernel*/
	cl_int status;
	size_t sizeAux;
	OCLW opencl;

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	status = opencl.GPU_program(filename);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Creating our kernel" << endl;
	status = opencl.GPU_kernel(fun);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*Creating necessary immutable buffers and arguments*/
	if (DEBUG) cout << "Creating necessary buffers and immutable arguments" << endl;

	if (DEBUG) cout << "Buffer I: infonodes" << endl;
	status = opencl.GPU_buffer_input(nnodos * sizeof(infonode), infonodes);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I: conexiones" << endl;
	status = opencl.GPU_buffer_input(nedges * sizeof(edge), conexiones);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: sucesores" << endl;
	status = opencl.GPU_buffer_input_output_empty((nnodos-1) * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: buffers" << endl;
	status = opencl.GPU_argument_buffers();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	if (DEBUG) cout << "Argument: idEnd" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&fin, 5);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nnodos" << endl;
	status = opencl.GPU_argument(sizeof(nnodos), (void*)&nnodos, 6);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nedges" << endl;
	status = opencl.GPU_argument(sizeof(nedges), (void*)&nedges, 7);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	/*-------- END GPU ----------*/

	clear_search_variables();

	inicial.type = ini;
	indexnodes++;
	inicial.id = indexnodes;
	inicial.g = 0;
	inicial.h = 0;
	inicial.f = 0;
	inicial.parent = 0;

	append_open_list(inicial);

	while (nabiertos > 0 && !found) {
		/*We obtain the node with smallest f value (located at the back of the list thanks to our quicksort).*/
		actual = pop_open_list();
		if (DEBUG) cout << "Actual node: " << actual.type << endl;

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}
		/* Expandimos cada sucesor*/
		if (DEBUG) cout << nsucesores << " node(s) to be expanded. Expanding each successor node." << endl;

		/*-------- GPU ----------*/
		/*We compute each successor heuristic in our kernel*/

		/*We create the necessary buffers*/

		if (DEBUG) cout << "Argument variable: actual" << endl;
		status = opencl.GPU_argument(sizeof(actual), (void*)&actual, 3);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Argument variable: nsucesores" << endl;
		status = opencl.GPU_argument(sizeof(nsucesores), (void*)&nsucesores, 4);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		if (DEBUG) cout << "Updating successors buffer" << endl;
		status = opencl.GPU_buffer_update(2, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		if (DEBUG) cout << "Computing optimal work sizes" << endl;
		status = opencl.GPU_work_sizes_optimal(nsucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*End setting up GPU variables*/

		/*Executing kernel*/
		if (DEBUG) cout << "Executing kernel" << endl;
		status = opencl.GPU_run();
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*Updating our succesor nodes' H value*/
		if (DEBUG) cout << "Reading the kernel's output" << endl;
		status = opencl.GPU_buffer_read_host(2, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*-------- END GPU ----------*/


		i = 0;

		while (i < nsucesores) {

			if (found) {
				break;
			}
			sucesor = sucesores[i];

			if (DEBUG) cout << "Successor node: " << sucesor.type << endl;

			/*Si es el nodo final, terminamos.*/
			if (sucesor.type == fin) {
				if (DEBUG) cout << "Goal node (" << sucesor.type << ") found " << endl;
				found = true;
				i++;
				continue;
			}


			/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the open nodes list." << endl;
			bool flagSkip = false;
			j = 0;
			while (j < nabiertos) {
				if (abiertos[j].type == sucesor.type && abiertos[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (flagSkip) {
				i++;
				continue;
			}

			/*Buscamos si hay un nodos con el mismo id en cerrados. Si existe Y con una f menor, se descarta el sucesor.*/
			if (DEBUG) cout << "Looking up the closed nodes list." << endl;
			j = 0;
			while (j < ncerrados) {
				if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
					flagSkip = true;
					break;
				}
				j++;
			}

			if (!flagSkip) {
				if (DEBUG) cout << "Adding it to the open list." << endl;
				append_open_list(sucesor);
			}

			i++;

		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;

		if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		append_closed_list(actual);

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

	opencl.GPU_clear();

	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}
cl_ulong* Search_AStar::search_A_star_GPU_v3() {
	node* mem_aux = NULL;
	cl_ulong* path;
	bool found = false;
	node actual;
	node nodeFound;
	node inicial;
	cl_ulong i, j;

	/*-------- GPU ----------*/
	/*Extra variables necessary because of our GPU kernel*/
	cl_int status;
	size_t sizeAux;
	OCLW opencl;
	cl_ulong *output = NULL;

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	status = opencl.GPU_program(filename);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Creating our kernel" << endl;
	status = opencl.GPU_kernel(fun);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*Creating necessary immutable buffers and arguments*/
	if (DEBUG) cout << "Creating necessary buffers and immutable arguments" << endl;

	if (DEBUG) cout << "Buffer I: infonodes" << endl;
	status = opencl.GPU_buffer_input(nnodos * sizeof(infonode), infonodes);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	//cout << "Size: " << nedges * sizeof(edge) <<  "bytes" << endl;
	if (DEBUG) cout << "Buffer I: conexiones" << endl;
	status = opencl.GPU_buffer_input(nedges * sizeof(edge), conexiones);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I: abiertos" << endl;
	status = opencl.GPU_buffer_input_empty(nnodos * sizeof(node)); /*TODO shitty and it is going to be a problem in a matter of secs*/
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I: cerrados" << endl;
	status = opencl.GPU_buffer_input_empty(nnodos * sizeof(node)); /*TODO shitty and it is going to be a problem in a matter of secs*/
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: sucesores" << endl;
	status = opencl.GPU_buffer_input_output_empty((nnodos - 1) * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	if (DEBUG) cout << "Buffer O: output (ulong)" << endl;
	status = opencl.GPU_buffer_output((nnodos - 1) * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	
	if (DEBUG) cout << "Argument: buffers" << endl;
	status = opencl.GPU_argument_buffers();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nnodos" << endl;
	status = opencl.GPU_argument(sizeof(nnodos), (void*)&nnodos, 10);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nedges" << endl;
	status = opencl.GPU_argument(sizeof(nedges), (void*)&nedges, 11);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: idEnd" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&fin, 12);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	/*-------- END GPU ----------*/

	clear_search_variables();

	inicial.type = ini;
	indexnodes++;
	inicial.id = indexnodes;
	inicial.g = 0;
	inicial.h = 0;
	inicial.f = 0;
	inicial.parent = 0;

	append_open_list(inicial);


	while (nabiertos > 0 && !found) {
		/*We obtain the node with smallest f value (located at the back of the list thanks to our quicksort).*/
		actual = pop_open_list();
		if (DEBUG) cout << "Actual node: " << actual.type << endl;

		if (TOFILE) {
			unsigned long passTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			myfile << to_string(passTime - startTime) << " " << to_string(actual.type) << " ";
		}

		/*Generamos sucesores*/
		if (DEBUG) cout << "Generating list of successors." << endl;
		nsucesores = 0;
		genera_sucesores(actual);
		if (nsucesores == 0) {
			if (DEBUG) cout << "There are no more nodes left to explore and the goal node was not found." << endl;
			/*Free memory*/
			break;
		}
		/* Expandimos cada sucesor*/
		if (DEBUG) cout << nsucesores << " node(s) to be expanded. Expanding each successor node." << endl;

		/*-------- GPU ----------*/
		/*We compute each successor heuristic in our kernel*/

		if (DEBUG) cout << "Argument: actual" << endl;
		status = opencl.GPU_argument(sizeof(actual), (void*)&actual, 6);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Argument: nsucesores" << endl;
		status = opencl.GPU_argument(sizeof(nsucesores), (void*)&nsucesores, 7);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}
		
		if (DEBUG) cout << "Argument: nabiertos" << endl;
		status = opencl.GPU_argument(sizeof(nabiertos), (void*)&nabiertos, 8);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}
		
		if (DEBUG) cout << "Argument: ncerrados" << endl;
		status = opencl.GPU_argument(sizeof(ncerrados), (void*)&ncerrados, 9);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		if (DEBUG) cout << "Updating open nodes list buffer" << endl;
		if (nabiertos > 0) {
			status = opencl.GPU_buffer_update(2, nabiertos * sizeof(node), abiertos);
		}
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Updating closed nodes list buffer" << endl;
		if (ncerrados > 0) {
			status = opencl.GPU_buffer_update(3, ncerrados * sizeof(node), cerrados);
		}
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Updating successors buffer" << endl;
		status = opencl.GPU_buffer_update(4, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Computing optimal work sizes" << endl;
		status = opencl.GPU_work_sizes_optimal(nsucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*End setting up GPU variables*/

		/*Executing kernel*/
		if (DEBUG) cout << "Executing kernel" << endl;
		status = opencl.GPU_run();
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		output = (cl_ulong*)malloc(nsucesores * sizeof(cl_ulong));
		if (DEBUG) cout << "Reading the kernel's output" << endl;
		status = opencl.GPU_buffer_read_host(5, nsucesores * sizeof(cl_ulong), output);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		status = opencl.GPU_buffer_read_host(4, nsucesores * sizeof(node), sucesores);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*-------- END GPU ----------*/


		i = 0;

		while (i < nsucesores) {
			if (TOFILE) {
				myfile << to_string(sucesores[i].type) << " ";
			}
			//cout << output[i] << " " << sucesores[i].f << endl;
			switch (output[i]) {
				case 2:
					found = true;
					nodeFound = sucesores[i];
					break;
				case 1:
					append_open_list(sucesores[i]);
					break;
				case 0:
					break;
				default:
					/*TODO free memory*/
					return NULL;
			}
			i++;
		}

		if (TOFILE) {
			myfile << endl;
		}

		free(sucesores);
		nsucesores = 0;
		sucesores = NULL;

		free(output);
		output = NULL;

		if (DEBUG) cout << "Adding actual node to the closed nodes list." << endl;
		append_closed_list(actual);

		if (DEBUG) cout << "Quicksorting the open nodes list." << endl;
		quicksort(abiertos, 0, nabiertos - 1);

	}


	if (found) {
		if (DEBUG) cout << "Retrieving the generated path." << endl;
		path = get_path_A_star(nodeFound);
	}
	else {
		path = NULL;
	}

	/*Free memory*/
	if (DEBUG) cout << "Freeing memory and resetting variables." << endl;
	clear_search_variables();

	if (DEBUG) cout << "Clearing GPU resources." << endl;
	opencl.GPU_clear();


	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}

cl_ulong* Search_AStar::search_A_star_GPU_inside() {
	node* mem_aux = NULL;
	cl_ulong* path;
	ncerrados = nnodos;
	nabiertos = nnodos;
	/*-------- GPU ----------*/
	/*Extra variables necessary because of our GPU kernel*/
	cl_int status;
	size_t sizeAux;
	OCLW opencl;
	node *output = NULL;
	cl_ulong *output_longs = NULL;
	cl_int *output_state = NULL;

	output = (node*)malloc(sizeof(node));
	output_longs = (cl_ulong*)malloc(3 * sizeof(cl_ulong));
	output_state = (cl_int*)malloc(sizeof(cl_int));

	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	status = opencl.GPU_program(filename);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Creating our kernel" << endl;
	status = opencl.GPU_kernel(fun);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*Creating necessary immutable buffers and arguments*/
	if (DEBUG) cout << "Creating necessary buffers and immutable arguments" << endl;

	if (DEBUG) cout << "Buffer I: infonodes" << endl;
	status = opencl.GPU_buffer_input(nnodos * sizeof(infonode), infonodes);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I: conexiones" << endl;
	status = opencl.GPU_buffer_input(nedges * sizeof(edge), conexiones);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: abiertos" << endl;
	status = opencl.GPU_buffer_input_output_empty(nabiertos * nnodos * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: cerrados" << endl;
	status = opencl.GPU_buffer_input_output_empty(ncerrados * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: sucesores" << endl;
	status = opencl.GPU_buffer_input_output_empty((nnodos - 1) * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer O: output (node)" << endl;
	status = opencl.GPU_buffer_output(sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer O: output_longs (cl_ulong)" << endl;
	status = opencl.GPU_buffer_output(3*sizeof(cl_ulong));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer O: output_state (cl_int)" << endl;
	status = opencl.GPU_buffer_output(sizeof(cl_int));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: buffers" << endl;
	status = opencl.GPU_argument_buffers();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	if (DEBUG) cout << "Argument: nnodos" << endl;
	status = opencl.GPU_argument(sizeof(nnodos), (void*)&nnodos, 8);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nedges" << endl;
	status = opencl.GPU_argument(sizeof(nedges), (void*)&nedges, 9);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: idStart" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&ini, 10);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
	}

	if (DEBUG) cout << "Argument: idEnd" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&fin, 11);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	nabiertos = 0;
	if (DEBUG) cout << "Argument: nabiertos" << endl;
	status = opencl.GPU_argument(sizeof(nabiertos), (void*)&nabiertos, 12);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	ncerrados = 0;
	if (DEBUG) cout << "Argument: ncerrados" << endl;
	status = opencl.GPU_argument(sizeof(ncerrados), (void*)&ncerrados, 13);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	indexnodes = 0;
	if (DEBUG) cout << "Argument: indexnodes" << endl;
	status = opencl.GPU_argument(sizeof(indexnodes), (void*)&indexnodes, 14);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Computing optimal work sizes" << endl;
	status = opencl.GPU_work_sizes_optimal(1);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*Executing kernel*/
	if (DEBUG) cout << "Executing kernel" << endl;
	status = opencl.GPU_run();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Reading the kernel's output state (int=> 2=not finished, 1=found, 0=not found)" << endl;
	status = opencl.GPU_buffer_read_host(7, sizeof(cl_int), output_state);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	while (output_state[0] == 2) {

		if (DEBUG) cout << "Reading the kernel's output longs (3 cl_ulongs)" << endl;
		status = opencl.GPU_buffer_read_host(6, 3*sizeof(cl_ulong), output_longs);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		nabiertos = output_longs[0];
		if (DEBUG) cout << "Argument: nabiertos" << endl;
		status = opencl.GPU_argument(sizeof(nabiertos), (void*)&nabiertos, 12);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		ncerrados = output_longs[1];
		if (DEBUG) cout << "Argument: ncerrados" << endl;
		status = opencl.GPU_argument(sizeof(ncerrados), (void*)&ncerrados, 13);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}


		indexnodes = output_longs[2];
		if (DEBUG) cout << "Argument: indexnodes" << endl;
		status = opencl.GPU_argument(sizeof(indexnodes), (void*)&indexnodes, 14);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*Executing kernel*/
		if (DEBUG) cout << "Executing kernel" << endl;
		status = opencl.GPU_run();
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Reading the kernel's output state (int=> 2=not finished, 1=found, 0=not found)" << endl;
		status = opencl.GPU_buffer_read_host(7, sizeof(cl_int), output_state);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}



	}

	if (DEBUG) cout << "Reading the kernel's output result (node)" << endl;
	status = opencl.GPU_buffer_read_host(5, sizeof(node), output);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	ncerrados = nnodos;
	cerrados = (node*)malloc(ncerrados * sizeof(node));
	if (DEBUG) cout << "Reading the kernel's output (closed nodes list)" << endl;
	status = opencl.GPU_buffer_read_host(3, ncerrados * sizeof(node), cerrados);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*-------- END GPU ----------*/

	if (DEBUG) cout << "Result: Id(" << output[0].id << "), type(" << output[0].type << ")" << endl;
	if (output_state[0]) {
		if (DEBUG) cout << "Retrieving the generated path." << endl;
		path = get_path_A_star(output[0]);
	}
	else {
		path = NULL;
	}

	/*Free memory*/
	free(output);
	free(output_longs);
	free(output_state);

	if (DEBUG) cout << "Clearing GPU resources." << endl;
	opencl.GPU_clear();


	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}
cl_ulong* Search_AStar::search_A_star_GPU() {
	node* mem_aux = NULL;
	cl_ulong* path;
	ncerrados = nnodos;
	nabiertos = nnodos;
	/*-------- GPU ----------*/
	/*Extra variables necessary because of our GPU kernel*/
	cl_int status;
	size_t sizeAux;
	OCLW opencl;
	node *output = NULL;
	cl_ulong *nlongs = NULL;
	cl_int *output_state = NULL;
	cl_int *info_threads = NULL;
	int num_nlongs = 4;
	int num_childthreads = nnodos - 1;

	output = (node*)malloc(sizeof(node));
	nlongs = (cl_ulong*)malloc(num_nlongs * sizeof(cl_ulong));
	output_state = (cl_int*)malloc(sizeof(cl_int));
	info_threads = (cl_int*)malloc(num_childthreads * sizeof(cl_int));


	/*Creating context, command queue and program for our kernel*/
	status = opencl.GPU_setup();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	cout << "BUilding program " << endl;
	status = opencl.GPU_program(filename);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	cout << "Creating our kernel" << endl;
	status = opencl.GPU_kernel(fun);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	
	/*Creating necessary immutable buffers and arguments*/
	if (DEBUG) cout << "Creating necessary buffers and immutable arguments" << endl;

	if (DEBUG) cout << "Buffer I: infonodes" << endl;
	status = opencl.GPU_buffer_input(nnodos * sizeof(infonode), infonodes);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I: conexiones" << endl;
	status = opencl.GPU_buffer_input(nedges * sizeof(edge), conexiones);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: abiertos" << endl;
	status = opencl.GPU_buffer_input_output_empty(nabiertos * nnodos * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: cerrados" << endl;
	status = opencl.GPU_buffer_input_output_empty(ncerrados * sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	sucesores = (node*)malloc((nnodos - 1) * sizeof(node));
	for (cl_ulong i = 0; i < (nnodos-1); i++) {
		sucesores[i].id = 0;
	}
	if (DEBUG) cout << "Buffer I/O: sucesores" << endl;
	status = opencl.GPU_buffer_input_output((nnodos - 1) * sizeof(node), sucesores);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	for (cl_ulong i = 0; i < num_childthreads; i++) {
		info_threads[i] = 3;
	}
	if (DEBUG) cout << "Buffer I/O: info_threads" << endl;
	status = opencl.GPU_buffer_input_output(num_childthreads * sizeof(cl_int), info_threads);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer I/O: actual" << endl;
	status = opencl.GPU_buffer_input_output_empty(sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	nabiertos = 0;
	ncerrados = 0;
	nsucesores = 0;
	indexnodes = 0;
	nlongs[0] = nabiertos;
	nlongs[1] = ncerrados;
	nlongs[2] = nsucesores;
	nlongs[3] = indexnodes;

	if (DEBUG) cout << "Buffer I/O: nlongs (cl_ulong)" << endl;
	status = opencl.GPU_buffer_input_output(num_nlongs * sizeof(cl_ulong), nlongs);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	output_state[0] = 3;
	if (DEBUG) cout << "Buffer I/O: output_state (cl_int)" << endl;
	status = opencl.GPU_buffer_input_output(sizeof(cl_int), output_state);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Buffer O: out_result (node)" << endl;
	status = opencl.GPU_buffer_output(sizeof(node));
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}



	if (DEBUG) cout << "Argument: buffers" << endl;
	status = opencl.GPU_argument_buffers();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	if (DEBUG) cout << "Argument: nnodos" << endl;
	status = opencl.GPU_argument(sizeof(nnodos), (void*)&nnodos, 10);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: nedges" << endl;
	status = opencl.GPU_argument(sizeof(nedges), (void*)&nedges, 11);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Argument: idStart" << endl;
	status = opencl.GPU_argument(sizeof(ini), (void*)&ini, 12);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
	}

	if (DEBUG) cout << "Argument: idEnd" << endl;
	status = opencl.GPU_argument(sizeof(fin), (void*)&fin, 13);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	if (DEBUG) cout << "Computing optimal work sizes" << endl;
	status = opencl.GPU_work_sizes_optimal_onecore();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	/*Executing kernel*/
	if (DEBUG) cout << "Executing kernel" << endl;
	status = opencl.GPU_run();
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Reading the kernel's output state (int=> 2=not finished, 1=found, 0=not found)" << endl;
	status = opencl.GPU_buffer_read_host(8, sizeof(cl_int), output_state);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}


	while (output_state[0] == 2) {

		/*Resetting info_threads and output_state*/
		status = opencl.GPU_buffer_update(5, num_childthreads * sizeof(cl_int), info_threads);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		output_state[0] = 3;
		status = opencl.GPU_buffer_update(8, sizeof(cl_int), output_state);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		/*Executing kernel*/
		if (DEBUG) cout << "Executing kernel" << endl;
		status = opencl.GPU_run();
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

		if (DEBUG) cout << "Reading the kernel's output state (int=> 2=not finished, 1=found, 0=not found)" << endl;
		status = opencl.GPU_buffer_read_host(8, sizeof(cl_int), output_state);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}

	}

	if (DEBUG) cout << "Reading the kernel's output result (node)" << endl;
	status = opencl.GPU_buffer_read_host(9, sizeof(node), output);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}

	if (DEBUG) cout << "Reading the kernel's output nlongs (4*cl_ulong)" << endl;
	status = opencl.GPU_buffer_read_host(7, num_nlongs *sizeof(cl_ulong), nlongs);
	if (status != CL_SUCCESS) {
		if (DEBUG) opencl.debug_GPU_errors(status);
		/*TODO free memory*/
		return NULL;
	}
	nabiertos = nlongs[0];
	ncerrados = nlongs[1];
	nsucesores = nlongs[2];
	indexnodes = nlongs[3];
	if (DEBUG) {
		cout << "nabiertos: " << nabiertos << endl;
		cout << "ncerrados: " << ncerrados << endl;
		cout << "nsucesores: " << nsucesores << endl;
		cout << "indexnodes: " << indexnodes << endl;
	}
	if (ncerrados > 0) {
		cerrados = (node*)malloc(ncerrados * sizeof(node));
		if (DEBUG) cout << "Reading the kernel's output (closed nodes list)" << endl;
		status = opencl.GPU_buffer_read_host(3, ncerrados * sizeof(node), cerrados);
		if (status != CL_SUCCESS) {
			if (DEBUG) opencl.debug_GPU_errors(status);
			/*TODO free memory*/
			return NULL;
		}
	}

	if (DEBUG) {
		for (int i = 0; i < ncerrados; i++) {
			cout << "Id: " << cerrados[i].id << ", Type: " << cerrados[i].type << endl;
		}
	}
	/*-------- END GPU ----------*/

	if (DEBUG) cout << "Result: Id(" << output[0].id << "), type(" << output[0].type << ")" << endl;
	if (output_state[0]) {
		if (DEBUG) cout << "Retrieving the generated path." << endl;
		path = get_path_A_star(output[0]);
	}
	else {
		path = NULL;
	}

	/*Free memory*/
	free(output);
	free(nlongs);
	free(output_state);
	free(info_threads);

	if (DEBUG) cout << "Clearing GPU resources." << endl;
	opencl.GPU_clear();


	if (DEBUG) cout << "Exiting function." << endl;
	return path;

}



/*----- HEURISTICS ------- */
void Search_AStar::infonodes_random(cl_uint maxdistance) {
	cl_ulong i, c;
	cl_float p;
	
	infonodes = (infonode*)malloc(nnodos * sizeof(infonode));

	srand(time(NULL));
	
	i = 0;
	while (i < nnodos) {
		infonodes[i].id = i;
		c = (rand() % maxdistance) + 1;
		p = (cl_float)rand() / (cl_float)RAND_MAX;
		infonodes[i].x = p;
		c = (rand() % maxdistance) + 1;
		p = (cl_float)rand() / (cl_float)RAND_MAX;
		infonodes[i].y = p;
		i++;
	}

	return;
}

void Search_AStar::infonodes_grid()
{
	int i, j;

	infonodes = (infonode*)malloc(dim * dim * sizeof(infonode));

	for (i = 0; i<dim; i++)
	{
		for (j = 0; j<dim; j++)
		{
			int pos = (i * dim) + j;
			infonodes[pos].id = pos;
			infonodes[pos].x = i;
			infonodes[pos].y = j;

		}
	}


	return;
}

cl_float Search_AStar::heuristic_distance(cl_ulong idStart, cl_ulong idEnd) {
	cl_float xStart, yStart, xEnd, yEnd;
	cl_int i;
	cl_int reps = 1;
	cl_float res;
	float leg1, leg2;

	xStart = infonodes[idStart].x;
	yStart = infonodes[idStart].y;
	xEnd = infonodes[idEnd].x;
	yEnd = infonodes[idEnd].y;

	for (i = 0; i < reps; i++) {
		leg1 = fabs(xStart - xEnd);
		leg2 = fabs(yStart - yEnd);
		leg1 = pow(leg1, 2);
		leg2 = pow(leg2, 2);
		res = sqrt(leg1 + leg2);
	}
	return res;
}

/*----- MEASURE ELAPSED TIME------- */
double Search_AStar::time_CPU_search_A_star() {

	clock_t start, end;
	double elapsed;
	cl_ulong* resultSearch;
	cl_ulong i;

	if (DEBUG) cout << "time_CPU_search_A_star(...) starts." << endl;

	if (TOFILE) {
		startTime = 0;
		startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		string filename = "times/"+ to_string(startTime) +"_CPU-nnodos" + to_string(nnodos) + ".txt";

		myfile.open(filename, std::ofstream::out | std::ofstream::trunc);

		myfile << to_string(nnodos) << endl;
		myfile << to_string(dim) << endl;
	}

	start = clock();
	resultSearch = search_A_star();
	end = clock();
	elapsed = double(end - start) / CLOCKS_PER_SEC;

	if (TOFILE) myfile.close();

	if (resultSearch == NULL) {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "There is no path between " << ini << " and " << fin << " nodes." << endl;
		//cout << "0" << endl;
	}
	else {
		//cout << resultSearch[0] << endl;
		cout << "Time elapsed: " << elapsed << endl;
		cout << "Path found with " << resultSearch[0] << " nodes! It is:";
		for (i = 0; i < resultSearch[0]; i++) {
			cout << " " << resultSearch[1 + i];
		}
		cout << endl;
	}

	return elapsed;
}

double Search_AStar::time_GPU_search_A_star() {

	clock_t start, end;
	double elapsed;
	cl_ulong* resultSearch;
	cl_ulong i;

	if (DEBUG) cout << "time_GPU_search_A_star(...) starts." << endl;

	if (TOFILE) {
		startTime = 0;
		startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		string filename = "times/"+ to_string(startTime) +"_GPU-nnodos" + to_string(nnodos) + ".txt";
		myfile.open(filename, std::ofstream::out | std::ofstream::trunc);
		myfile << to_string(nnodos) << endl;
		myfile << to_string(dim) << endl;
	}

	start = clock();
	resultSearch = search_A_star_GPU();
	end = clock();
	elapsed = double(end - start) / CLOCKS_PER_SEC;

	if (TOFILE) myfile.close();

	if (resultSearch == NULL) {
		cout << "Time elapsed: " << elapsed << endl;
		cout << "There is no path between " << ini << " and " << fin << " nodes." << endl;
		//cout << "0" << endl;
	}
	else {
		//cout << resultSearch[0] << endl;
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
void Search_AStar::debug_print_connections() {
	cl_ulong i, j;
	cout << "The edges are:" << endl;
	cout << "from -> cost -> to" << endl;
	for (i = 0; i < nedges; i++) {
		cout << conexiones[i].from << "->" << conexiones[i].cost << "->" << conexiones[i].to << endl;
	}

}

/*----- UTILITY ------- */
void Search_AStar::random_start_end() {
	
	srand(time(NULL));

	ini = rand() % nnodos;

	ini = rand() % nnodos;

}
cl_bool Search_AStar::compareNodes(node* const &n1, node* const &n2) {
	return n1->f > n2->f;
}
void Search_AStar::clear_search_variables() {
	cl_ulong i = 0;
	if (DEBUG) cout << "Clearing successors list (" << nsucesores << " nodes)." << endl;

	if (sucesores != NULL && nsucesores > 0) {
		free(sucesores);
	}

	nsucesores = 0;
	sucesores = NULL;

	if (DEBUG) cout << "Clearing open list (" << nsucesores << " nodes)." << endl;

	if (abiertos != NULL && nabiertos > 0) {
		free(abiertos);
	}
	nabiertos = 0;
	abiertos = NULL;

	if (DEBUG) cout << "Clearing closed list (" << nsucesores << " nodes)." << endl;

	if (cerrados != NULL && ncerrados > 0) {
		free(cerrados);
	}
	ncerrados = 0;
	cerrados = NULL;

	expand = 0;
	insert = 0;

}
node Search_AStar::pop_open_list() {
	node nodo;
	node *mem_aux = NULL;
	nodo.id = 0;

	if (nabiertos == 0) {
		return nodo;
	}

	nodo = abiertos[nabiertos - 1];
	nabiertos--;
	mem_aux = (node*)realloc(abiertos, nabiertos * sizeof(node));
	if (mem_aux == NULL && nabiertos > 0) {
		nodo.id = 0;
		return nodo;
	}
	abiertos = mem_aux;
	mem_aux = NULL;

	return nodo;
}
node Search_AStar::append_open_list(node nodo) {
	node *mem_aux = NULL;
	node res;
	res.id = 0;

	nabiertos++;
	mem_aux = (node*)realloc(abiertos, nabiertos * sizeof(node));
	if (mem_aux == NULL) {
		return res;
	}
	abiertos = mem_aux;
	mem_aux = NULL;
	abiertos[nabiertos - 1] = nodo;

	res = nodo;
	return res;
}
node Search_AStar::pop_closed_list() {
	node nodo;
	node *mem_aux = NULL;
	nodo.id = 0;

	if (ncerrados == 0) {
		return nodo;
	}

	nodo = cerrados[ncerrados - 1];
	ncerrados--;
	mem_aux = (node*)realloc(cerrados, ncerrados * sizeof(node));
	if (mem_aux == NULL && ncerrados > 0) {
		nodo.id = 0;
		return nodo;
	}
	cerrados = mem_aux;
	mem_aux = NULL;

	return nodo;
}
node Search_AStar::append_closed_list(node nodo) {
	node *mem_aux = NULL;
	node res;
	res.id = 0;

	ncerrados++;
	mem_aux = (node*)realloc(cerrados, ncerrados * sizeof(node));
	if (mem_aux == NULL) {
		return res;
	}
	cerrados = mem_aux;
	mem_aux = NULL;
	cerrados[ncerrados - 1] = nodo;
	res = nodo;

	return res;
}

Search_AStar::Search_AStar(cl_ulong nnodos, cl_float sparsefactor, cl_uint maxcoste, cl_uint maxdistance, cl_ulong ini, cl_ulong fin)
{
	Search_AStar::nnodos = nnodos;
	Search_AStar::dim = 0;
	Search_AStar::sparsefactor = sparsefactor;
	Search_AStar::maxcoste = maxcoste;
	Search_AStar::ini = ini;
	Search_AStar::fin = fin;
	Search_AStar::graphtype = Search_AStar::STANDARD;
	nabiertos = ncerrados = nsucesores = expand = insert = indexnodes = 0;
	abiertos = cerrados = sucesores = NULL;
	create_undirected_graph();
	while (nedges == 0) {
		create_undirected_graph();
	}
	infonodes_random(maxdistance);
}
Search_AStar::Search_AStar(cl_ulong nnodos, cl_float sparsefactor, cl_uint maxcoste, cl_uint maxdistance, cl_ulong ini, cl_ulong fin, char *filename, char *fun)
{
	Search_AStar::nnodos = nnodos;
	Search_AStar::dim = 0;
	Search_AStar::sparsefactor = sparsefactor;
	Search_AStar::maxcoste = maxcoste;
	Search_AStar::ini = ini;
	Search_AStar::fin = fin;
	Search_AStar::graphtype = Search_AStar::STANDARD;
	nabiertos = ncerrados = nsucesores = expand = insert = indexnodes = 0;
	abiertos = cerrados = sucesores = NULL;
	create_undirected_graph();
	while (nedges == 0) {
		create_undirected_graph();
	}

	infonodes_random(maxdistance);
	Search_AStar::filename = filename;
	Search_AStar::fun = fun;
	//filename = "HelloWorld_Kernel.cl";
	//fun = "searchastar";
}

Search_AStar::Search_AStar(cl_ulong dim, cl_float blockfactor, cl_ulong ini, cl_ulong fin, char *filename, char *fun)
{
	Search_AStar::nnodos = dim*dim;
	Search_AStar::dim = dim;
	Search_AStar::sparsefactor = blockfactor;
	Search_AStar::maxcoste = 0;
	Search_AStar::ini = ini;
	Search_AStar::fin = fin;
	Search_AStar::graphtype = Search_AStar::GRID;
	nabiertos = ncerrados = nsucesores = expand = insert = indexnodes = 0;
	abiertos = cerrados = sucesores = NULL;
	create_undirected_graph_grid();
	while (nedges == 0) {
		create_undirected_graph();
	}
	infonodes_grid();
	Search_AStar::filename = filename;
	Search_AStar::fun = fun;
	//filename = "HelloWorld_Kernel.cl";
	//fun = "searchastar";
}

Search_AStar::~Search_AStar(){
	//cout << "ENTRANDO AL DESTRUCTOR" << endl;
	if (conexiones != NULL && nedges > 0) {
		free(conexiones);
		nedges = 0;
		conexiones = NULL;

		if (infonodes != NULL) {
			free(infonodes);
			infonodes = NULL;
		}
	}

	clear_search_variables();

	nnodos = 0;
	dim = 0;
	sparsefactor = 0;
	maxcoste = 0;
	ini = 0;
	fin = 0;
	indexnodes = 0;

	filename = NULL;
	fun = NULL;
}