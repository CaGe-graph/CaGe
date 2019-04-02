#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "td_common.h"
#include "td_bitvector.h"
#include "td_graphutils.h"
#include "../layer.h"

/*
	ADDING / REMOVING
*/

struct edge* adddangling(struct edge* edgebefore) {
	struct edge* newedge;
	newedge = malloc(sizeof(struct edge));
	newedge->start = edgebefore->start;
	newedge->end = 0;
	newedge->inv = NULL;
	
	newedge->next = edgebefore->next;
	newedge->prev = edgebefore;
	edgebefore->next->prev = newedge;
	edgebefore->next = newedge;
	return newedge;
}

struct edge* addnewvertex(struct td_patch* patch, struct edge* edgebefore) {
	struct edge* newedge;
	struct edge* newedgeinv;
	newedge = malloc(sizeof(struct edge));
	newedgeinv = malloc(sizeof(struct edge));

	newedge->inv = newedgeinv;
	newedgeinv->inv = newedge;

	newedge->start = newedge->inv->end = edgebefore->start;
	newedge->end = newedge->inv->start = patch->nrofvertices+1;
	patch->nrofvertices++;

	newedge->next = edgebefore->next;
	newedge->prev = edgebefore;
	edgebefore->next->prev = newedge;
	edgebefore->next = newedge;

	newedge->inv->next = newedge->inv->prev = newedge->inv;
	return newedge;
}

struct edge* addedge(struct edge* edgebefore1, struct edge* edgebefore2) {
	struct edge* newedge;
	struct edge* newedgeinv;
	newedge = malloc(sizeof(struct edge));
	newedgeinv = malloc(sizeof(struct edge));

	newedge->inv = newedgeinv;
	newedgeinv->inv = newedge;

	newedge->start = newedge->inv->end = edgebefore1->start;
	newedge->end = newedge->inv->start = edgebefore2->start;

	newedge->next = edgebefore1->next;
	edgebefore1->next->prev = newedge;
	newedge->prev = edgebefore1;
	edgebefore1->next = newedge;

	newedge->inv->next = edgebefore2->next;
	edgebefore2->next->prev = newedge->inv;
	newedge->inv->prev = edgebefore2;
	edgebefore2->next = newedge->inv;
	return newedge;
}

int endvertex_degree(struct edge* edge) {
	int degree = 3;
	if (edge->inv->next->end == 0) {
		degree = 2;
	}
	return degree;
}

struct edge* getNextInFace(struct edge* sedge) {
	struct edge *redge;
	redge = sedge->inv->next;
	if (redge->end == 0) {
		redge = redge->next;
	}
	return redge;
}

struct edge* getPreviousInFace(struct edge* sedge) {
	struct edge *redge;
	redge = sedge->prev;
	if (redge->end == 0) {
		redge = redge->prev;
	}
	return redge->inv;
}

/*
	BORDERCODE -> Returns False if there is a vertex that occurs more than once in the border
*/
unsigned char get_bordercode(struct edge *startedge, int nrofvertices, unsigned char **bordercode) {
	unsigned char simple, *seen;
	int i;
	struct edge *current;
	simple = 1;
	(*bordercode) = malloc(2*(nrofvertices+1)*sizeof(unsigned char));
	seen = calloc(nrofvertices+1, sizeof(unsigned char));
	//transform face to bordercode
	current =startedge;
	i = 1;
	(*bordercode)[i] = endvertex_degree(current)-2;
	current = getNextInFace(current);
	i += 1;
	while (current != startedge) {
		if (seen[current->end] != 0) {
			simple = 0;
		}
		seen[current->end] = 1;
		(*bordercode)[i] = endvertex_degree(current)-2;
		current = getNextInFace(current);
		i += 1;
	}
	(*bordercode)[0] = i - 1;
	free(seen);
	return simple;
}

unsigned char isValidUface(struct td_patch* patch, struct ufaces *uface, int **arr) {
	unsigned char valid, *bordercode, simple;
	int nrtwo, nrthree, i, size, last, doubles;
	struct edge *can;

	can = uface->minedge;
	valid = 1;
	simple = get_bordercode(can, 2*patch->nrofvertices, &bordercode);
	last = -1;
	doubles = 0;
	/* COUNT NUMBER OF TWOS */
	nrtwo = 0;
	nrthree = 0;
	for (i=1; i <= bordercode[0]; i++) {
		if (bordercode[i] == 0) {
			nrtwo++;
			if (last == 0) {
				doubles = 1;
			}
			last = 0;
		} else {
			nrthree++;
			last = 1;
		}
	}
	size = bordercode[0];
	(*arr)[0] = 0;
	(*arr)[1] = 0;
	(*arr)[2] = 0;
	(*arr)[3] = 0;

	if (nrtwo == 0 && (size < 5 || size > 7 || patch->facesleft[size - 4] == 0)) {
		valid = 0;
	}
	if (simple && uface->toborderbuilt == 0 && (nrthree - nrtwo < 6 - patch->facesleft[1] || nrthree -nrtwo > 6 + patch->facesleft[3])) {
		valid = 0;
	} else if (uface->toborderbuilt == 0) {
		if (doubles == 0 && nrthree > nrtwo 
				&& ((patch->facesleft[1] <= spent && patch->facesleft[2] <= shex && patch->facesleft[3] <= shept)
					|| (patch->facesleft[0] + 1 <= spent && patch->facesleft[0] + 1 <= shex && patch->facesleft[0] + 1 <= shept))) {
			valid = tree_lookup(bordercode, arr);
		} else {
			if (nrthree -nrtwo > 6) {	
				(*arr)[0] = 0;
				(*arr)[2] = (nrthree - nrtwo) - 6;
			} else {
				(*arr)[0] = 6 - (nrthree -nrtwo);
				(*arr)[2] = 0;
			}
		}
	}
	if (nrtwo == 1 && patch->maxinternalvertices < 3) {
		valid = 0;
	}
	free(bordercode);
	return valid;
}

/* Checks if startvertex of first is connected with startvertex from second */
unsigned char areConnected(struct edge *first, struct edge *second) {
	unsigned char connected;
	struct edge *temp;
	connected = 0;
	if (first->end != second->start) {
		temp = first->next;
		while(temp != first) {
			if (temp->end == second->start) {
				connected = 1;
			}
			temp = temp->next;
		}
	} else {
		connected = 1;
	}
	return connected;
}

/*
	MAXMIN: 0 for max, 1 for min
*/
void cannonical_edge(struct ufaces *face) {
	struct bitvector *border, *valuemax, *valuemin;
	unsigned long long bordernumber, maxbitnumber, valuenumbermax, valuenumbermin;
	struct edge *current, *result;
	int length, i, indexmax, indexmin;
	char cmax, cmin;
	current = face->current;

	if (endvertex_degree(face->current) == 3) {
		bordernumber = 1;
	} else {
		bordernumber = 0;
	}

	length = 1;
	current = getNextInFace(current);
	while (current != face->current) {
		if (length == 64) {
			border = malloc(sizeof(struct bitvector));
			border->lastused = 0;
			border->bits[0] = bordernumber;
			border->maxbit = lastbit;
			if (endvertex_degree(current) == 3) {
				bit_add(border, 1);
			} else {
				bit_add(border, 0);
			}
		} else if (length < 64) {
			bordernumber = bordernumber << 1;
			if (endvertex_degree(current) == 3) {
				bordernumber += 1;
			}
		} else if (length < 640) {
			if (endvertex_degree(current) == 3) {
				bit_add(border, 1);
			} else {
				bit_add(border, 0);
			}
		} else {
			fprintf(stderr, "Border too long, exiting, change bitvector size\n");
			exit(EXIT_FAILURE);
		}

		length += 1;
		current = getNextInFace(current);
	}
	
	indexmax = 0;
	indexmin = 0;

	if (length > 64) {
		valuemax = malloc(sizeof(struct bitvector));
		valuemin = malloc(sizeof(struct bitvector));
		memcpy(valuemax, border, sizeof(struct bitvector));
		memcpy(valuemin, border, sizeof(struct bitvector));
		for (i=1; i < length; i++) {
			bit_rotate(border);

			cmax = bit_compare(border, valuemax);
			if (cmax == 1) {
				memcpy(valuemax, border, sizeof(struct bitvector));
				indexmax = i;
			}

			cmin = bit_compare(border, valuemin);
			if (cmin == -1) {
				memcpy(valuemin, border, sizeof(struct bitvector));
				indexmin = i;
			}

		}
		free(border);
		free(valuemax);
		free(valuemin);
	} else {
		maxbitnumber = ((unsigned long long) 1) << (length - 1);
		valuenumbermax = bordernumber;
		valuenumbermin = bordernumber;
		for (i=1; i< length; i++) {
			if (bordernumber % 2 == 1) {
				bordernumber = (bordernumber >> 1) | maxbitnumber;
			} else {
				bordernumber = (bordernumber >> 1);
			}
			if (bordernumber > valuenumbermax) {
				valuenumbermax = bordernumber;
				indexmax = i;
			}
			if (bordernumber < valuenumbermin) {
				valuenumbermin = bordernumber;
				indexmin = i;
			}
		}
	}

	result = face->current;
	for (i=0; i < indexmax; i++) {
		result = getPreviousInFace(result);
	}
	face->maxedge = result;
	
	result = face->current;
	for (i=0; i < indexmin; i++) {
		result = getPreviousInFace(result);
	}
	face->minedge = result;
}

/*
	MAXMIN: 0 for max, 1 for min
*/
struct edge* cannonical_edge_simple(struct td_patch* patch, struct edge* startedge, unsigned char maxmin) {
	unsigned long long maxbit, value, border;
	int length, i, index;
	struct edge *current, *result;
	current = startedge;
	border = 0;
	if (endvertex_degree(startedge) == 3) {
		border = 1;
	}
	length = 1;
	current = getNextInFace(current);
	while (current != startedge) {
		border = border << 1;
		if (endvertex_degree(current) == 3) {
			border += 1;
		}
		length += 1;
		current = getNextInFace(current);
	}
	if (length > 63) {
		return cannonical_edge_simple_large(patch, startedge, maxmin);
	}

	result = startedge;
	maxbit = ((unsigned long long) 1) << (length - 1);
	index = 0;
	value = border;
	for (i=1; i< length; i++) {
		if (border % 2 == 1) {
			border = (border >> 1) | maxbit;
		} else {
			border = (border >> 1);
		}
		if ((!maxmin && border > value) || (maxmin && border < value)) {
			value = border;
			index = i;
		}
	}

	for (i=0; i< index; i++) {
		result = getPreviousInFace(result);
	}
	return result;
}

struct edge* cannonical_edge_simple_large(struct td_patch* patch, struct edge* startedge, unsigned char maxmin) {
	struct edge *result;
	int i, j, k, size;
	unsigned char* bordercode;
	unsigned char found, ismaxmin;
	//transform face to bordercode
	get_bordercode(startedge, 2*(patch->nrofvertices+1), &bordercode);
	size = bordercode[0];
	ismaxmin = 0;
	i = 0;
	while (i < size && !ismaxmin) {
		j = 1;
		found = 0;
		while(!found && j < size) {
			k = 0;
			while(k < size && bordercode[(i+k) % size + 1] == bordercode[(i+j+k) % size + 1]) {
				k++;
			}
			if (k != size && ( 
				(maxmin && bordercode[(i+k) % size + 1] > bordercode[(i+j+k) % size + 1]) ||
				(!maxmin && bordercode[(i+k) % size + 1] < bordercode[(i+j+k) % size + 1]))) {
				i = i+j;
				found = 1;
			}
			j++;
		}
		if (j == size) {
			ismaxmin = 1;
		}
	}
	
	result = startedge;
	for (j=0; j < i; j++) {
		result = getNextInFace(result);
	}
	free(bordercode);
	return result;
}

void print_bordercode(struct edge *startedge, int nrofvertices) {
	unsigned char* bordercode;
	int i;
	get_bordercode(startedge, nrofvertices, &bordercode);
	printf("%d %d\n", startedge->start, startedge->end);
	for (i=0; i <= bordercode[0]; i++) {
		printf("%d ", bordercode[i]);
	}
	printf("\n");
	free(bordercode);
}
