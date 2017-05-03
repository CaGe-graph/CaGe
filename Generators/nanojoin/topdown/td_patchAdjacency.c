#include <stdlib.h>
#include <stdio.h>

#include "td_patchAdjacency.h"
#include "td_graphutils.h"
#include "td_common.h"
#include "td_isomorphismcheck.h"
#include "isomorphismcheck.h"

void getPatchAdjacency2(struct edge* edge, vertextype*** result);
void getPatchAdjacency(struct td_patch* patch, vertextype*** result);
void writePatchToFile(struct td_patch* patch);
void adjacencyToFile(vertextype*** result);
void addRing(vertextype *** result);

void addRing(vertextype*** result) {
	int i, length, start, end, numvertex, nrofvertices, currentnew;
	unsigned char ldegree, cdegree;
	vertextype previous, next, current;

	nrofvertices = (*result)[0][0];
	for (numvertex=1; numvertex <= nrofvertices; numvertex++) {
		if ((*result)[numvertex][2] == 0) {
			/* found vertex of degree 2 */

			/* Find right direction and length*/
			length = 1;
			previous = numvertex;
			next = (*result)[numvertex][0];
			while (next != numvertex) {
				if ((*result)[next][0] == previous) {
					previous = next;
					next = (*result)[next][1];
				} else if ((*result)[next][1] == previous && (*result)[next][2] != 0) {
					previous = next;
					next = (*result)[next][2];
				} else {
					previous = next;
					next = (*result)[next][0];
				}
				length++;
			}
			if (length <= 7) {
				length = 1;
				previous = numvertex;
				next = (*result)[numvertex][1];
				while (next != numvertex) {
					if ((*result)[next][0] == previous) {
						previous = next;
						next = (*result)[next][1];
					} else if ((*result)[next][1] == previous && (*result)[next][2] != 0) {
						previous = next;
						next = (*result)[next][2];
					} else {
						previous = next;
						next = (*result)[next][0];
					}
					length++;
				}
			}

			/* Creating extra vertices */
			*result = realloc((*result), ((*result)[0][0] + 1 + length) * sizeof(vertextype*));
			start = (*result)[0][0] + 1;
			end = (*result)[0][0] + length + 1;
			for (i=start; i < end; i++) {
				(*result)[i] = calloc(3, sizeof(vertextype));
			}

			start = (*result)[0][0] + 2;
			end = (*result)[0][0] + length + 1;
			for (i= start; i < end; i++) {
				(*result)[i][0] = i - 1;
			}
			(*result)[start-1][0] = (*result)[0][0] + length;

			start = (*result)[0][0] + 1;
			end = (*result)[0][0] + length;
			for (i=start; i < end; i++) {
				(*result)[i][1] = i + 1;
			}
			(*result)[end][1] = (*result)[0][0] + 1;

			current = numvertex;

			/* Connecting them */

			/* currentnew contains the position in the newly added cycle */
			currentnew = 1;
			ldegree = cdegree = 0;
			for (i=1; i <= length; i++) {
				/* ldegree and cdegree keep track of convex and concave edge */
				ldegree = cdegree;
				if ((*result)[current][2] == 0) {
					cdegree = 2;
				} else {
					cdegree = 3;
				}
				if (ldegree == 2 && cdegree == 2) {
					currentnew += 2;
				} else if (ldegree == 3 && cdegree == 3) {
					currentnew -= 2;
				}
				currentnew = (currentnew + length - 1) % length + 1;

				if ((*result)[current][0] == previous) {
					if ((*result)[current][2] == 0) {
						(*result)[current][2] = (*result)[current][1];
						(*result)[current][1] = (*result)[0][0] + currentnew;
						(*result)[(*result)[0][0]+currentnew][2] = current;
						previous = current;
						current = (*result)[current][2];
					} else {
						previous = current;
						current = (*result)[current][1];
					}
				} else if ((*result)[current][1] == previous) {
					if ((*result)[current][2] == 0) {
						(*result)[current][2] = (*result)[0][0] + currentnew;
						(*result)[(*result)[0][0]+currentnew][2] = current;
						previous = current;
						current = (*result)[current][0];
					} else {
						previous = current;
						current = (*result)[current][2];
					}
				} else {
					previous = current;
					current = (*result)[current][0];
				}
				currentnew += 1;
			}
			(*result)[0][0] = (*result)[0][0] + length;
		}
	}
}

struct triple {
	int x;
	int y;
	int z;
};

void markVertex(struct edge* edge) {
	struct edge* current;

	current = edge;
	do {
		current->start = -current->start;
		if (current->end != 0) {
			current->inv->end = current->start;
		}
		current = current->next;
	} while (current != edge);
}

struct triple* getMinMax(struct edge* edge) {
	struct triple* result;
	struct triple* temp;
	struct edge* current;
	result = malloc(sizeof(struct triple));
	result->x = result->y = edge->start;
	result->z = 1;
	markVertex(edge);
	current = edge;
	do {
		if (current->end > 0) {
			temp = getMinMax(current->inv);
			if (result->x > temp->x) {
				result->x = temp->x;
			}
			if (result->y < temp->y) {
				result->y = temp->y;
			}
			result->z += temp->z;
			free(temp);
		}
		current = current->next;
	} while (current != edge);
	return result;
}

void getPatchAdjacency2(struct edge* edge, vertextype*** result) {
	int i, j;
	int vertexnr = 1;
	struct triple* t;
	struct edge *current;
	int minvertexnr, maxvertexnr, nrofvertices;
	vertextype* vertexmap;
	struct edge** firstedges;
	/* find max and min vertexnumber */
	t = getMinMax(edge);
	minvertexnr = t->x;
	maxvertexnr = t->y;
	nrofvertices = t->z;

	(*result) = malloc((nrofvertices+1)*sizeof(vertextype*));
	(*result)[0] = malloc(3*sizeof(int));
	(*result)[0][0] = nrofvertices;
	(*result)[0][1] = edge->start;
	(*result)[0][2] = edge->end;

	vertexmap = calloc((maxvertexnr-minvertexnr+1), sizeof(vertextype));
	firstedges = calloc(nrofvertices, sizeof(struct edge*));

	firstedges[0] = edge;
	markVertex(edge);
	vertexmap[edge->start - minvertexnr] = vertexnr;
	for (i=0; i < nrofvertices; i++) {
		(*result)[i+1] = calloc(3, sizeof(vertextype*));
		current = firstedges[i];
		j = 0;
		do {
			if (current->end < 0) {
				markVertex(current->inv);
				firstedges[vertexnr] = current->inv;
				vertexmap[current->end - minvertexnr] = ++vertexnr;
			}
			if (current->end != 0) {
				(*result)[i+1][j] = vertexmap[current->end - minvertexnr];
				j += 1;
			}
			current = current->next;
		} while (current != firstedges[i]);
	}
	free(firstedges);
	free(vertexmap);
}

void getPatchAdjacency(struct td_patch* patch, vertextype*** result) {
	vertextype i, nrofvertices;
	int j;
	struct edge** firstedges;
	struct edge* currentedge;
	struct edge* nextedge;
	nrofvertices = patch->nrofvertices;
	firstedges = malloc(nrofvertices*sizeof(struct edge*));
	if (firstedges == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
	}
	for (i=0; i < nrofvertices; i++) {
		firstedges[i] = NULL;
	}
	(*result) = malloc((nrofvertices+1)*sizeof(vertextype*));
	if (*result == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
	}
	(*result)[0] = malloc(3*sizeof(vertextype));
	if (*result[0] == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
	}
	(*result)[0][0] = nrofvertices;
	(*result)[0][1] = patch->mark->start;
	(*result)[0][2] = patch->mark->end;

	firstedges[0] = patch->firstedge;
	for(i=0; i < *result[0][0]; i++) {
		currentedge = firstedges[i];
		(*result)[i+1] = malloc(3*sizeof(vertextype));
		if ((*result)[i+1] == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
		}
		(*result)[i+1][0] = currentedge->end;
		firstedges[currentedge->end-1] = currentedge->inv;
		(*result)[i+1][1] = (*result)[i+1][2] = 0;
		nextedge = currentedge->next;
		j = 1;
		while (nextedge != currentedge) {
			if (nextedge->end != 0) {
				firstedges[nextedge->end-1] = nextedge->inv;
				(*result)[i+1][j] = nextedge->end;
				j += 1;
			}
			nextedge = nextedge->next;
		}
	}
	free(firstedges);
}

void prepareWrite(int pent, int hex, int hept) {
	int i;
	int maxvertices;
	printf("%s", ">>planar_code<<");
	maxvertices = 5*pent+6*hex+7*hept + 3*(outsideparameters[0] + outsideparameters[1]);
	for (i=0; i < insidenanocaps; i++) {
		maxvertices += 3*(insideparameters[2*i+1] + insideparameters[1*i+2]);
	}
	maxvertices /= 3;
	maxvertices += 1;
	prepareIsomorphism(maxvertices);
}

void newJoin(struct td_patch* patch) {
	if (checkJoin(patch)) {
		writePatchToFile(patch);
	}
}

void writePatchToFile(struct td_patch* patch) {
	int i;
	vertextype** result;
	getPatchAdjacency(patch, &result);


	ALL_JOINS++;
	for (i=0; i < RINGSTOADD; i++) {
		addRing(&result);
	}
	adjacencyToFile(&result);

	for (i=result[0][0]; i >= 0; i--) {
		free(result[i]);
	}

	free(result);
}

void adjacencyToFile(vertextype*** result) {
	int i, nrofvertices, j;
	nrofvertices = (*result)[0][0];
	printf("%c", nrofvertices);
	for (i = 1; i <= nrofvertices; i++) {
		j = 0;
		while (j < 3 && (*result)[i][j] != 0) {
			printf("%c", (*result)[i][j++]);
		}
		printf("%c", 0);
	}

}