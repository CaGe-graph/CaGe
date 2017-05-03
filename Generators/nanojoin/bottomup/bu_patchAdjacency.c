/*
 * patchAdjency.c
 *
 *  Created on: Nov 21, 2012
 *      Author: Dieter
 */

#include <stdlib.h>
#include <string.h>

#include "bu_patchAdjacency.h"
#include "bu_nanojoin.h"

void bu_getSingleFaceAdjacency(int facesize, vertextype*** result) {
	int i;
	*result = malloc((facesize + 1) * sizeof(vertextype*));
	if ((*result) == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	//meta date : 0 = nrofvertices, 1 = begin vertex of mark, 2 = end vertex of mark
	(*result)[0] = malloc(3 * sizeof(vertextype));
	if ((*result)[0] == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	(*result)[0][0] = facesize;
	(*result)[0][1] = facesize;
	(*result)[0][2] = 1;
	for (i = 1; i <= facesize; i++) {
		(*result)[i] = malloc(3 * sizeof(vertextype));
		if ((*result)[i] == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		(*result)[i][0] = facesize - ((2 * facesize - (i - 1)) % facesize); //index of vertex before i
		(*result)[i][1] = facesize - ((2 * facesize - (i + 1)) % facesize); //index of vertex after i
		(*result)[i][2] = 0;
	}
}

vertextype bu_getNextBorderVertex(vertextype** adjacencyrow, vertextype prevvertex) {
	int max;
	int i = 0;
	while ((*adjacencyrow)[i] != prevvertex) {
		i++;
	}
	if ((*adjacencyrow)[2] == 0) {
		max = 2;
	} else {
		max = 3;
	}
	return (*adjacencyrow)[(i + 1) % max];

}

vertextype bu_getPreviousBorderVertex(vertextype** adjacencyrow, vertextype nextvertex) {
	int max;
	int i = 0;
	while ((*adjacencyrow)[i] != nextvertex) {
		i++;
	}
	if ((*adjacencyrow)[2] == 0) {
		max = 2;
	} else {
		max = 3;
	}
	return (*adjacencyrow)[(max + (i - 1)) % max];

}

int bu_mergeOne(vertextype*** patch, vertextype** translatetable) {
	int i, j;
	int currentindex = 1, passed;
	passed = 1;
	for (i = 1; i <= (*patch)[0][0]; i++) {
		if (currentindex == (*translatetable)[i - 1]) {
			//just adjust ids
			for (j = 0; j < 3; j++) {
				if ((*patch)[i][j] != 0) {
					(*patch)[currentindex][j] = (*translatetable)[(*patch)[i][j] - 1];
				} else {
					(*patch)[currentindex][j] = 0;
				}
			}
			currentindex++;
		} else {
			//two vertices have to be merged
			if ((*patch)[i][2] != 0) {
				//one with degree 2 and one with degree 3 -> just copy
				(*patch)[(*translatetable)[i - 1]][0] = (*translatetable)[(*patch)[i][0] - 1];
				(*patch)[(*translatetable)[i - 1]][1] = (*translatetable)[(*patch)[i][1] - 1];
				(*patch)[(*translatetable)[i - 1]][2] = (*translatetable)[(*patch)[i][2] - 1];
			} else if ((*patch)[i][2] == 0 && (*patch)[(*translatetable)[i - 1]][2] == 0) {
				//both vertices have degree 2 (successor needs to be checked)
				if ((*patch)[(*translatetable)[i - 1]][0] == (*translatetable)[(*patch)[i][1] - 1]) {
					//end of mergepath
					(*patch)[(*translatetable)[i - 1]][0] = (*translatetable)[(*patch)[i][0] - 1];
					(*patch)[(*translatetable)[i - 1]][2] = (*translatetable)[(*patch)[i][1] - 1];
				} else if ((*patch)[(*translatetable)[i - 1]][1] == (*translatetable)[(*patch)[i][0] - 1]) {
					//begin of mergepath
					(*patch)[(*translatetable)[i - 1]][1] = (*translatetable)[(*patch)[i][1] - 1];
					(*patch)[(*translatetable)[i - 1]][2] = (*translatetable)[(*patch)[i][0] - 1];
				} else {
					fprintf(stderr, "Impossible error 2\n");
				}
			}
		}
	}
	/* check if valid 
		should be integrated into code
	*/
	for (i = 1; i < (*patch)[0][0]; i++) {
		if ((*patch)[i][0] == (*patch)[i][1] || (*patch)[i][0] == (*patch)[i][2] || (*patch)[i][1] == (*patch)[i][2]) {
			passed = 0;
		}
	}
	for (i = currentindex; i <= (*patch)[0][0]; i++) {
		free((*patch)[i]);
	}
	return passed;
}

void bu_mergeTwo(vertextype*** left, vertextype*** result, vertextype** translatetable) {
	int degree, i, j;
	//add left to right
	for (i = 1; i <= (*left)[0][0]; i++) {
		//does i have 2 or 3 edges
		if ((*left)[i][2] == 0) {
			degree = 2;
		} else {
			degree = 3;
		}
		//change old vertex ids by new ones
		for (j = 0; j < degree; j++) {
			(*left)[i][j] = (*translatetable)[(*left)[i][j] - 1];
		}
		if ((*translatetable)[i - 1] > (*result)[0][0]) {
			//just replace right vertex with left one
			(*result)[(*translatetable)[i - 1]] = malloc(3 * sizeof(vertextype));
			if ((*result)[(*translatetable)[i - 1]] == NULL) {
				fprintf(stderr, "ERROR: malloc returned NULL\n");
				exit(EXIT_FAILURE);
			}
			memcpy((*result)[(*translatetable)[i-1]], (*left)[i], 3*sizeof(vertextype));
		} else if ((*left)[i][2] != 0) {
			memcpy((*result)[(*translatetable)[i-1]], (*left)[i], 3*sizeof(vertextype));
		} else if ((*result)[(*translatetable)[i - 1]][2] == 0) {
			if ((*result)[(*translatetable)[i - 1]][0] == (*left)[i][1]) {
				//succesor of left patch vertex is right patch vertex
				(*result)[(*translatetable)[i - 1]][0] = (*left)[i][0];
				(*result)[(*translatetable)[i - 1]][2] = (*left)[i][1];
			} else if ((*result)[(*translatetable)[i - 1]][1] == (*left)[i][0]) {
				//successor of right patch vertex is left patch vertex
				(*result)[(*translatetable)[i - 1]][2] = (*left)[i][0];
				(*result)[(*translatetable)[i - 1]][1] = (*left)[i][1];
			} else {
				//for debugging reasons
				fprintf(stderr, "Impossible error 1\n");
			}
		}
	}
}

int bu_gotoStartLeft(vertextype*** patch, vertextype* lcurrent, vertextype* lnext, int loffset) {
	int offset;
	vertextype temp;
	offset = 0;
	*lcurrent = (*patch)[0][2];
	*lnext = bu_getNextBorderVertex(&((*patch)[*lcurrent]), (*patch)[0][1]);
	if (loffset > 0) {
		*lnext = *lcurrent;
		*lcurrent = (*patch)[0][1];
		offset = 1;
		while (offset++ < loffset) {
			temp = bu_getPreviousBorderVertex(&((*patch)[*lcurrent]), *lnext);
			*lnext = *lcurrent;
			*lcurrent = temp;
		}
	}
	return offset;
}

int bu_gotoStartRight(vertextype*** patch, vertextype* rcurrent, vertextype* rprev, int roffset) {
	int offset;
	vertextype temp;
	offset = 0;
	*rcurrent = (*patch)[0][2];
	*rprev = (*patch)[0][1];
	while (offset++ < roffset) {
		temp = bu_getNextBorderVertex(&((*patch)[*rcurrent]), *rprev);
		*rprev = *rcurrent;
		*rcurrent = temp;
	}
	return offset;
}

void bu_updateCurrentNext(vertextype*** left, vertextype*** right, vertextype* lcurrent, vertextype* lnext, vertextype* rprev, vertextype* rcurrent) {
	vertextype templ, tempr;
	templ = bu_getPreviousBorderVertex(&((*left)[*lcurrent]), *lnext);
	tempr = bu_getNextBorderVertex(&((*right)[*rcurrent]), *rprev);
	*lnext = *lcurrent;
	*rprev = *rcurrent;
	*lcurrent = templ;
	*rcurrent = tempr;
}

int bu_buildTranslatetableOne(vertextype*** patch, vertextype lcurrent, vertextype lnext, vertextype rprev, vertextype rcurrent, vertextype** translatetable) {
	int i, offset;
	//start with identity function
	for (i = 0; i < (*patch)[0][0]; i++) {
		(*translatetable)[i] = i + 1;
	}

	//adjust ids of vertices in mergepath
	(*translatetable)[lcurrent - 1] = rcurrent;
	bu_updateCurrentNext(patch, patch, &lcurrent, &lnext, &rprev, &rcurrent);
	(*translatetable)[lcurrent - 1] = rcurrent;
	while (!((*patch)[lcurrent][2] == 0 && (*patch)[rcurrent][2] == 0)) {
		bu_updateCurrentNext(patch, patch, &lcurrent, &lnext, &rprev, &rcurrent);
		(*translatetable)[lcurrent - 1] = rcurrent;
	}

	//adjust other vertices
	offset = 0;
	for (i = 1; i <= (*patch)[0][0]; i++) {
		if ((*translatetable)[i - 1] == i) {
			(*translatetable)[i - 1] = ++offset;
		} else if ((*translatetable)[i - 1] < i) {
			(*translatetable)[i - 1] = (*translatetable)[(*translatetable)[i - 1] - 1];
		} else {
			//make sure vertex id can only get smaller translatetable[i] <= i+1
			(*translatetable)[(*translatetable)[i - 1] - 1] = i;
			(*translatetable)[i - 1] = ++offset;
		}
	}
	//setting mark
	(*patch)[0][2] = (*translatetable)[lcurrent - 1];
	(*patch)[0][1] = (*translatetable)[bu_getPreviousBorderVertex(&((*patch)[lcurrent]), lnext) - 1];
	return offset;
}

int bu_buildTranslatetableTwo(vertextype*** left, vertextype*** right, vertextype lcurrent, vertextype lnext, vertextype rprev, vertextype rcurrent, vertextype** translatetable) {
	int i, offset;
	vertextype lfirst, rfirst;

	//initialize to zero
	for (i = 0; i < (*left)[0][0]; i++) {
		(*translatetable)[i] = 0;
	}

	lfirst = lcurrent;
	rfirst = rcurrent;
	(*translatetable)[lcurrent - 1] = rcurrent;
	bu_updateCurrentNext(left, right, &lcurrent, &lnext, &rprev, &rcurrent);
	(*translatetable)[lcurrent - 1] = rcurrent;

	//map vertices in mergepath
	while (!((*left)[lcurrent][2] == 0 && (*right)[rcurrent][2] == 0) && lcurrent != lfirst && rcurrent != rfirst) {
		bu_updateCurrentNext(left, right, &lcurrent, &lnext, &rprev, &rcurrent);
		(*translatetable)[lcurrent - 1] = rcurrent;
	}

	//cycle or not
	if ((lcurrent != lfirst) == (rcurrent != rfirst)) {
		//no cycle -> adjust ids so that you have all numbers from 1..n
		offset = (*right)[0][0];
		for (i = 0; i < (*left)[0][0]; i++) {
			if ((*translatetable)[i] == 0) {
				(*translatetable)[i] = ++offset;
			}
		}
	} else {
		//there is a cycle -> store needed information in first 3 elements of translatetable
		offset = 0;
		if (lcurrent == lfirst) {
			//left patch inner patch
			(*translatetable)[0] = 1;
			(*translatetable)[1] = rprev;
			(*translatetable)[2] = rcurrent;
		} else {
			//right patch inner patch
			(*translatetable)[0] = 0;
			(*translatetable)[1] = lcurrent;
			(*translatetable)[2] = lnext;
		}
	}
	return offset;
}

int bu_getPatchAdjacency(struct bu_patch* p, vertextype*** result) {
	int i, offset, passed;
	vertextype rprev, rcurrent, lnext, lcurrent;
	unsigned char** left;
	vertextype* translatetable;
	passed = 1;
	if (!p->lp && !p->rp) {
		//patch is a single face
		if (p->border[0] >= 5 && p->border[0] <= 7) {
			bu_getSingleFaceAdjacency(p->border[0], result);
		} else {
			printf("Single faces should be of size 5,6 or 7, not %d", p->border[0]);
		}
	} else if (p->lp && p->rp) {
		//two patches need to be merged
		passed = bu_getPatchAdjacency(p->lp, &left);
		passed = passed & bu_getPatchAdjacency(p->rp, result);

		if (passed) {

			if (left[0][0] + (*result)[0][0] > 255) {
				fprintf(stderr, "Graph has too many vertices: %d\n", left[0][0] + (*result)[0][0]);
				exit(EXIT_FAILURE);
			}

			//find startpoints mergepad

			bu_gotoStartLeft(&left, &lcurrent, &lnext, p->loffset);
			bu_gotoStartRight(result, &rcurrent, &rprev, p->roffset);

			//now rcurrent and lcurrent are the vertices corresponding to the first common vertex of the new patch

			//build translatetable (mapping of vertexid in old patch to vertexid in new patch)
			translatetable = malloc(left[0][0] * sizeof(vertextype));
			if (translatetable == NULL) {
				fprintf(stderr, "ERROR: malloc returned NULL\n");
				exit(EXIT_FAILURE);
			}
			offset = bu_buildTranslatetableTwo(&left, result, lcurrent, lnext, rprev, rcurrent, &translatetable);

			//build new patch
			if (offset != 0) {
				//simple merge operation, no inner patch (cutpath in new patch has no cycle)
				*result = realloc((*result), (offset + 1) * sizeof(vertextype*));
				if (*result == NULL) {
					fprintf(stderr, "ERROR: malloc returned NULL\n");
					exit(EXIT_FAILURE);
				}
				(*result)[0][1] = rprev;
				(*result)[0][2] = rcurrent;
				bu_mergeTwo(&left, result, &translatetable);
				(*result)[0][0] = offset;
			} else {
				//two merge operations needed (cutpath in new patch has cycle)
				if (translatetable[0] == 0) {
					//right patch is inner patch

					//paste left patch to itself
					offset = bu_buildTranslatetableOne(&left, translatetable[1], translatetable[2], bu_getPreviousBorderVertex(&(left[lcurrent]), lnext), lcurrent, &translatetable);
					bu_mergeOne(&left, &translatetable);
					left[0][0] = offset;

					//adjust lcurrent and lnext to new vertexids
					lcurrent = translatetable[lcurrent - 1];
					lnext = translatetable[lnext - 1];

					//add inner patch
					translatetable = realloc(translatetable, left[0][0] * sizeof(vertextype));
					if (translatetable == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					offset = bu_buildTranslatetableTwo(&left, result, lcurrent, bu_getPreviousBorderVertex(&(left[lcurrent]), lnext), rprev, rcurrent, &translatetable);
					*result = realloc((*result), (offset + 1) * sizeof(vertextype*));
					if (*result == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					bu_mergeTwo(&left, result, &translatetable);

					(*result)[0][0] = offset;
					(*result)[0][1] = translatetable[left[0][1]-1];
					(*result)[0][2] = translatetable[left[0][2]-1];
				} else {
					//left patch is inner patch

					//paste right patch to itself
					translatetable = realloc(translatetable, (*result)[0][0] * sizeof(vertextype));
					if (translatetable == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					offset = bu_buildTranslatetableOne(result, rcurrent, bu_getNextBorderVertex(&((*result)[rcurrent]), rprev), translatetable[1], translatetable[2], &translatetable);
					bu_mergeOne(result, &translatetable);
					(*result)[0][0] = offset;

					//adjust rcurrent and rprev
					rcurrent = translatetable[rcurrent - 1];
					rprev = translatetable[rprev - 1];

					//add inner patch
					translatetable = realloc(translatetable, left[0][0] * sizeof(vertextype));
					if (translatetable == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					offset = bu_buildTranslatetableTwo(&left, result, lcurrent, lnext, bu_getNextBorderVertex(&((*result)[rcurrent]), rprev), rcurrent, &translatetable);
					*result = realloc((*result), (offset + 1) * sizeof(vertextype*));
					if (*result == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					bu_mergeTwo(&left, result, &translatetable);

					(*result)[0][0] = offset;
				}
			}

			//clear no longer needed memory
			for (i = left[0][0]; i >= 0; i--) {
				free(left[i]);
			}
			free(left);
			free(translatetable);
		}
	} else if (p->rp) {
		//there is only a right patch cutpath in new patch ends in a different face than the one in starts in
		passed = bu_getPatchAdjacency(p->rp, result);
		if (passed) {
			bu_gotoStartLeft(result, &lcurrent, &lnext, p->loffset);
			bu_gotoStartRight(result, &rcurrent, &rprev, p->roffset);

			translatetable = malloc((*result)[0][0] * sizeof(vertextype));
			if (translatetable== NULL) {
				fprintf(stderr, "ERROR: malloc returned NULL\n");
				exit(EXIT_FAILURE);
			}
			offset = bu_buildTranslatetableOne(result, lcurrent, lnext, rprev, rcurrent, &translatetable);

			passed = bu_mergeOne(result, &translatetable);
			(*result)[0][0] = offset;

			free(translatetable);
		}
	}
	return passed;
}

void bu_writeHeaderToFile(FILE* fp) {
	unsigned char str[15] = ">>planar_code<<";
	fwrite(str, sizeof(unsigned char), 15, fp);
}

int bu_writeSingleToFile(struct bu_patch* patch, FILE* fp) {
	int i, r;
	vertextype** result;
	r = bu_getPatchAdjacency(patch, &result);
	if (r) {
		r = 1;
		bu_adjacencyToFile(&result, fp);
	} else {
		r = 0;
	}
	for (i=result[0][0]; i >= 0; i--) {
		free(result[i]);
	}
	free(result);
	return r;
}

void bu_adjacencyToFile(vertextype*** result, FILE* fp) {
	int i, nrofvertices, ret;
	nrofvertices = (*result)[0][0];
	fputc(nrofvertices, fp);
	for (i = 1; i <= nrofvertices; i++) {
		fwrite((*result)[i], sizeof(unsigned char), 3, fp);
		if ((*result)[i][2] != 0) {
			fputc(0, fp);
		}
	}
	ret = fflush(fp);
	if (ret != 0) {
		fprintf(stderr, "Could not flush data");
	}

}

