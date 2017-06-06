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


__kernel void searchastar(__global infonode *infonodes,
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