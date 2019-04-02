#include <stdlib.h>
#include <stdio.h>

#include "td_patchAdjacency.h"

void getPatchAdjacency(struct td_patch* patch, vertextype*** result);
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

void getPatchAdjacency(struct td_patch* patch, vertextype*** result) {
	vertextype i, nrofvertices;
	int j;
	struct edge** firstedges;
	struct edge* currentedge;
	struct edge* nextedge;
	nrofvertices = patch->nrofvertices;
	firstedges = malloc(nrofvertices*sizeof(struct edge*));

	/* firstedges array */
	if (firstedges == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}

	for (i=0; i < nrofvertices; i++) {
		firstedges[i] = NULL;
	}

	/* result array */
	(*result) = malloc((nrofvertices+1)*sizeof(vertextype*));
	if (*result == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
	}

	(*result)[0] = malloc(3*sizeof(vertextype));
	if (*result[0] == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
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
			exit(EXIT_FAILURE);
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

void writePatchToFile(struct td_patch* patch) {
	int i;
	vertextype** result;

	getPatchAdjacency(patch, &result);
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
	printf("%c%c", nrofvertices % 256, nrofvertices/256);
	for (i = 1; i <= nrofvertices; i++) {
		j = 0;
		while (j < 3 && (*result)[i][j] != 0) {
			printf("%c%c", (*result)[i][j] % 256, (*result)[i][j]/256);
			j++;
		}
		printf("%c%c", 0, 0);
	}
	printf("%c", 0);

}