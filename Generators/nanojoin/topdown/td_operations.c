#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "td_common.h"
#include "td_graphutils.h"
#include "td_common.h"
#include "td_nanojoin.h"
#include "td_patchAdjacency.h"

int newUface(struct td_patch* patch, struct edge* edge, int toborderbuilt1nr, int toborderbuilt2nr);
void mergeUface(struct td_patch* patch, struct edge *old);
void createSpecialFace(struct td_patch *patch, struct edge *from, int nrofedges, int lparameter, int mparameter, int offset);
void createPathCicle(struct td_patch* patch, struct edge *from, int pathlength, int cyclelength);
void connect(struct td_patch* patch, struct edge* from, struct edge* to, int nrofedges, unsigned char filling);

int newUface(struct td_patch* patch, struct edge* edge, int toborderbuiltnr1, int toborderbuiltnr2) {
	struct ufaces* newuface;
	int* arr;
	int p, s, h, i, f;
	unsigned char valid;
	newuface = malloc(sizeof(struct ufaces));
	newuface->current = edge->inv;
	newuface->next = patch->ufaces;
	newuface->toborderbuiltnr= toborderbuiltnr2;
	newuface->faceleftres = 0;
	newuface->pentres = 0;
	newuface->hexres = 0;
	newuface->heptres = 0;
	newuface->ivres = 0;
	newuface->maxres = patch->ufaces->maxedge;
	newuface->minres = patch->ufaces->minedge;
	patch->ufaces->current = edge;
	patch->ufaces->toborderbuiltnr = toborderbuiltnr1;
	patch->ufaces = newuface;
	//printf("%d %d: %d\n", newuface->current->start, newuface->current->end, newuface->toborderbuiltnr);
	//printf("%d %d: %d\n", newuface->next->current->start, newuface->next->current->end, newuface->next->toborderbuiltnr);
	/*
		Check if both ufaces are valid ufaces
	*/
	valid = 0;
	arr = malloc(5*sizeof(int));
	//patch->ufaces->maxedge = cannonical_edge(patch, patch->ufaces, 0);
	cannonical_edge(patch->ufaces);
	if (isValidUface(patch, patch->ufaces, &arr)) {
		p = arr[0];
		h = arr[1];
		s = arr[2];
		i = arr[3];
		f = arr[4];
		//patch->ufaces->next->maxedge = cannonical_edge(patch, patch->ufaces->next, 0);
		cannonical_edge(patch->ufaces->next);
		if (isValidUface(patch, patch->ufaces->next, &arr)) {
			valid = 1;
			if (patch->facesleft[1] < p + arr[0] 
					|| patch->facesleft[2] < h + arr[1] 
					|| patch->facesleft[3] < s + arr[2] 
					|| patch->maxinternalvertices < i + arr[3] 
					|| patch->facesleft[0] < f + arr[4])
			{
				valid = 0;
			} else {
				patch->ufaces->next->pentres = arr[0];
				patch->ufaces->next->hexres = arr[1];
				patch->ufaces->next->heptres = arr[2];
				patch->ufaces->next->ivres = arr[3];
				patch->ufaces->next->faceleftres = arr[4];
				patch->facesleft[0] -= arr[4];
				patch->facesleft[1] -= arr[0];
				patch->facesleft[2] -= arr[1];
				patch->facesleft[3] -= arr[2];
				patch->maxinternalvertices -= arr[3];
			}
		}
	} 
	free(arr);
	return valid;
}

void mergeUface(struct td_patch* patch, struct edge *old) {
	struct ufaces* temp;
	patch->ufaces->next->toborderbuiltnr = patch->ufaces->next->toborderbuiltnr + patch->ufaces->toborderbuiltnr;
	temp = patch->ufaces->next;
	temp->maxedge = patch->ufaces->maxres;
	temp->minedge = patch->ufaces->minres;
	free(patch->ufaces);
	patch->ufaces = temp;
	patch->ufaces->current = old;
	patch->facesleft[0] += patch->ufaces->faceleftres;
	patch->facesleft[1] += patch->ufaces->pentres;
	patch->facesleft[2] += patch->ufaces->hexres;
	patch->facesleft[3] += patch->ufaces->heptres;
	patch->maxinternalvertices += patch->ufaces->ivres;
	patch->ufaces->pentres = 0;
	patch->ufaces->heptres = 0;
	patch->ufaces->hexres = 0;
	patch->ufaces->ivres = 0;
	patch->ufaces->faceleftres = 0;
}

unsigned char check_isovectors(unsigned char tobuild, unsigned char willbuild) {
	int i;
	unsigned char first, second, valid;
	valid = 1;
	for (i=0; i < isovectors[0]; i++) {
		first = isovectors[2*i+1];
		second = isovectors[2*i+2];
		/* both not build yet */
		if ((first & tobuild) != 0 && (second & tobuild) != 0) {
			if ((willbuild & first) == 0 && (willbuild & second) != 0) {
				valid = 0;
			}
		}
	}
	return valid;
}

/* make sure inv lies in new uface */
void connect(struct td_patch* patch, struct edge* from, struct edge* to, int nrofedges, unsigned char filling) {
	struct edge *first, *last, *current, *next, *old;
	int i;

	/* Special case since two dangling edges must be merged */
	if (nrofedges == 1) {
		first = malloc(sizeof(struct edge));
		first->inv = malloc(sizeof(struct edge));
		first->start = first->inv->end = from->start;
		first->end = first->inv->start = to->start;

		from->prev->next = from->next->prev = first;
		to->prev->next = to->next->prev = first->inv;
		first->prev = from->prev;
		first->next = from->next;
		first->inv->inv = first;
		first->inv->prev = to->prev;
		first->inv->next = to->next;
	} else {
		first = malloc(sizeof(struct edge));
		first->inv = malloc(sizeof(struct edge));
		last = malloc(sizeof(struct edge));
		last->inv = malloc(sizeof(struct edge));

		/* Vertex numbers */
		first->start = first->inv->end = from->start;
		last->start = last->inv->end = to->start;
		patch->nrofvertices++;
		first->end = first->inv->start = patch->nrofvertices;

		/* Order around vertex */
		from->next->prev = from->prev->next = first;
		to->next->prev = to->prev->next = last;
		first->prev = from->prev;
		first->inv->inv = first;
		first->next = from->next;
		last->prev = to->prev;
		last->next = to->next;
		first->inv->prev = first->inv->next = first->inv;

		/* Adding new edges */
		current = first->inv;
		for (i=2; i < nrofedges; i++) {
			current = addnewvertex(patch, current);
			current = current->inv;
		}
		last->end = last->inv->start = patch->nrofvertices;
		last->inv->inv = last;
		last->inv->prev = last->inv->next = current;
		current->prev = current->next = last->inv;

		/* Add dangling edges */
		current = first;
		for (i=0; i < nrofedges -1; i++) {
			if (i % 2 == 0 || filling) {
				adddangling(current->inv);
			} else {
				adddangling(current->inv->next);
			}
			current = getNextInFace(current);
		}
	}
	/*
	maxpower = 1 << insidenanocaps;
	maxpower -= 1;
	for (i=0; i <= maxpower; i++) {
		if ((unsigned char) (patch->ufaces->toborderbuilt | ~i) == 255 && check_isovectors(patch->ufaces->toborderbuilt, i)) {
			old = patch->ufaces->current;
			if (newUface(patch, first->inv, i, patch->ufaces->toborderbuilt ^ i)) {
				dfs(patch);
			}
			mergeUface(patch, old);
		}
	}*/

	for (i=0; i <= patch->ufaces->toborderbuiltnr; i++) {
		old = patch->ufaces->current;
		if (newUface(patch, first->inv, i, patch->ufaces->toborderbuiltnr - i)) {
			dfs(patch);
		}
		mergeUface(patch, old);
	}
	
	if (nrofedges == 1) {
		free(first->inv);
		free(first);
		from->prev->next = from;
		from->next->prev = from;
		to->next->prev = to;
		to->prev->next = to;
	} else {
		/* Free dangling edge */
		current = first;
		for (i=0; i < nrofedges -1; i++) {
			if (i % 2 == 0 || filling) {
				free(current->inv->next);
				current->inv->next = current->inv->prev;
				current->inv->next->prev = current->inv;
			} else {
				free(current->inv->prev);
				current->inv->prev = current->inv->next;
				current->inv->prev->next = current->inv;
			}
			current = getNextInFace(current);
		}
		current = first;
		next = getNextInFace(current);
		/* Free added normal edges */
		for (i=0; i < nrofedges; i++) {
			free(current->inv);
			free(current);
			current = next;
			next = getNextInFace(current);
		}
		/* Reset edges around start and end point */
		from->prev->next = from->next->prev = from;
		to->next->prev = to->prev->next = to;
		patch->nrofvertices -= (nrofedges-1);
	}

}

void createPathCicle(struct td_patch* patch, struct edge *from, int pathlength, int cyclelength) {
	struct edge *first, *current, *last, *old;
	int i, r;

	first = malloc(sizeof(struct edge));
	first->inv = malloc(sizeof(struct edge));
	first->inv->inv = first;
	first->start = first->inv->end = from->start;
	patch->nrofvertices++;
	first->end = first->inv->start = patch->nrofvertices;
	first->inv->next = first->inv->prev = first->inv;
	first->prev = from->prev;
	first->next = from->next;
	first->next->prev = first;
	first->prev->next = first;
	current = first;

	/* ADDING PATH */
	for(i=1; i < pathlength; i++) {
		current = addnewvertex(patch, current->inv);
	}

	last = current;

	/* ADDING CYLCE */
	for (i=0; i < cyclelength - 1; i++) {
		current = addnewvertex(patch, current->inv);
	}
	if (pathlength % 2 == 0) {
		current = addedge(current->inv, last->inv->next);
	} else {
		current = addedge(current->inv, last->inv);
	}

	current = first;
	for(i=1; i < pathlength; i++) {
		if (i % 2 == 1) {
			adddangling(current->inv);
		} else {
			adddangling(current->inv->next);
		}
		current = getNextInFace(current);
	}

	if (pathlength % 2 == 0) {
		r = 0;
		current = current->inv->next;
	} else {
		current = current->inv->prev;
		r = 1;
	}

	for(i=1; i < cyclelength; i++) {
		if ((i + r) % 2 == 1) {
			adddangling(current->inv);
		} else {
			adddangling(current->inv->next);
		}
		current = getNextInFace(current);
	}


	/*
	maxpower = 1 << insidenanocaps;
	maxpower -= 1;
	for (i=0; i <= maxpower; i++) {
		if ((unsigned char) (patch->ufaces->toborderbuilt | ~i) == 255 && check_isovectors(patch->ufaces->toborderbuilt, i)) {
			old = patch->ufaces->current;
			if (newUface(patch, last->inv->next, i, patch->ufaces->toborderbuilt ^ i)) {
				dfs(patch);
			}
			mergeUface(patch, old);
		}
	}*/


	for (i=0; i <= patch->ufaces->toborderbuiltnr; i++) {
		old = patch->ufaces->current;
		if (newUface(patch, last->inv->next, i, patch->ufaces->toborderbuiltnr - i)) {
			dfs(patch);
		} else {
		}
		mergeUface(patch, old);
	}

	/* Freeing edges */
	/* Dangling */
	current = first;
	for(i=0; i < pathlength + cyclelength; i++) {
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

	/* Normal */
	last = first;
	for (i=0; i < pathlength + cyclelength - 1; i++) {
		current = getNextInFace(last);
		free(last->inv);
		free(last);
		last = current;
	}
	free(last->inv);
	free(last);

	/* Reset edges around start and end point */
	from->prev->next = from->next->prev = from;
	patch->nrofvertices -= (pathlength + cyclelength -1);
}

void createSpecialFace(struct td_patch *patch, struct edge *from, int nrofedges, int lparameter, int mparameter, int offset) {
	struct edge *first, *current, *last, *tempmax, *tempmin, *temp;
	int i;
	/* Adding internal edges */

	first = malloc(sizeof(struct edge));
	first->inv = malloc(sizeof(struct edge));
	first->inv->inv = first;
	first->start = first->inv->end = from->start;
	patch->nrofvertices++;
	first->end = first->inv->start = patch->nrofvertices;
	first->inv->next = first->inv->prev = first->inv;
	first->prev = from->prev;
	first->next = from->next;
	first->next->prev = first;
	first->prev->next = first;
	current = first;
	
	for (i = 1; i < nrofedges; i++) {
		current = addnewvertex(patch, current->inv);
	}
	last = current;

	
	/* Adding edges of special face */
	
	for (i=0; i < 2*(lparameter + mparameter) - 1; i++) {
		current = addnewvertex(patch, current->inv);
	}
	current = addedge(current->inv, last->inv);

	/* Adding dangling edges */



	for (i=0; i < 2*offset; i++) {
		current = getNextInFace(current);
	}
	if (offset != 0 && offset <= mparameter) {
		current = getPreviousInFace(current);
	}
	for (i=0; i < lparameter; i++) {
		current = getNextInFace(current);
		if (current->prev == current->next) {
			adddangling(current);
		}
		adddangling(current->inv);
		current = getNextInFace(current);
	}
	for (i=0; i < mparameter; i++) {
		adddangling(current->inv);
		current = getNextInFace(current);
		current = getNextInFace(current);
		if (current->prev == current->next) {
			adddangling(current);
		}
	}

	current = first;
	for(i=1; i < nrofedges; i++) {
		if (i % 2 == 1) {
			adddangling(current->inv);
		} else {
			adddangling(current->inv->next);
		}
		current = getNextInFace(current);
	}
	
	tempmax = patch->ufaces->maxedge;
	tempmin = patch->ufaces->minedge;
	//patch->ufaces->maxedge = cannonical_edge(patch, patch->ufaces, 0);
	temp = patch->ufaces->current;
	patch->ufaces->current = first;
	cannonical_edge(patch->ufaces);
	dfs(patch);
	patch->ufaces->current = temp;
	patch->ufaces->maxedge = tempmax;
	patch->ufaces->minedge = tempmin;


	/* Freeing edges */
	/* Dangling */
	current = first;
	for(i=0; i < 2*(lparameter + mparameter) + nrofedges; i++) {
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

	/* Normal */
	last = first;
	for (i=0; i < 2*(lparameter + mparameter) + nrofedges - 1; i++) {
		current = getNextInFace(last);
		free(last->inv);
		free(last);
		last = current;
	}
	free(last->inv);
	free(last);
	/*Reset edges around start point */
	from->prev->next = from->next->prev = from;
	patch->nrofvertices -= nrofedges + 2*(lparameter + mparameter) -1;
}


/* SPLIT THE CURRENT UFACE IN TWO SEPERATE UFACES */
void split(struct td_patch* patch, struct edge *mark) {
	int i;
	struct edge *startedge, *endedge;
	//startedge = cannonical_edge(patch, patch->ufaces->current, 1);
	startedge = mark;
	endedge = getNextInFace(startedge);

	while(endedge != startedge) {
		if (endvertex_degree(endedge) == 2) {
			for(i=1; i <= patch->maxinternalvertices + 1; i++){
				if (i > 1 || !areConnected(startedge->inv, endedge->inv)) {
					patch->maxinternalvertices -= (i - 1);
					connect(patch, startedge->inv->next, endedge->inv->next, i, 0);
					patch->maxinternalvertices += (i - 1);
				}
			}
		}
		endedge = getNextInFace(endedge);
	}
}


void unfold(struct td_patch* patch, struct edge *mark) {
	int i, j, k, lastl, lastm;
	unsigned char temp, power;

	//mark = cannonical_edge(patch, patch->ufaces->current, 1);
	power = 1;
	lastl = 0;
	lastm = 0;
	patch->ufaces->toborderbuiltnr -= 1;
	for (i=1; i <= 2*insideparameters[0]; i+= 2) {
		/* ADD EXTRA FOR TWO SPECIAL FACES WITH SAME PARAMETERS*/
		if ((unsigned char) (patch->toborderbuilt | ~power) == 255 && (lastl != insideparameters[i] || lastm != insideparameters[i+1])) {
			lastl = insideparameters[i];
			lastm = insideparameters[i+1];
			temp = patch->toborderbuilt;
			patch->toborderbuilt = patch->toborderbuilt & ~power;
			/* Build face of length 2*(insidenanocapborders[i]+insidenanocapborders[i+1]) */
			for (j=1; j <= patch->maxinternalvertices+1; j++) {
				patch->maxinternalvertices -= (j-1);
				if (insideparameters[i+1] != 0) {
					for (k = 0; k < (insideparameters[i] + insideparameters[i+1]); k+=1) {
						createSpecialFace(patch, mark->inv->next, j, insideparameters[i], insideparameters[i+1], k);
					}
				} else {
					createSpecialFace(patch, mark->inv->next, j, insideparameters[i], insideparameters[i+1], 0);
				}

				patch->maxinternalvertices += (j-1);
			}
			patch->toborderbuilt = temp;
			
		}
		power *= 2;
	}
	patch->ufaces->toborderbuiltnr += 1;
}

void unwrap(struct td_patch* patch, struct edge *mark) {
	int totallength, pathlength, cyclelength;

	//mark = cannonical_edge(patch, patch->ufaces->current, 1);
	for (totallength = 4; totallength <= patch->maxinternalvertices + 1; totallength += 1) {
		for (pathlength = 1; pathlength < totallength - 3; pathlength += 1) {
			patch->maxinternalvertices -= (totallength-1);
			cyclelength = totallength - pathlength;
			createPathCicle(patch, mark->inv->next, pathlength, cyclelength);
			patch->maxinternalvertices += (totallength-1);
		}
	}
}

unsigned char fill(struct td_patch *patch) {
	struct edge *startedge, *endedge;
	int length;
	unsigned char split;
	split = 1;
	startedge = patch->ufaces->maxedge;

	endedge = startedge;
	length = 1;
	while(endvertex_degree(endedge) == 3 && getNextInFace(endedge) != getPreviousInFace(startedge)) {
		endedge = getNextInFace(endedge);
		length += 1;
	}
	endedge = getNextInFace(endedge);
	length += 1;
	/* Length = number of vertices */
	if (getNextInFace(endedge) != startedge) {
		startedge = startedge->prev;
		endedge = endedge->prev;
		
		if (length > 7) {
			split = 0;
		}

		/* 7-gon */
		else if (length == 7) {
			split = 0;
			if (patch->facesleft[3] != 0 && !areConnected(startedge, endedge)) {
				connect(patch, startedge, endedge, 1, 1);
			}
		} 

		/* 7 or 6-gon */
		else if (length == 6) {
			split = 0;
			if (patch->facesleft[3] != 0 && patch->maxinternalvertices >= 1) {
				patch->maxinternalvertices -= 1;
				connect(patch, startedge, endedge, 2, 1);
				patch->maxinternalvertices += 1;
			}
			if (patch->facesleft[2] != 0 && !areConnected(startedge, endedge)) {
				connect(patch, startedge, endedge, 1, 1);
			}
		} 


		/* 6 or 5-gon */
		else if (length == 5 && patch->facesleft[3] == 0) {
			split = 0;
			if (patch->facesleft[2] != 0 && patch->maxinternalvertices >= 1) {
				patch->maxinternalvertices -= 1;
				connect(patch, startedge, endedge, 2, 1);
				patch->maxinternalvertices += 1;
			}
			if (patch->facesleft[1] != 0 && !areConnected(startedge, endedge)) {
				connect(patch, startedge, endedge, 1, 1);
			}
		} 

		/* Only 5-gon */
		else if (length == 4 && patch->facesleft[2] == 0 && patch->facesleft[3] == 0) {
			split = 0;
			if (patch->facesleft[1] != 0 && patch->maxinternalvertices >= 1) {
				patch->maxinternalvertices -= 1;
				connect(patch, startedge, endedge, 2, 1);
				patch->maxinternalvertices += 1;
			}
		} 
	}
	return split;
}
