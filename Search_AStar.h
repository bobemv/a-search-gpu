#pragma once

#include<CL/cl.h>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<cmath>
#include<time.h>
#include<chrono>
#include <string.h>
#include <string>
#include <fstream>

#include "OCLW.h"

#define SUCCESS 0
#define FAILURE 1
#define DEBUG false
#define RESULT true
#define TOFILE false

using namespace std;

struct node {
	cl_ulong id;
	cl_ulong parent;
	cl_ulong type;
	cl_float f, g, h;
};

struct edge {
	cl_ulong from;
	cl_ulong to;
	cl_ulong cost;
};

struct infonode {
	cl_ulong id;
	cl_float x;
	cl_float y;
};



class Search_AStar {
public:
	/*General variables*/
	cl_ulong nnodos;
	cl_ulong dim;
	cl_float sparsefactor;
	cl_uint maxcoste;
	cl_ulong ini;
	cl_ulong fin;
	edge *conexiones;
	cl_ulong nedges;
	cl_ulong indexnodes;

	typedef enum GraphTypes { STANDARD = 1, GRID = 2 };
	GraphTypes graphtype;

	/*CPU and GPU setup variables necessary for A* algorithm*/
	node *abiertos, *cerrados, *sucesores;
	cl_ulong nabiertos, ncerrados, nsucesores, expand, insert;
	infonode *infonodes;
	OCLW opencl;

	/*Location of GPU function*/
	char *filename;
	char *fun;

	/*Algorithm search evolution to file related variables*/
	unsigned long startTime;
	ofstream myfile;


	Search_AStar(cl_ulong nnodos, cl_float sparsefactor, cl_uint maxcoste, cl_uint maxdistance, cl_ulong ini, cl_ulong fin);
	Search_AStar(cl_ulong nnodos, cl_float sparsefactor, cl_uint maxcoste, cl_uint maxdistance, cl_ulong ini, cl_ulong fin, char *filename, char *fun);
	Search_AStar(cl_ulong nnodos, cl_float sparsefactor, cl_uint maxcoste, cl_uint maxdistance, cl_ulong ini, cl_ulong fin, char *filename, char *fun, OCLW opencl);
	Search_AStar(cl_ulong dim, cl_float blockfactor, cl_ulong ini, cl_ulong fin, char *filename, char *fun);

	~Search_AStar();

	/*----- STANDARD SORTING AND SEARCHING------- */
	int swap(node* numero1, node* numero2);
	int partir(node* tabla, cl_ulong ip, cl_ulong iu);
	int quicksort(node* tabla, cl_ulong ip, cl_ulong iu);

	/*----- GENERAL GRAPHS ------- */
	int create_undirected_graph();
	int create_undirected_graph_grid();
	cl_int binary_search(cl_int* nums, cl_int n, cl_int e);
	cl_ulong binary_search_odd(cl_ulong* nums, cl_ulong n, cl_ulong e);
	cl_ulong search_cost_node_2_node(cl_ulong from, cl_ulong to);
	node* genera_sucesores(node nodo);

	/*-----A* SEARCH------- */
	cl_ulong* get_path_A_star(node result);
	cl_ulong* search_A_star();
	cl_ulong* search_A_star_GPU_v1(); /*First implementation: only heuristic computation parallelizable*/
	cl_ulong* search_A_star_GPU_v2(); /*Second implementation: search for g value too.*/
	cl_ulong* search_A_star_GPU_v3(); /*Third implementation: check the successor with the closed and open nodes list. Complete implementation.*/
	cl_ulong* search_A_star_GPU();
	cl_ulong* search_A_star_GPU_inside();
	cl_ulong* search_A_star_GPU_inside_parallel();
	cl_ulong* search_A_star_GPU_inside_instances(cl_int numInstances);
	cl_ulong* search_A_star_CPU_inside_instances(cl_int numInstances);
	cl_ulong* search_A_star_CPU_instances(cl_int numInstances);
	/*----- HEURISTICS ------- */
	void infonodes_random(cl_uint maxdistance);
	void infonodes_grid();
	cl_float heuristic_distance(cl_ulong idStart, cl_ulong idEnd);

	/*----- MEASURE ELAPSED TIME------- */
	double time_CPU_search_A_star();
	double time_GPU_search_A_star();
	double time_GPU_v1_search_A_star();
	double time_GPU_v2_search_A_star();
	double time_GPU_v3_search_A_star();
	double time_GPU_inside_search_A_star();
	double time_GPU_inside_parallel_search_A_star();
	double time_GPU_inside_instances_search_A_star(int instances);
	double time_CPU_inside_instances_search_A_star(int instances);
	double time_CPU_instances_search_A_star(int instances);

	/*----- ERRORS------- */
	void debug_print_connections();

	/*----- UTILITY ------- */
	void random_start_end(int miniseed);
	cl_bool compareNodes(node* const &n1, node* const &n2);
	void clear_search_variables();
	node pop_open_list();
	node append_open_list(node nodo);
	node pop_closed_list();
	node append_closed_list(node nodo);
	//nodeGPU *copy_nodes_2_nodesGPU(node **nodes, unsigned long tam);
};