
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "td_nanojoin.h"
#include "td_patchAdjacency.h"
#include "td_graphutils.h"
#include "td_operations.h"

struct edge* searchDangling(struct td_patch *patch, int nr);

void dfs(struct td_patch* patch) {
	int i, temp, size;
	struct ufaces* fini;
	unsigned char *bordercode;
	int pentres, hexres, heptres, ivres, faceres;
	struct edge *mark;
	if (patch->ufaces == NULL) {
		if (!EXACTFACES || (patch->facesleft[1] == 0 && patch->facesleft[3] == 0)) {
			newJoin(patch);
		}
	} else if (patch->facesleft[0] >= 0) {
		get_bordercode(patch->ufaces->current, patch->nrofvertices, &bordercode);

		/* COUNT NUMBER OF TWOS */
		temp = 0;
		for (i=1; i <= bordercode[0]; i++) {
			if (bordercode[i] == 0) {
				temp++;
			}
		}
		size = bordercode[0];
		free(bordercode);
		/* FINISHED UFACE */
		if (temp == 0) {
			pentres = hexres = heptres = ivres = faceres = 0; //to avoid -Wmaybe-uninitialized
			if (size >= 5 && size <= 7 && patch->facesleft[size - 4] > 0) {
				if (0 == patch->ufaces->toborderbuiltnr) {
					patch->facesleft[size - 4]--;
					patch->facesused[size-5]++;
					fini = patch->ufaces;
					patch->ufaces = fini->next;
					if (patch->ufaces != NULL) {
						faceres = patch->ufaces->faceleftres;
						pentres = patch->ufaces->pentres;
						hexres = patch->ufaces->hexres;
						heptres = patch->ufaces->heptres;
						ivres = patch->ufaces->ivres;
						patch->facesleft[0] += faceres;
						patch->facesleft[1] += pentres;
						patch->facesleft[2] += hexres;
						patch->facesleft[3] += heptres;
						patch->maxinternalvertices += ivres;
						patch->ufaces->pentres = patch->ufaces->heptres = patch->ufaces->hexres = patch->ufaces->ivres = 0;
					}
					dfs(patch);
					if (patch->ufaces != NULL) {
						patch->facesleft[0] -= faceres;
						patch->facesleft[1] -= pentres;
						patch->facesleft[2] -= hexres;
						patch->facesleft[3] -= heptres;
						patch->maxinternalvertices -= ivres;
						patch->ufaces->faceleftres = faceres;
						patch->ufaces->pentres = pentres;
						patch->ufaces->hexres = hexres;
						patch->ufaces->heptres = heptres;
						patch->ufaces->ivres = ivres;
					}
					patch->ufaces = fini;
					patch->facesleft[size-4]++;
					patch->facesused[size-5]--;
				}
			}
		} else if (patch->facesleft[0] > 0) {
			if (patch->ufaces->toborderbuiltnr != 0) {
				/* We don't have all inner special faces */
				mark = patch->ufaces->minedge;
				split(patch, mark);
				unfold(patch, mark);
				unwrap(patch, mark);
			} else {
				/* Use filling algorithm | returns 1 if you have to split*/
				if (fill(patch)) {
					mark = patch->ufaces->minedge;
					split(patch, mark);
					unwrap(patch, mark);
				}
			}
		}
	}
}

unsigned char comparearrays(int *array1, int *array2, int length) {
	int i, j, l, m;
	unsigned char same;
	same = 1;
	for (i=0; i < length; i++) {
		l = array1[2*i];
		m = array1[2*i+1];
		j = 0;
		while(j < length && (l != array2[2*j] || m != array2[2*j+1])) {
			j += 1;
		}
		if (j == length) {
			same = 0;
		} else {
			array2[2*j] = -1;
			array2[2*j+1] = -1;
		}
	}
	return same;
}


void init_isomorphism() {
	int i, j, l, m, counter;
	int *temparray1, *temparray2;
	unsigned char same;
	counter = 0;
	//what needs to be checked
	for (i = 0; i < insideparameters[0]; i++) {
		if (insideparameters[2 * i + 1] == outsideparameters[0] && insideparameters[2 * i + 2] == outsideparameters[1]) {
			checknormal = 1;
		}
		if (insideparameters[2 * i + 1] == outsideparameters[1] && insideparameters[2 * i + 2] == outsideparameters[0]) {
			if (insideparameters[0] == 1){
				checkinverse = 1;
			} else {
				temparray1 = malloc(2*(insideparameters[0]-1)*sizeof(int));
				temparray2 = malloc(2*(insideparameters[0]-1)*sizeof(int));
				counter = 0;
				/* Flip rest */
				for (j=0; j < insideparameters[0]; j++) {
					if (j != i) {
						l = insideparameters[2*j+1];
						m = insideparameters[2*j+2];
						temparray1[counter] = l;
						temparray1[counter+1] = m;
						if (m != 0) {
							temparray2[counter] = m;
							temparray2[counter+1] = l;
						} else {
							temparray2[counter] = l;
							temparray2[counter+1] = m;
						}
						counter += 2;
					}
				}

				/* compare two temparrays */
				same = comparearrays(temparray1, temparray2, insideparameters[0]-1);
				if (same) {
					checkinverse = 1;
				}
				free(temparray1);
				free(temparray2);
			}
		}
	}
	if (outsideparameters[1] == 0) {
		checknormal = 1;
		checkinverse = 1;
	}
	if (outsideparameters[0] == outsideparameters[1]) {
		temparray1 = malloc(2*(insideparameters[0])*sizeof(int));
		temparray2 = malloc(2*(insideparameters[0])*sizeof(int));
		counter = 0;
		/* Flip rest */
		for (j=0; j < insideparameters[0]; j++) {
			l = insideparameters[2*j+1];
			m = insideparameters[2*j+2];
			temparray1[counter] = l;
			temparray1[counter+1] = m;
			if (m != 0) {
				temparray2[counter] = m;
				temparray2[counter+1] = l;
			} else {
				temparray2[counter] = l;
				temparray2[counter+1] = m;
			}
			counter += 2;
		}

		/* compare two temparrays */
		same = comparearrays(temparray1, temparray2, insideparameters[0]);
		if (same) {
			checkinverse = 1;
		}
		free(temparray1);
		free(temparray2);
	}
}

void init_isovectors() {
	int i, j, counter;
	isovectors = malloc(2*insideparameters[0]*insideparameters[0]*sizeof(unsigned char));
	isovectors[0] = 0;
	for(i = 0; i < insideparameters[0]; i++) {
		for (j = i + 1; j < insideparameters[0]; j++) {
			if (insideparameters[2*i+1] == insideparameters[2*j+1] && insideparameters[2*i+2] == insideparameters[2*j+2]) {
				/* Same parameters */
				counter = isovectors[0];
				isovectors[2*counter+1] = 1 << i;
				isovectors[2*counter+2] = 1 << j;
				isovectors[0] += 1;
			}
		}
	}
}

struct edge* searchDangling(struct td_patch* patch, int nr) {
	struct edge** firstarray;
	int* stack;
	int stackindex = 0;
	int vertexnr, nextindex = 0;
	struct edge *cedge, *start;

	firstarray = calloc((patch->nrofvertices + 1), sizeof(struct edge*));
	firstarray[0] = patch->firstedge; //this can be any edge
	stack = malloc((patch->nrofvertices+1)*sizeof(int));

	stack[0] = patch->firstedge->start;
	firstarray[stack[0]] = patch->firstedge;

	cedge = patch->firstedge;
	while (cedge->start != nr) {
		start = firstarray[stack[stackindex]];
		cedge = start;

		do {
			if (cedge->end != 0) {
				vertexnr = cedge->inv->start;
				if (firstarray[vertexnr] == NULL) {
					firstarray[vertexnr] = cedge->inv;
					nextindex++;
					stack[nextindex] = vertexnr;
					//printf("B%d\n", nextindex);

				}
			}
			cedge = cedge->next;
		} while (cedge != start);
		stackindex++;
	}

	while (cedge->end != 0) {
		cedge = cedge->next;
	}
	return cedge;

}

/*
	MAIN METHODS
*/

void run_topdown(unsigned char pent, unsigned char hex, unsigned char hept, int iv, int msec, int rings, unsigned char exactfaces) {
	int i, j, k, counter, P;
	int internalvertices, insideborderfinished;
	struct td_patch *patch;
	struct edge* previous;
	struct edge* current;
	
	RINGSTOADD = rings;
	EXACTFACES = exactfaces;
	FOUND_JOINS = 0;
	ISO_JOINS = 0;

	/* Initiating gloal variables */
	internalvertices = iv;

	init_isomorphism();
	init_isovectors();

	insideborderfinished = 1;
	for (i=1; i<= insidenanocaps;i++) {
		insideborderfinished *= 2;
	}
	if (internalvertices < 0) {
		free(outsideparameters);
		free(insideparameters);
		//printf("Found 0 Joins\n");
		exit(EXIT_SUCCESS);
	}

	insideborderfinished--;


	P = (pent + 1) * (hex + 1) * (hept + 1) + 1;

	//initialize indextranslate and faceswithmergepaths
	counter = 0;
	indextranslate = malloc((pent + 1) * sizeof(int**));
	for (i = 0; i <= pent; i++) {
		indextranslate[i] = malloc((hex + 1) * sizeof(int*));
		for (j = 0; j <= hex; j++) {
			indextranslate[i][j] = malloc((hept + 1) * sizeof(int));
			for (k = 0; k <= hept; k++) {
				indextranslate[i][j][k] = counter++;
			}
		}
	}

	//initialize statistics
	statistics = malloc(P*sizeof(int));
	for (i=0; i < P; i++) {
		statistics[i] = 0;
	}
	
	/* Initiating start patch */
	patch = malloc(sizeof(struct td_patch));
	patch->nrofvertices = 2;
	patch->maxinternalvertices = internalvertices;
	patch->facesleft = malloc(4*sizeof(int));
	patch->facesleft[0] = pent + hex + hept;
	patch->facesleft[1] = pent;
	patch->facesleft[2] = hex;
	patch->facesleft[3] = hept;
	patch->facesused = malloc(3*sizeof(int));
	patch->facesused[0] = patch->facesused[1] = patch->facesused[2] = 0;
	patch->firstedge = malloc(sizeof(struct edge));
	patch->firstedge->inv = malloc(sizeof(struct edge));

	
	/* Create cycle corresponding to outer face */
	patch->firstedge->inv->inv = patch->firstedge;
	patch->firstedge->start = patch->firstedge->inv->end = 1;
	patch->firstedge->end = patch->firstedge->inv->start = 2;
	patch->firstedge->next = patch->firstedge->prev = patch->firstedge;
	patch->firstedge->inv->next = patch->firstedge->inv->prev = patch->firstedge->inv;
	previous = patch->firstedge;
	
	for(i=0; i < 2*(outsideparameters[0] + outsideparameters[1])-2; i++) {
		addnewvertex(patch, previous->inv);
		previous = previous->inv->next;
	}

	/* Close the cycle */
	addedge(patch->firstedge, previous->inv);

	current = patch->firstedge;

	/* Add dangling for l part */
	for(i=0; i < 2*outsideparameters[0]; i++) {
		if (i % 2 == 0) {
			adddangling(current);
		} else {
			adddangling(current->prev);
		}
		current = getNextInFace(current);
	}

	/* Add dangling for m part */
	for (i=0; i < 2*outsideparameters[1]; i++) {
		if (i % 2 == 0) {
			adddangling(current->prev);
		} else {
			adddangling(current);
		}
		current = getNextInFace(current);
	}
	
	/* Initiate first uface */
	patch->ufaces = malloc(sizeof(struct ufaces));
	patch->ufaces->current = patch->firstedge;
	patch->toborderbuilt = insideborderfinished;
	patch->ufaces->toborderbuiltnr = insidenanocaps;
	patch->ufaces->maxedge = cannonical_edge_old(patch, patch->ufaces->current, 0);
	patch->ufaces->minedge = cannonical_edge_old(patch, patch->ufaces->current, 1);
	patch->ufaces->next = NULL;
	patch->ufaces->faceleftres = 0;
	patch->ufaces->pentres = 0;
	patch->ufaces->hexres = 0;
	patch->ufaces->heptres = 0;
	patch->ufaces->ivres = 0;
	
	/* Set mark - needed for isomorphism */
	patch->mark = cannonical_edge_old(patch, patch->firstedge->inv, 0);
	
	/* Start algorithm */
	prepareWrite(pent, hex, hept);
	clock_t start = clock(), diff;

	dfs(patch);

	diff = clock() - start;
	msec += diff * 1000 / CLOCKS_PER_SEC;
	

	/* Print information */
	/*
	printf("Total %3d (%d)\t Joins IN %d s %d ms\n", FOUND_JOINS, ALL_JOINS, msec/1000, msec % 1000);
	
	printf("-----------------------------------\n");
	for (i=0; i <= pent; i++) {
		for (j=0; j<= hex; j++) {
			for (k=0; k <= hept; k++) {
				if (statistics[indextranslate[i][j][k]] != 0) {
					fprintf(stdout, "Found %3d Joins with %2d pentagons, %2d hexagons, %2d heptagons\n", statistics[indextranslate[i][j][k]], i, j, k);
				}
			}
		}
	}
	printf("-----------------------------------\n");*/
	free(patch->ufaces);
	free(patch->facesleft);
	free(patch->facesused);

	/* Free dangling edges */
	current = patch->firstedge;
	for(i=0; i < 2*(outsideparameters[0] + outsideparameters[1]); i++) {
		if (current->prev->end == 0) {
			free(current->prev);
			current->prev = current->next;
			current->prev->next = current;
		} else if (current->next->end == 0) {
			free(current->next);
			current->next = current->prev;
			current->next->prev = current;
		}
		current = getNextInFace(current);
	}


	/* Free normal edges */
	previous = patch->firstedge;
	for(i=0; i < 2*(outsideparameters[0] + outsideparameters[1]) - 1; i++) {
		current = getNextInFace(previous);
		free(previous->inv);
		free(previous);
		previous = current;
	}
	free(previous->inv);
	free(previous);
	free(patch);

	for (i = 0; i <= pent; i++) {
		for (j = 0; j <= hex; j++) {
			free(indextranslate[i][j]);
		}
		free(indextranslate[i]);
	}

	free(indextranslate);
	free(insideparameters);
	free(outsideparameters);

	finishUp(output);

}
