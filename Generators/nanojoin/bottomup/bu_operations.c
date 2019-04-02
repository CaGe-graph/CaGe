#include <stdio.h>
#include <stdlib.h>

#include "bu_operations.h"
#include "bu_extra.h"
#include "bu_nanojoin.h"

/*
 * Method that combines two patches the normal way (no wrap around)
 *
 * Input bordercode can not contain a negative number, since they do not contain valid mergepaths.
 *
 */
int combine(struct bu_patch *rp, struct bu_patch *lp, int roffset, int loffset, int mergesize, struct bu_patch *combined) {
	int rightmergelength, leftmergelength;
	int realroffset, realloffset, nrofvertices;
	int size, rsize, lsize;
	int i, j, valid;
	//COMBINE_NUMBER++;
	valid = 1;

	rsize = rp->border[0];
	lsize = lp->border[0];

	//calculate by how many digits the mergepath is represented in the bordercode
	if (mergesize % 2 == 0) {
		rightmergelength = 3;
		leftmergelength = 1;
	} else {
		rightmergelength = 2;
		leftmergelength = 2;
	}

	if (rsize - rightmergelength >= 2 && lsize - leftmergelength >= 2) {
		//new mark should not be inside a 32323232
		if (rp->border[(roffset - 1 + rsize) % rsize + 1] == 0) {
			size = (rsize - rightmergelength - 1) + (lsize - leftmergelength - 1);

			i = 0;
			combined->border[i++] = size;
			//build new border
			if (1 + lp->border[(loffset + leftmergelength) % lsize + 1] < 128) {
				combined->border[i++] = 1 + lp->border[(loffset + leftmergelength) % lsize + 1];
			} else {
				fprintf(stderr, "Element of bordercode is bigger than 127\n");
				exit(EXIT_FAILURE);
			}
			//copy from left
			for (j = 0; j < lsize - leftmergelength - 2; j++) {
				combined->border[i++] = lp->border[(loffset + leftmergelength + 1 + j) % lsize + 1];
			}
			if (lp->border[(loffset + lsize - 1) % lsize + 1] + 1 + rp->border[(roffset + rightmergelength) % rsize + 1] < 128) {
				combined->border[i++] = lp->border[(loffset + lsize - 1) % lsize + 1] + 1 + rp->border[(roffset + rightmergelength) % rsize + 1];
			} else {
				fprintf(stderr, "Element of bordercode is bigger than 127\n");
				exit(EXIT_FAILURE);
			}

			//copy from right
			for (j = 0; j < rsize - rightmergelength - 2; j++) {
				combined->border[i++] = rp->border[(roffset + rightmergelength + 1 + j) % rsize + 1];
			}
		} else {
			valid = 0;
		}
	} else if (rsize - rightmergelength == 1 && lsize - leftmergelength >= 2) {
		//will not be a valid border because mark will be inside a 3-2 sequence when there is a 2-2
		valid = 0;
	} else if (rsize - rightmergelength >= 2 && lsize - leftmergelength == 1) {
		if (rp->border[(roffset - 1 + rsize) % rsize + 1] == 0) {
			size = (rsize - rightmergelength - 1);
			i = 0;
			combined->border[i++] = size;
			if (2 + lp->border[(loffset + leftmergelength) % lsize + 1] + rp->border[(roffset + rightmergelength) % rsize + 1] < 128) {
				combined->border[i++] = 2 + lp->border[(loffset + leftmergelength) % lsize + 1] + rp->border[(roffset + rightmergelength) % rsize + 1];
			} else {
				fprintf(stderr, "Element of bordercode is bigger than 127\n");
				exit(EXIT_FAILURE);
			}
			//copy from right
			for (j = 0; j < rsize - rightmergelength - 2; j++) {
				combined->border[i++] = rp->border[(roffset + rightmergelength + 1 + j) % rsize + 1];
			}
		} else {
			valid = 0;
		}
	} else if (rsize - rightmergelength == 1 && lsize - leftmergelength == 1) {
		/* Totally symmetric border */
		valid = 0;
	} else if (rsize - rightmergelength >= 1 && lsize - leftmergelength == 0) {
		size = 2 + (rsize - rightmergelength);
		i = 0;
		combined->border[i++] = size;
		combined->border[i++] = -1;
		combined->border[i++] = -1;
		for (j = 0; j < rsize - rightmergelength; j++) {
			combined->border[i++] = rp->border[(roffset + rightmergelength + j) % rsize + 1];
		}
	} else {
		valid = 0;
	}

	if (valid) {
		if (lexicographical_biggest(combined->border) == 1) {
			valid = 0;
		}
		if (valid) {
			combined->lp = lp;
			combined->rp = rp;

			//setting real offset to number of vertices instead of element in bordercode
			if (rp->border[1] == 0) {
				realroffset = 0;
			} else {
				realroffset = -1;
			}
			for (i = 0; i < roffset; i++) {
				realroffset += 2 * rp->border[i + 1] + 1;
			}

			if (lp->border[1] == 0) {
				realloffset = 0;
			} else {
				realloffset = -1;
			}
			nrofvertices = 0;
			for (i = 0; i < loffset; i++) {
				realloffset += 2 * lp->border[i + 1] + 1;
				nrofvertices += 2 * lp->border[i + 1] + 1;
			}
			for (i = loffset; i < lsize; i++) {
				nrofvertices += 2 * lp->border[i + 1] + 1;
			}
			realloffset = (nrofvertices - (realloffset + mergesize) + nrofvertices) % nrofvertices;

			combined->roffset = realroffset;
			combined->loffset = realloffset;
			combined->internalvertices = rp->internalvertices + lp->internalvertices + mergesize - 1;
		}
	}
	return valid;
}