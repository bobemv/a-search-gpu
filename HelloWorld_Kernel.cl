#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

typedef struct infonode {
	ulong id;
	float x;
	float y;
}infonode;

typedef struct node {
	ulong id;
	ulong parent;
	ulong type;
	float f;
	float g;
	float h;
}node;

typedef struct edge {
	ulong from;
	ulong to;
	ulong cost;
}edge;


float heuristic(__global infonode *infonodes, const ulong idStart, const ulong idEnd){
	float xStart, yStart, xEnd, yEnd;
	float leg1, leg2;
	int i;
	int reps = 1;
	float res;

	xStart = infonodes[idStart].x;
	yStart = infonodes[idStart].y;
	xEnd = infonodes[idEnd].x;
	yEnd = infonodes[idEnd].y;

	for(i=0; i < reps; i++){
		leg1 = fabs((float)(xStart - xEnd));
		leg2 = fabs((float)(yStart - yEnd));

		leg1 = pown(leg1, 2.0);
		leg2 = pown(leg2, 2.0);
		res = sqrt(leg1+leg2);
	}
	return res;
	
}

ulong search_cost_node_2_node(__global edge *conexiones, ulong nedges, ulong from, ulong to) {
	ulong i;
	ulong res = 0;

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


__kernel void searchastar_v1(__global infonode *infonodes,
						 __global node *nodes,
						 const ulong nsucesores,
						 const ulong idEnd,
						 const ulong nnodos){
	int num = get_global_id(0);
	int i, j;

	//printf("GPU - num: %d, nsucesores: %ld", num, nsucesores);
	if(num >= nsucesores){
		num = 0; //For those threads which doesn't have an associate succesor.
	}						 
						 
	nodes[num].h = heuristic(infonodes, nodes[num].type, idEnd);
	//printf("GPU - num: %d, H: %f", num, nodes[num].h);	

}

__kernel void searchastar_v2(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *nodes,
						 const node actual,
						 const ulong nsucesores,
						 const ulong idEnd,
						 const ulong nnodos,
						 const ulong nedges){
	int num = get_global_id(0);
	int i, j;

	//printf("GPU - num: %d", num);
	if(num >= nsucesores){
		num = 0; //For those threads which doesn't have an associate succesor.
	}						 
						 
	nodes[num].h = heuristic(infonodes, nodes[num].type, idEnd);
	nodes[num].g = actual.g + search_cost_node_2_node(conexiones, nedges, actual.type, nodes[num].type);
	nodes[num].f = nodes[num].g + nodes[num].h;
	//printf("GPU - num: %d, H: %f", num, nodes[num].h);		 
    
}

__kernel void searchastar_v3(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *abiertos,
						 __global node *cerrados,
						 __global node *sucesores,
						 __global ulong *out,
						 const node actual,
						 const ulong nsucesores,
						 const ulong nabiertos,
						 const ulong ncerrados,
						 const ulong nnodos,
						 const ulong nedges,
						 const ulong idEnd){
	int num = get_global_id(0);
	int i, j;
	node sucesor;

	//printf("GPU - num: %d", num);
	if(num >= nsucesores){
		num = 0; //For those threads which doesn't have an associate succesor.
	}						 
						 

	//printf("GPU - num: %d, H: %f", num, nodes[num].h);		 
 
	sucesor = sucesores[num];

	if (sucesor.type == idEnd) {
	    out[num] = 2;
		return;
	}

	sucesor.h = heuristic(infonodes, sucesor.type, idEnd);
	sucesor.g = actual.g + search_cost_node_2_node(conexiones, nedges, actual.type, sucesor.type);
	sucesor.f = sucesor.g + sucesor.h;

	bool flagSkip = false;
	j = 0;
	while (j < nabiertos) {
		if (abiertos[j].type == sucesor.type && abiertos[j].f <= sucesor.f) {
			out[num] = 0;
			return;
		}
		j++;
	}

	j = 0;
	while (j < ncerrados) {
		if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
			out[num] = 0;
			return;
		}
		j++;
	}

	sucesores[num] = sucesor;
	out[num] = 1;

}


ulong genera_sucesores(__global node *sucesores, __global edge *conexiones, const node nodo, const ulong nedges, const ulong indexnodes) {
	ulong i, j;
	ulong estimated;
	node n;

	ulong nsucesores = 0;
	ulong nids = indexnodes;

	for (i = 0; i < nedges; i++) {
		if (conexiones[i].from == nodo.type) {
			nsucesores++;
			n.parent = nodo.id;
			nids++;
			n.id = nids;
			n.type = conexiones[i].to;
			n.g = 0;
			n.h = 0;
			n.f = 0;
			sucesores[nsucesores-1] = n;
		}
		else if (conexiones[i].to == nodo.type) {
			nsucesores++;
			n.parent = nodo.id;
			nids++;
			n.id = nids;
			n.type = conexiones[i].from;
			n.g = 0;
			n.h = 0;
			n.f = 0;
			sucesores[nsucesores-1] = n;
		}
		
	}


	return nsucesores;
}


void bubblesort(__global node *elems, const ulong numElems){
	ulong i, j;
	node aux; 
	bool flagNoChanges = false;

	if(numElems == 0){
		return;
	}

	for (i = 0; i < (numElems - 1); i++) {
		flagNoChanges = true;
		for (j = 0; j < (numElems-1); j++) {
			if (elems[j + 1].f > elems[j].f) {
				flagNoChanges = false;
				aux = elems[j];
				elems[j] = elems[j + 1];
				elems[j + 1] = aux;
			}
		}

		if(flagNoChanges){
			break;
		}
	}

}

__kernel void searchastar_inside(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *abiertos,
						 __global node *cerrados,
						 __global node *sucesores,
						 __global node *out,
						 __global ulong *out_longs,
						 __global int *out_state,
						 const ulong nnodos,
						 const ulong nedges,
						 const ulong idStart,
						 const ulong idEnd,
						 const ulong nabiertos_o,
						 const ulong ncerrados_o,
						 const ulong indexnodes_o){
	int num = get_global_id(0);
	
	ulong i, j, k;

	ulong nabiertos = nabiertos_o;
	ulong ncerrados = ncerrados_o;
	node inicial;
	ulong indexnodes = indexnodes_o;
	bool found = false;
	ulong nsucesores = 0;
	bool flagSkip = false;
	node sucesor;
	node actual;
	int max = 5;
	int reps = 0;
	 
						 

	//Thread principal
	if(num == 0){
		out_state[0] = 0;

		if(nabiertos == 0){
			inicial.type = idStart;
			indexnodes++;
			inicial.id = indexnodes;
			inicial.g = 0;
			inicial.parent = 0;

			abiertos[nabiertos++] = inicial;
		}

		while (nabiertos > 0 && !found) {
			reps++;
			if(max <= reps){
				break;
			}

			actual = abiertos[nabiertos-1];

			nabiertos--;

			
			/*Generamos sucesores*/
			nsucesores = genera_sucesores(sucesores, conexiones, actual, nedges, indexnodes);

			indexnodes += nsucesores;
			if (nsucesores == 0) {
				break;
			}
			/* Expandimos cada sucesor*/
			i = 0;

			while (i < nsucesores) {
				//printf("sucesores[%ld].id=%ld, sucesores[%ld].type=%ld\n", i, sucesores[i].id, i, sucesores[i].type);

				sucesor = sucesores[i];

				/*Si es el nodo final, terminamos.*/
				if (sucesor.type == idEnd) {
					found = true;
					break;
				}


				/*Calculamos, f, g y h.*/
				sucesor.g = actual.g + search_cost_node_2_node(conexiones, nedges, actual.type, sucesor.type);
				sucesor.h = heuristic(infonodes, sucesor.type, idEnd);
				sucesor.f = sucesor.g + sucesor.h;

				/*Buscamos si hay un nodos con el mismo id en abiertos. Si existe Y con una f menor, se descarta el sucesor.*/
				flagSkip = false;
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
				j = 0;
				while (j < ncerrados) {

					if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
						flagSkip = true;
						break;
					}
					j++;
				}

				if (!flagSkip) {
					abiertos[nabiertos++] = sucesor;
				}

				i++;

			}

			nsucesores = 0;

			cerrados[ncerrados++] = actual;

			bubblesort(abiertos, nabiertos); 

					
		}

		
		if(max <= reps){
				//printf("TIME LIMIT\n");
				sucesor.id = 0;
				sucesor.type = 0;
				sucesor.parent = 0;
				out[0] = sucesor;
				out_longs[0] = nabiertos;
				out_longs[1] = ncerrados;
				out_longs[2] = indexnodes;
				out_state[0] = 2;
		}
		else{
			if(found){
				//printf("FOUND\n");
				out[0] = sucesor;
				out_state[0] = 1;
			}
			else{
				//printf("NOT FOUND\n");
				sucesor.id = 0;
				sucesor.type = 0;
				sucesor.parent = 0;
				out[0] = sucesor;
				out_state[0] = 0;
			}
		}
		

		
	}

}
__kernel void searchastar_playground(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *abiertos,
						 __global node *cerrados,
						 __global node *sucesores,
						 __global int *info_threads,
						 __global node *actual,
						 __global ulong *nlongs,
						 __global int *out_state,
						 __global node *out_result,
						 const ulong nnodos,
						 const ulong nedges,
						 const ulong idStart,
						 const ulong idEnd){


		int num = get_global_id(0);
		int numLocal = get_local_id(0);
		node sucesor;

		int i = 0;
		int reps = 0;

		while(true){
			if(reps > 0){
				return;
			}

			reps++;

			if(num != 128){
				return;
			}
		}

		out_result[0] = sucesor;
}

__kernel void searchastar(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *abiertos,
						 __global node *cerrados,
						 __global node *sucesores,
						 __global int *info_threads,
						 __global node *actual,
						 __global ulong *nlongs,
						 __global int *out_state,
						 __global node *out_result,
						 const ulong nnodos,
						 const ulong nedges,
						 const ulong idStart,
						 const ulong idEnd){
	/*nlongs: (4 ulongs)
	[0] = nabiertos
	[1] = ncerrados
	[2] = nsucesores
	[3] = indexnodes
	*/

	/*out_state: (1 int)
	Information about search state to host (CPU)
	0 = path not found
	1 = path found
	2 = time limit, search has not finished. kernel will have to be called again
	*/


	/*info_threads: (nnodos int)

	Information from child threads to main thread
	0 = ignore node
	1 = node is a candidate. Add it to open list
	2 = goal node. stop searching

	Information between child threads
	3 = node needs to be explored
	4 o 4+ = node is being explored by thread con globalid = num - 3

	*/

	int num = get_local_id(0);
	int numGlobal = get_global_id(0);
	int globalSize = get_global_size(0);
	int numGroup = get_group_id(0);
	int localSize = get_local_size(0);
	int groupSize = get_num_groups(0);

	ulong i, j, k;

	node inicial;
	__local bool found;
	bool flagSkip = false;
	node sucesor;
	int max = 20;
	int reps = 0;
	ulong nsucesores = 0;
	__local int beginToExpand;
	__local int numExpansiones;
	int numExpansionesChild = 0;
	int globalReps = 0;
	int option = 0;
	int numnodes = 0;
	
	//Thread principal

	if(num == 0){
		beginToExpand = 0;
		numExpansiones = 0;
		found = false;

		if(nlongs[0] == 0){
			inicial.type = idStart;
			nlongs[3]++;
			inicial.id = nlongs[3];
			inicial.g = 0;
			inicial.parent = 0;

			abiertos[nlongs[0]++] = inicial;
		}
	}

	barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);


	while(globalReps < max){

		barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

		if(nlongs[0] == 0 || found){
			break;
		}

		if(num == 0){
			actual[0] = abiertos[nlongs[0]-1];
			nlongs[0]--;

			nlongs[2] = genera_sucesores(sucesores, conexiones, actual[0], nedges, nlongs[3]);

			nlongs[3] += nlongs[2];

			if (nlongs[2] != 0) {
				for(i = 0; i < nlongs[2]; i++){
					info_threads[i] = 3;
				}
			}
			
		}

		barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

		globalReps++;
		
		if(nlongs[2] == 0){

			continue;
		}


		if(num != 0){ //Threads para sucesores	

			i = 0;
			j = 0;
			k = 0;
			numnodes = 0;

			nsucesores = nlongs[2];

			numnodes = (int) (nsucesores/(localSize-1));

			if((nsucesores % (localSize-1)) >= num){
				numnodes++;
			}
			

			for(k=0; k<numnodes; k++){


				i = (num-1) + (localSize-1)*k;
                sucesor = sucesores[i]; 

                if (sucesor.type == idEnd) {
                
                	info_threads[i]=2;
                    continue;
                }
				
                sucesor.h = heuristic(infonodes, sucesor.type, idEnd);

                sucesor.g = actual[0].g + search_cost_node_2_node(conexiones, nedges, actual[0].type, sucesor.type);

                sucesor.f = sucesor.g + sucesor.h;


                flagSkip = false;
                j = 0;
                while (j < nlongs[0]) {
                    if (abiertos[j].type == sucesor.type && abiertos[j].f <= sucesor.f) {
                        flagSkip = true;
                        break;
                    }
                    j++;
                }

                if(flagSkip){
                    info_threads[i]=0;
                    continue;
                }

                j = 0;
                while (j < nlongs[1]) {
                    if (cerrados[j].type == sucesor.type && cerrados[j].f <= sucesor.f) {
                        flagSkip = true;
                        break;
                    }
                    j++;
                }

                if(flagSkip){
                    info_threads[i]=0;
                    continue;
                }

                sucesores[i] = sucesor;
                info_threads[i]=1;

			}

		}//end if num != 0

		barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

		if(num == 0){

			i = 0;
			while (i < nlongs[2]) {
				option = info_threads[i];
				
				if(option == 2){
					found = true;
					sucesor = sucesores[i];
					nlongs[0]=0;
					i++;
				}
				else if (option == 1){
					abiertos[nlongs[0]] = sucesores[i];
					nlongs[0]++;
					i++;
				}
				else if(option == 0){
					i++;
				}
				else{
					printf("P-WTF...\n");
					i++;
				}
			}

			

			cerrados[nlongs[1]] = actual[0];
			nlongs[1]++;
			bubblesort(abiertos, nlongs[0]); 

			
		}

		

	}//while principal

	
    if(num == 0){
        if(max <= globalReps && !found){

                sucesor.id = 0;
                sucesor.type = 0;
                sucesor.parent = 0;
                
                out_result[0] = sucesor;
                out_state[0] = 2;

                nlongs[2] = 0;

        }
        else{

            if(found){

                out_result[0] = sucesor;

                out_state[0] = 1;

                nlongs[2] = 0;
            }
            else{
                out_state[0] = 0;

            }
        }
    }
	

	return;

}