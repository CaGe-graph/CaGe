/*
 * isomorphismcheck.c
 *
 *  Created on: Mar 15, 2013
 *      Author: Dieter
 */

#include <stdlib.h>
#include <string.h>
#include "td_isomorphismcheck.h"

 /* PREFIX TREE */

struct prefixelement {
    vertextype element;
    struct prefixelement *child;
    struct prefixelement *next;
};

struct prefixelement* prefixroot;

typedef struct {
    unsigned char firstparam;
    unsigned char secondparam;

    int maxvertexnum;
    int length;
    struct edge** looseedges;
    unsigned char picked;
} tube;


unsigned char findPrefix(vertextype* code, int length);
void addPrefix(vertextype* code, int length);

/* TUBES */
int maxtubelength;

tube** tubes;
tube *buildTubes(int firstparam, int secondparam, int length, int* currentvertexnum);

void addTube(tube* t, tube* join);
void removeTube(tube* tube);


/* PREFIX TREE IMPLEMENTATION */
struct edge* getNextWithSpecial(struct edge* edge) {
	if (edge->inv) {
		return edge->inv->next;
	} else {
		return edge->next;
	}
}

unsigned char findPrefix(vertextype* code, int length) {
	struct prefixelement *current;
	int cindex;

	current = prefixroot;
	cindex = 0;

	/* empty tree */
	if (!current->child) {
		return 0;
	}

	/* will stop when we have processed all vertices (code is prefix) or reach an endnode (prefix in tree) */
	while (cindex < length && current->child) {
		current = current->child;
		while (current->element < code[cindex] && current->next) {
			current = current->next;
		}

		/* prefix not in tree */
		if (current->element != code[cindex]) {
			return 0;
		}

		cindex++;
	}

	if (cindex <= length) {
		return 1;
	} else {
		/* this one is shorter */
		return 2;
	}
}

void freeNode(struct prefixelement *element) {
	if (element->child == NULL && element->next == NULL) {
		free(element);
	} else if (element->next == NULL) {
		freeNode(element->child);
		free(element);
	} else {
		freeNode(element->next);
		freeNode(element->child);
		free(element);
	}
}

void addPrefix(vertextype* code, int length) {
	struct prefixelement *current, *temp, *previous;
	int cindex;

	current = prefixroot;
	cindex = 0;

	while (cindex < length) {

		/* create new child node with current as parent */
		if (!current->child) {
			current->child = malloc(sizeof(struct prefixelement));
			current->child->element = code[cindex];
			current->child->next = NULL;
			current->child->child = NULL;
			current = current->child;
		} else {
			previous = current;
			current = current->child;

			/* find needed element */
			while (current->element < code[cindex] && current->next) {
				previous = current;
				current = current->next;
			}

			/*create new element if necessary */
			if (current->element != code[cindex]) {
				temp = current;
				current = malloc(sizeof(struct prefixelement));
				current->element = code[cindex];
				current->child = NULL;

				if (current->element < temp->element) {
					current->next = temp;
					if (previous->child == current->next) {
						previous->child = current;
					} else {
						previous->next = current;
					}
				} else {
					current->next = temp->next;
					temp->next = current;
				}
			}
		}

		cindex++;
	}

	/* Take shorter in tree */
	if (current->child != NULL) {
		freeNode(current->child);
		current->child = NULL;
	}
}

/* TUBES IMPLEMENTATION */

void addTube(tube* mytube, tube* join) {
	int i;
	struct edge *t, *j;
	for (i=0; i < mytube->length; i++) {
		t = mytube->looseedges[i];
		j = join->looseedges[i];

		j->inv = t;
		t->inv = j;
		j->end = t->start;
		t->end = j->start;
	}
}

void removeTube(tube* tube) {
	int i;
	struct edge *t, *j;
	for (i=0; i < tube->length; i++) {
		t = tube->looseedges[i];
		j = t->inv;

		j->end = 0;
		j->inv = NULL;

		t->inv = NULL;
		t->end = 0;
	}
}

tube* buildTubes(int firstparam, int secondparam, int length, int* currentvertexnum) {
	tube* mytube;
	struct edge** looseedges;
	struct edge **templooseedges, **templooseedges2;
	struct edge* currentreal;
	struct edge* currentrealinv;
	struct edge* prevrealinv;
	int i, j;
	int counter = 0;

	mytube = malloc(sizeof(tube));
	mytube->firstparam = firstparam;
	mytube->secondparam = secondparam;
	mytube->length = firstparam + secondparam;
	mytube->looseedges = malloc((firstparam+secondparam)*sizeof(struct edge*));
	looseedges = mytube->looseedges;

	templooseedges2 = NULL;
	templooseedges = malloc((firstparam+secondparam)*sizeof(struct edge*));

	for (j=0; j <= length; j++) {
		prevrealinv = NULL;
		/* build ring two edges at a time */
		for (i=0; i < firstparam + secondparam; i++) {
			/* create first edge and loose edges */
			currentreal = malloc(sizeof(struct edge));
			currentrealinv = malloc(sizeof(struct edge));

			currentreal->start = currentrealinv->end = (*currentvertexnum);
			currentreal->end = currentrealinv->start = (*currentvertexnum)+1;
			currentreal->inv = currentrealinv;
			currentrealinv->inv = currentreal;

			looseedges[counter] = malloc(sizeof(struct edge));
			templooseedges[counter] = malloc(sizeof(struct edge));
			looseedges[counter]->end = templooseedges[counter]->end = 0;


			/* order around first edge */
			if (i < firstparam) {
				looseedges[counter]->start = (*currentvertexnum);
				templooseedges[counter]->start = (*currentvertexnum)+1;

				currentreal->next = looseedges[counter];
				currentrealinv->next = templooseedges[counter];

				looseedges[counter]->prev = currentreal;
				looseedges[counter]->next = prevrealinv;
				templooseedges[counter]->prev = currentrealinv;
			} else {
				looseedges[counter]->start = (*currentvertexnum)+1;
				templooseedges[counter]->start = (*currentvertexnum);

				currentreal->prev = templooseedges[counter];
				currentrealinv->prev = looseedges[counter];

				templooseedges[counter]->next = currentreal;
				templooseedges[counter]->prev = prevrealinv;
				looseedges[counter]->next = currentrealinv;
			}

			/* connect to previous edge */
			if (prevrealinv != NULL) {

				if (i < firstparam) {
					currentreal->prev = prevrealinv;
					prevrealinv->next = currentreal;
					prevrealinv->prev = looseedges[counter];
				} else {
					currentreal->next = prevrealinv;
					prevrealinv->prev = currentreal;
					prevrealinv->next = templooseedges[counter];
				}

			}
			/* create second edge*/
			currentreal = malloc(sizeof(struct edge));
			currentreal->start = (*currentvertexnum)+1;
			currentreal->end = (*currentvertexnum)+2;

			/* order around second edge */
			if (i < firstparam) {
				templooseedges[counter]->next = currentreal;
				currentreal->next = currentrealinv;
				currentreal->prev = templooseedges[counter];
				currentrealinv->prev = currentreal;
			} else {
				looseedges[counter]->prev = currentreal;
				currentreal->prev = currentrealinv;
				currentreal->next = looseedges[counter];
				currentrealinv->next = currentreal;
			}

			currentrealinv = malloc(sizeof(struct edge));
			currentreal->inv = currentrealinv;
			currentrealinv->inv = currentreal;

			currentrealinv->end = (*currentvertexnum)+1;
			currentrealinv->start = (*currentvertexnum)+2;

			/* Go to next edges */
			counter += 1;
			(*currentvertexnum) += 2;
			prevrealinv = currentrealinv;
		}
		/* close ring */
		currentreal = looseedges[0]->prev;
		prevrealinv->start = prevrealinv->inv->end = currentreal->start;
		prevrealinv->next = currentreal;
		prevrealinv->prev = looseedges[0];

		looseedges[0]->next = prevrealinv;
		currentreal->prev = prevrealinv;

		/* connect ring */
		if (templooseedges2 != NULL) {
			
			for (i=0; i < firstparam + secondparam; i++) {
				templooseedges2[i]->inv = looseedges[i];
				looseedges[i]->inv = templooseedges2[i];
				templooseedges2[i]->end = looseedges[i]->start;
				looseedges[i]->end = templooseedges2[i]->start;
			}
			free(looseedges);
			free(templooseedges2);
		}
		templooseedges2 = templooseedges;
		looseedges = malloc((firstparam+secondparam)*sizeof(struct edge*));
		templooseedges = malloc((firstparam+secondparam)*sizeof(struct edge*));
		counter = 0;
	}

	free(looseedges);
	free(templooseedges2);

	mytube->maxvertexnum = *currentvertexnum - 1;
	return mytube;
}


/* ISOMORPHISM IMPLEMENTATION */

void prepareIsomorphism(int maxvertices) {
	int i, currentvertex;

	currentvertex = maxvertices;
	tubes = malloc(nrofnanocaps*sizeof(tube*));
	tubes[0] = buildTubes(outsideparameters[0], outsideparameters[1], maxvertices, &currentvertex);
	for (i=0; i <nrofnanocaps-1; i++) {
		tubes[i+1] = buildTubes(insideparameters[2*i+1], insideparameters[2*i+2], maxvertices, &currentvertex);
	}

	prefixroot = malloc(sizeof(struct prefixelement));
	prefixroot->child = NULL;
	prefixroot->next = NULL;
}

void freeGraphEdge(struct edge* startedge) {
	return;
	struct edge* current;
	startedge->start = 0;
	current = startedge->next;
	while (current != startedge) {
		if (current->end != 0) {
			freeGraphEdge(current->inv);
		}
		free(current);
	}

	if (current->start != 0) {
		freeGraphEdge(startedge->inv);
	}
	free(current);
}

void finishUpIsomorphism() {
	int i;
	freeNode(prefixroot);
	for (i=0; i < nrofnanocaps; i++) {
		freeGraphEdge(tubes[i]->looseedges[0]);
		free(tubes[i]);
	}
	free(tubes);
}

void getSpecialEdges(struct td_patch* patch, struct edge*** edges, tube*** jointtubes, int size) {
	int i, facecounter, arraycounter, arraycounter2;
	int nrofedges;
	struct edge** vertexes;
	struct edge *cedge, *fedge;

	unsigned char *visitedloose;

	unsigned char tubenr = 0;
	unsigned int tubelength = 0;
	unsigned char special = 0;

	unsigned int nospecial;
	unsigned char lastdanglingdistance;
	struct edge* doubletwoedge = NULL;
	unsigned char switched;


	if (size == 0) {
		nrofedges = 0;
	} else {
		nrofedges = patch->facesused[size-5]*size;
	}

	if (jointtubes != NULL) {
		(*jointtubes) = malloc(nrofnanocaps*sizeof(tube*));
	}

	(*edges) = malloc(nrofedges*sizeof(struct edge*));
	
	arraycounter = 0;

	vertexes = malloc((patch->nrofvertices+1)*sizeof(struct edge*));
	visitedloose = calloc(patch->nrofvertices+1, sizeof(unsigned char));
	vertexes[1] = patch->firstedge;

	for (i=1; i <= patch->nrofvertices; i++) {
		cedge = vertexes[i];
		do {
			if (cedge->end != 0) {
				vertexes[cedge->end] = cedge->inv;

				/* check if edge part of face with length 5 */
				facecounter = 1;
				fedge = getNextWithSpecial(cedge);

				/* iterate over face */
				while (fedge != cedge && facecounter <= 7) {

					/* We have dangling we have not visited yet */
					if (fedge->end == 0 && visitedloose[fedge->start] == 0) {
						special = 1;
					}

					facecounter++;
					fedge = getNextWithSpecial(fedge);
				}

				if (jointtubes && special) {
					doubletwoedge = NULL;
					tubelength = 0;
					nospecial = 0;
					(*jointtubes)[tubenr] = malloc(sizeof(tube));
					(*jointtubes)[tubenr]->firstparam = (*jointtubes)[tubenr]->secondparam = 0;
					(*jointtubes)[tubenr]->picked = 0;

					/* find face length and double three edge (edge after second three) */
					if (cedge->end != 0) {
						nospecial += 1;
					}
					fedge = getNextWithSpecial(cedge);
					while (fedge != cedge) {
						if (fedge->end == 0) {
							visitedloose[fedge->start] = 1;
							tubelength++; /* nr of loose edges */
							nospecial = 0;
						} else {
							nospecial += 1;
							if (nospecial > 2) {
								doubletwoedge = fedge;
							}
						}
						fedge = getNextWithSpecial(fedge);
					}
					//double two lies close to cedge
					if (cedge->end != 0 && nospecial > 1) {
						nospecial += 1;
						doubletwoedge = cedge;
					} else if (cedge->end != 0 && getNextWithSpecial(cedge)->end != 0 && nospecial > 0) {
						nospecial += 2;
						doubletwoedge = getNextWithSpecial(cedge);
					}

					if (nospecial > 2 || doubletwoedge == NULL) {
						doubletwoedge = cedge;
					}

					(*jointtubes)[tubenr]->length = tubelength;
					(*jointtubes)[tubenr]->looseedges = malloc(tubelength*sizeof(struct edge*));
					arraycounter2 = 0;
					switched = 0;
					lastdanglingdistance = 3;

					fedge = getNextWithSpecial(doubletwoedge);
					while (fedge != doubletwoedge) {
						if (fedge->end == 0) {
							(*jointtubes)[tubenr]->looseedges[arraycounter2++] = fedge;

							if (lastdanglingdistance < 2) {
								switched = 1;
							}

							if (!switched) {
								(*jointtubes)[tubenr]->firstparam++;
							} else {
								(*jointtubes)[tubenr]->secondparam++;
							}
							lastdanglingdistance = 0;

						} else {
							lastdanglingdistance += 1;
						}
						fedge = getNextWithSpecial(fedge);

					}
					tubenr++;

				} else if (facecounter == size) {
					(*edges)[arraycounter++] = cedge;
				}
			}
			cedge = cedge->next;
			special = 0;
		} while (cedge != vertexes[i]);
	}

	free(vertexes);

}

int getCode(struct td_patch* patch, struct edge* startedge, vertextype** code, int maxvertices, vertextype** mincode, int minlength, unsigned char mirror) {
	unsigned int *vertexmap;
	unsigned char *seen, isminimum;
	struct edge** firstedges;
	struct edge* cedge;
	int feindex, fesize;
	int cindex;
	int lastempty;
	int nextnumber;

	if (mirror) {
		startedge = startedge->inv;
	}


	/* We will not use zero position for vertex map*/
	seen = calloc((maxvertices+1), sizeof(unsigned char));
	/* maps graph vertex number onto code vertex number */
	vertexmap = calloc(maxvertices + 1, sizeof(unsigned int));
	/* FIFO list of startedges for the vertices we visited*/
	firstedges = calloc(maxvertices, sizeof(struct edge*));
	cindex = 0;
	isminimum = 1; //1 while equal - 0 when bigger - 2 when smaller

	vertexmap[startedge->start] = 1;
	/* next number in code */
	nextnumber = 2;
	/* bitstring that represents visited vertices*/
	lastempty = 1;

	firstedges[0] = startedge;
	feindex = 0;
	fesize = 1;
	while (lastempty < patch->nrofvertices && isminimum) {
		cedge = firstedges[feindex];
		/* vertex is part of join and has not been processed yet */
		if (cedge->start <= patch->nrofvertices && seen[cedge->start] == 0) {
			seen[cedge->start] = 1;
			/* find next vertex that has not been processed yet*/
			while (lastempty <= patch->nrofvertices && seen[lastempty] != 0) {
				lastempty += 1;
			}
		}

		
		/* rotate over edges arount vertex */
		do {
			/* end vertex needs number */
			if (cedge->end != 0 && vertexmap[cedge->end] == 0) {
				vertexmap[cedge->end] = nextnumber++;
				firstedges[fesize++] = cedge->inv;
			}
			(*code)[cindex++] = vertexmap[cedge->end];
			if (!mirror) {
				cedge = cedge->next;
			} else {
				cedge = cedge->prev;
			}

			if (isminimum == 1 && (((*code)[cindex-1] > (*mincode)[cindex-1] && (*mincode)[cindex-1] != 0) || (cindex > minlength && minlength != 0))) {
				isminimum = 0;
			} else if (isminimum == 1 && (*code)[cindex-1] < (*mincode)[cindex-1]) {
				isminimum = 2;
			}
		} while (cedge != firstedges[feindex] && isminimum);

		feindex++;
	}
	free(seen);
	free(vertexmap); 
	free(firstedges);
	return isminimum ? cindex : -1;
}

unsigned char checkJoin(struct td_patch* patch) {
	int i, j, maxvertices, minlength, length, nrofedges, size;
	unsigned char iscopy;
	struct edge** specialedges;
	tube** jointtubes;

	vertextype* code1;
	vertextype* code2;
	vertextype* code;
	vertextype* min;
	vertextype* temp;
	maxvertices = tubes[nrofnanocaps-1]->maxvertexnum;
	code1 = calloc(maxvertices*3, sizeof(vertextype));
	code2 = calloc(maxvertices*3, sizeof(vertextype));

	minlength = maxvertices*3;
	min = code1;
	code = code2;

	size = 0;
	if (patch->facesused[0] != 0) {
		size = 5;
	} else if (patch->facesused[2] != 0) {
		size = 7;
	}


	/*search for pentagons and special face edges */
	getSpecialEdges(patch, &specialedges, &jointtubes, size);
	/* attach tubes */

	for (i=0; i < nrofnanocaps; i++) {
		j = 0;
		while (jointtubes[j]->picked == 1 || jointtubes[j]->firstparam != tubes[i]->firstparam || jointtubes[j]->secondparam != tubes[i]->secondparam) {
			j += 1;
		}
		addTube(tubes[i], jointtubes[j]);
		jointtubes[j]->picked = 1; /* already picked */
	}

	/* build code */
	nrofedges = patch->facesused[0]*5;
	if (nrofedges == 0) {
		nrofedges = patch->facesused[2]*7;
	}

	/* normal order */
	for (j = 0; j <= checkinverse; j++) {
		for (i=0; i < nrofedges; i++) {
			length = getCode(patch, specialedges[i], &code, maxvertices, &min, minlength, j);
			if (length != -1) {
				minlength = length;
				temp = min;
				min = code;
				code = temp;
			}
		}
	}

	/* remove tubes */
	free(specialedges);
	for (i=0; i < nrofnanocaps; i++) {
		free(jointtubes[i]->looseedges);
		free(jointtubes[i]);
		removeTube(tubes[i]);
	}
	free(jointtubes);

	//only hexagons => only one
	iscopy = (patch->facesused[0] == patch->facesused[2] && patch->facesused[2] == 0 && statistics[indextranslate[0][patch->facesused[1]][0]] != 0);
	iscopy = iscopy ? 1 : !(findPrefix(min, minlength) == 0);
	if (!iscopy) {
		addPrefix(min, minlength);
		statistics[indextranslate[patch->facesused[0]][patch->facesused[1]][patch->facesused[2]]]++;
		FOUND_JOINS++;
	} else {
		ISO_JOINS++;
	}

	free(code1);
	free(code2);

	return !iscopy;
}

