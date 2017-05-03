#include <stdio.h>
#include <stdlib.h>
#include "bu_nanojoin.h"
#include "bu_mergepaths.h"
#include "bu_operations.h"

/*
 * Adds a new mergepath from currentpatch starting at offset (offset = digit index in border code) with size size to list
 */
void addNewMergePath(struct patcheslist_element* currentpatch, int offset, int size, struct mergepath*** list) {
	struct mergepath* newel;
	newel = malloc(sizeof(struct mergepath));
	if (newel == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	if (faceswithmergepaths[currentpatch->pent][currentpatch->hex][currentpatch->hept] == 0) {
		faceswithmergepaths[currentpatch->pent][currentpatch->hex][currentpatch->hept] = 1;
	}
	int index = indextranslate[currentpatch->pent][currentpatch->hex][currentpatch->hept];
	newel->patch = currentpatch->patch;
	newel->offset = offset;
	newel->next = list[index][size - 1];
	list[index][size - 1] = newel;
}

/*
 * Fills mergepathlist with all head elements of the linked lists in list with:
 * number of pentagons <= pent
 * number of hexagons <= hex
 * number of heptagons <= hept
 */
void getmergepathlist(unsigned char pent, unsigned char hex, unsigned char hept, struct mergepath*** type) {
	int i, j, k, count;
	count = 0;
	for (i = 0; i <= pent; i++) {
		for (j = 0; j <= hex; j++) {
			for (k = 0; k <= hept; k++) {
				if (faceswithmergepaths[i][j][k] == 1) {
					mergepathlist->indexToFaces[count][0] = i;
					mergepathlist->indexToFaces[count][1] = j;
					mergepathlist->indexToFaces[count][2] = k;
					mergepathlist->list[count++] = type[indextranslate[i][j][k]];
				}
			}
		}
	}
	mergepathlist->list[count] = NULL;
}


void free_mergepath(struct mergepath** list) {
	struct mergepath* newhead = (*list)->next;
	if ((*list)->patch->border[0] != 0) {
		(*list)->patch->border[0] = 0;
		addPatchToList((*list)->patch, &new_patches);
	}
	free(*list);
	(*list) = newhead;
}

void combineleft(struct patcheslist_element* patch, int start, int size, unsigned char dfs) {
	struct mergepath* right;
	struct bu_patch* new;
	int valid = 0;
	int j;
	new = malloc(sizeof(struct bu_patch));
	if (new == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	new->border[0] = 0;
	//right mergepaths have to have the same length
	getmergepathlist(maxpent - patch->pent, maxhex - patch->hex, maxhept - patch->hept, right_mergepaths);
	j = 0;
	while (mergepathlist->list[j] != NULL) {
		right = mergepathlist->list[j][size - 1];
		while (right != NULL) {
			//create patch
			valid = combine(right->patch, patch->patch, right->offset, start, size, new);
			if (valid) {
				if (dfs) {
					processdfs(new, patch->pent + mergepathlist->indexToFaces[j][0], patch->hex + mergepathlist->indexToFaces[j][1], patch->hept + mergepathlist->indexToFaces[j][2]);
				} else {
					addPatchToNewList(new, patch->pent + mergepathlist->indexToFaces[j][0], patch->hex + mergepathlist->indexToFaces[j][1], patch->hept + mergepathlist->indexToFaces[j][2], &new_patches);
					new = malloc(sizeof(struct bu_patch));
					if (new == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					new->border[0] = 0;
				}
			}
			right = right->next;
		}
		j++;
	}
	free(new);
}


void combineright(struct patcheslist_element* patch, int start, int length, unsigned char dfs) {
	struct mergepath* left;
	struct bu_patch* new;
	int valid = 0;
	int j;
	new = malloc(sizeof(struct bu_patch));
	if (new == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	new->border[0] = 0;
	getmergepathlist(maxpent - patch->pent, maxhex - patch->hex, maxhept - patch->hept, left_mergepaths);
	j = 0;
	while (mergepathlist->list[j] != NULL) {
		left = mergepathlist->list[j][length - 1];
		while (left != NULL) {
			//create patch
			valid = combine(patch->patch, left->patch, start, left->offset, length, new);
			if (valid) {
				if (dfs) {
					processdfs(new, patch->pent + mergepathlist->indexToFaces[j][0], patch->hex + mergepathlist->indexToFaces[j][1], patch->hept + mergepathlist->indexToFaces[j][2]);
				} else {
					addPatchToNewList(new, patch->pent + mergepathlist->indexToFaces[j][0], patch->hex + mergepathlist->indexToFaces[j][1], patch->hept + mergepathlist->indexToFaces[j][2], &new_patches);
					new = malloc(sizeof(struct bu_patch));
					if (new == NULL) {
						fprintf(stderr, "ERROR: malloc returned NULL\n");
						exit(EXIT_FAILURE);
					}
					new->border[0] = 0;
				}
			}
			left = left->next;
		}
		j++;
	}
	free(new);
}

void ismergepathleft(struct patcheslist_element* patch, int start, unsigned char dfs) {
	int length, size;
	length = patch->patch->border[0];
	if (patch->patch->border[start + 1] != 0) {
		size = 2 * patch->patch->border[start + 1];
		if (size < MERGEPATH_SIZE) {
			if (!dfs) {
				addNewMergePath(patch, start, size, left_mergepaths);
			}
			combineleft(patch, start, size, dfs);
		} else {
			fprintf(stderr, "Found a mergepath with lenght bigger than MERGEPATH_SIZE: %d. Increase MERGEPATH_SIZE if you want to run with these parameters.\n", size);
			exit(EXIT_FAILURE);
		}
	} else if (patch->patch->border[0] >= 2) {
		size = 2*patch->patch->border[(start+1) % length + 1] + 1;
		if (size < MERGEPATH_SIZE) {
			if (!dfs) {
				addNewMergePath(patch, start, size, left_mergepaths);
			}
			combineleft(patch, start, size, dfs);
		}
	}
}


void ismergepathright(struct patcheslist_element* patch, int start, unsigned char dfs) {
	int length, size;
	length = patch->patch->border[0];
	//There has to be a zero in front of the mergepath => max 3 pent
	if (patch->patch->border[(start - 1 + length) % length + 1] == 0) {
		//right mergepaths with form 0-X have size (2*X + 1) (nr of edges)
		size = 2 * patch->patch->border[(start + 1) % length + 1] + 1;
		if (size < MERGEPATH_SIZE) {
			if (!dfs) {
				addNewMergePath(patch, start, size, right_mergepaths);
			}
			combineright(patch, start, size, dfs);
		
		} else {
			fprintf(stderr, "Found a mergepath with lenght bigger than MERGEPATH_SIZE: %d. Increase MERGEPATH_SIZE if you want to run with these parameters.\n", size);
			exit(EXIT_FAILURE);
		}
		if (patch->patch->border[0] >= 4 && patch->patch->border[(start + 2) % length + 1] == 0) {
			//right mergepaths with form 0-X-0 have size (2*X + 2) (nr of edges)
			size = 2 * patch->patch->border[(start + 1) % length + 1] + 2;
			if (size < MERGEPATH_SIZE) {
				if (!dfs) {
					addNewMergePath(patch, start, size, right_mergepaths);
				}
				combineright(patch, start, size, dfs);
			} else {
				fprintf(stderr, "Found a mergepath with lenght bigger than MERGEPATH_SIZE: %d. Increase MERGEPATH_SIZE if you want to run with these parameters.\n", size);
				exit(EXIT_FAILURE);
			}
		}
	}

}