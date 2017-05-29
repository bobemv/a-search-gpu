
#ifndef SEARCHASTAR_H_
#define SEARCHASTAR_H_

#include<CL/cl.h>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<queue>
#include<list>
#include<cmath>
#include<time.h>
#include<chrono>
#include <string.h>
#include <string>
#include <fstream>

#define SUCCESS 0
#define FAILURE 1
#define DEBUG false

using namespace std;

struct node {
	cl_ulong id;
	node *parent;
	cl_float f, g, h;
};

struct nodeMin {
	cl_ulong id;
	cl_float f;
};

struct edge {
	cl_ulong start;
	cl_ulong end;
};

struct nodeGPU {
	cl_ulong parent;
	cl_ulong id;
	cl_ulong nodeNum;
	cl_float f, g, h;
};

struct infonode {
	cl_ulong id;
	cl_ulong x, y;
};

/*
* SearchAStar
* Class with A* search algorithms
*/

class SearchAStar
{
		/*General variables*/
		cl_ulong dim;
		cl_ulong nnodos;
		cl_float blockfactor;
		cl_ulong ini;
		cl_ulong fin;
		cl_ulong** conexiones;
		/*CPU and GPU setup variables necessary for A* algorithm*/
		node** abiertos, **cerrados, **sucesores;
		cl_ulong nabiertos, ncerrados, nsucesores, expand, insert;


		/*GPU setup variables necessary for A* algorithm*/
		char *filename;
		char *fun;
		cl_device_id *devices;
		cl_context context;
		cl_command_queue commandQueue;
		cl_program program;
		cl_mem *buffers;
		cl_uint nbuffers;
		cl_kernel kernel;
		cl_int* output = NULL;


		cl_ulong maxComputeUnits;
		cl_ulong maxWorkGroupSize;
		cl_uint workDim;
		size_t* globalWorkSize = NULL;
		size_t* localWorkSize = NULL;
		cl_ulong totalSize;

		/*Version 2 variables*/
		edge *edges;
		cl_ulong nedges;
		nodeGPU *output2 = NULL;
public:
	SearchAStar(cl_ulong dim, cl_float blockfactor, cl_ulong ini, cl_ulong fin)
	{
		SearchAStar::dim = dim;
		SearchAStar::nnodos = dim*dim;
		SearchAStar::blockfactor = blockfactor;
		SearchAStar::ini = ini;
		SearchAStar::fin = fin;
		nabiertos = ncerrados = nsucesores = expand = insert = 0;
		abiertos = cerrados = sucesores = NULL;
		create_undirected_graph_grid();
		//create_undirected_graph_grid_v2();
		if (DEBUG) cout << "Initialiting the object" << endl;
		filename = "HelloWorld_Kernel.cl";
		fun = "searchastar";
	}

	/*----- STANDARD SORTING AND SEARCHING------- */
	int swap(node** numero1, node** numero2);
	int partir(node** tabla, cl_ulong ip, cl_ulong iu);
	int quicksort(node** tabla, cl_ulong ip, cl_ulong iu);

	/*----- GENERAL GRAPHS ------- */
	cl_int** create_undirected_graph(cl_int nnodos, cl_int maxcoste, cl_float sparsefactor);
	cl_int binary_search(cl_int* nums, cl_int n, cl_int e);
	cl_int binary_search_odd(cl_int* nums, cl_int n, cl_int e);
	cl_int search_cost_node_2_node(cl_int** conexiones, cl_int from, cl_int to);
	node** genera_sucesores(node* nodo, cl_int** conexiones, cl_int nnodos);

	/*-----GRIDLIKE GRAPHS------- */
	int create_undirected_graph_grid();
	int genera_sucesores_grid(node* nodo);
	int genera_sucesores_grid_GPU();
	cl_float heuristic_grid(cl_ulong idStart, cl_ulong idEnd);

	/*New function using edge structure and an optimal way of creating said edges.
	However it will be more costly to look up for specific nodes in the new structure.
	Edges are only stored. It is memory efficient, but not time efficient.
	Only valid for a 4-way squares grid.*/
	int create_undirected_graph_grid_v2();

	/*----- GPU------- */
	int convertToString(const char *filename, string& s);
	int setup_GPU_variables();
	int create_GPU_program();
	int create_GPU_kernel();
	int create_GPU_kernel_v2();
	int create_GPU_kernel_v3();

	/*-----A* SEARCH------- */
	cl_ulong* get_path_A_star(node* result);
	cl_ulong* search_A_star();
	cl_ulong* search_A_star_GPU();
	
	/*New version where the whole A* search algorithm is implemented in GPU. - WIP*/
	cl_ulong* search_A_star_GPU_v2();

	/*New version simplified. Only is offloaded to the gpu the heuristic computation. - WIP*/
	cl_ulong* search_A_star_GPU_v3();

	/*----- MEASURE ELAPSED TIME------- */
	double time_CPU_search_A_star();
	double time_GPU_search_A_star();
	double time_GPU_search_A_star_v2();

	/*----- ERRORS------- */
	void debug_GPU_errors(cl_int status);
	void debug_print_connections();

	/*----- UTILITY ------- */
	cl_bool compareNodes(node* const &n1, node* const &n2);
	void clear_search_variables();
	node *pop_open_list();
	node *append_open_list(node *nodo);
	node *pop_closed_list();
	node *append_closed_list(node *nodo);
};
#endif // !SEARCHASTAR_H_
