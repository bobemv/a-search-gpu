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


__kernel void searchastar(__global infonode *infonodes,
						 __global edge *conexiones,
						 __global node *abiertos,
						 __global node *cerrados,
						 __global node *sucesores,
						 __global ulong *nlongs,
						 __global node *out,
						 __global int *out_state,
						 const ulong nnodos,
						 const ulong nedges,
						 const ulong idStart,
						 const ulong idEnd){
	int num = get_global_id(0);
	
	ulong i, j, k;

	ulong nabiertos = nlongs[0];
	ulong ncerrados = nlongs[1];
	node inicial;
	ulong indexnodes = nlongs[2];
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
				nlongs[0] = nabiertos;
				nlongs[1] = ncerrados;
				nlongs[2] = indexnodes;
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