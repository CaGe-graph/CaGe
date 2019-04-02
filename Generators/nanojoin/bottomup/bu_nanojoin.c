#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bu_nanojoin.h"
#include "bu_extra.h"
#include "bu_mergepaths.h"
#include "bu_patchAdjacency.h"

#include "../layer.h"

struct patcheslist_element* new_patches;
struct patcheslist_element* current_patches;
int ITERATION_NUMBER;

/**
 * Remove the head from the linked list list
 */
void removeHead(struct patcheslist_element** list) {
	struct patcheslist_element* newhead = (*list)->next;
	free(*list);
	(*list) = newhead;
}

void removeMergepathListHead(struct mergepathlists** list) {
	struct mergepathlists* newhead = (*list)->next;
	free(*list);
	(*list) = newhead;
}

/**
 * Add newpatch to the linked list list
 * After execution of this method, newpatch will be the head of list
 */
void addPatchToList(struct bu_patch* newpatch, struct patcheslist_element** list) {
	struct patcheslist_element* newel = malloc(sizeof(struct patcheslist_element));
	if (newel == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	newel->patch = newpatch;
	newel->next = *list;
	(*list) = newel;
}


/**
 * Add newpatch to the linked list list
 * After execution of this method, newpatch will be the head of list
 */
void addPatchToNewList(struct bu_patch* newpatch, unsigned char pent, unsigned char hex, unsigned char hept, struct patcheslist_element** list) {
	struct patcheslist_element* newel = malloc(sizeof(struct patcheslist_element));
	if (newel == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	newel->patch = newpatch;
	newel->pent = pent;
	newel->hex = hex;
	newel->hept = hept;
	newel->next = (*list);
	(*list) = newel;
}

/*
 * Method called when a patch has to be processed dfs.
 * We create and delete a struct of the type patcheslist_element here
 * We also set the recursion parameter mergepathlist to the correct one
 */
void processdfs(struct bu_patch* patch, unsigned char pent, unsigned char hex, unsigned char hept) {
	int P;
	struct patcheslist_element* element = malloc(sizeof(struct patcheslist_element));
	if (element == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	//make sure we are working with the right version of mergepathlist
	struct mergepathlists* next;
	if (mergepathlist->next == NULL) {
		P = (maxpent + 1) * (maxhex + 1) * (maxhept + 1) + 1;
		next = malloc(sizeof(struct mergepathlists));
		if (next == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		next->next = NULL;
		next->previous = mergepathlist;
		next->list = malloc(P * sizeof(struct mergepath**));
		if (next->list == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		next->indexToFaces = malloc(P * sizeof(facecount));
		if (next->indexToFaces == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		mergepathlist->next = next;
	}
	mergepathlist = mergepathlist->next;
	//create patch and process it
	element->patch = patch;
	element->pent = pent;
	element->hex = hex;
	element->hept = hept;
	element->next = NULL;
	processpatch(element, 1);
	free(element);
	mergepathlist = mergepathlist->previous;
}

void processpatch(struct patcheslist_element* element, unsigned char dfs) {
	int i, length, partlength, facesum;
	unsigned char savemergepath;

	facesum = element->pent + element->hex + element->hept;
	if (facesum + ITERATION_NUMBER <= maxpent + maxhex + maxhept) {
		savemergepath = 1;
	} else {
		savemergepath = 0;
	}

	if (!dfs && !savemergepath) {
		//this is the beginning of the dfs algorithm => do not forget to free patch, element will be freed in removehead method
		//after processdfs has been finished patch must be deleted (normally this happens by overwriting the variable new in the combine methods, this does not happen here)
		processdfs(element->patch, element->pent, element->hex, element->hept);
		free(element->patch);
	} else {
		if (maxinternalvertices >= element->patch->internalvertices) {
			tree_add(element->patch->border, element->pent, element->hex, element->hept, element->patch->internalvertices);
			length = element->patch->border[0];
			partlength = find_repetition(element->patch->border);

			//should always be the case since max 3 pentagons
			if (length >= 3) {
				for (i=0; i< partlength; i++) {
					ismergepathleft(element, i, dfs);
					if (element->patch->border[i+1] == 0) {
						ismergepathright(element, i, dfs);
					}
				}
			}
		}
	}

}

void nextiteration() {
	ITERATION_NUMBER++;
	current_patches = new_patches;
	new_patches = NULL;
	while (current_patches != NULL) {
		processpatch(current_patches, 0);
		removeHead(&(current_patches));
	}
	if (new_patches != NULL) {
		nextiteration();
	}
}



void run_bottomup(unsigned char pent, unsigned char hex, unsigned char hept, int maxivertices) {
	int counter, P, i, j, k;
	struct bu_patch *pentagon, *hexagon, *heptagon;
	maxpent = pent;
	maxhex = hex;
	maxhept = hept;
	maxinternalvertices = maxivertices;
	P = (maxpent + 1) * (maxhex + 1) * (maxhept + 1) + 1;

	counter = 0;
	indextranslate = malloc((maxpent + 1) * sizeof(int**));
	faceswithmergepaths = malloc((maxpent + 1)* sizeof(unsigned char**));
	if (indextranslate == NULL || faceswithmergepaths == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i <= maxpent; i++) {
		indextranslate[i] = malloc((maxhex + 1) * sizeof(int*));
		faceswithmergepaths[i] = malloc((maxhex + 1)*sizeof(unsigned char*));
		if (indextranslate[i] == NULL || faceswithmergepaths[i] == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		for (j = 0; j <= maxhex; j++) {
			indextranslate[i][j] = malloc((maxhept + 1) * sizeof(int));
			faceswithmergepaths[i][j] = malloc((maxhept + 1) * sizeof(unsigned char));
			if (indextranslate[i][j] == NULL || faceswithmergepaths[i][j] == NULL) {
				fprintf(stderr, "ERROR: malloc returned NULL\n");
				exit(EXIT_FAILURE);
			}
			for (k = 0; k <= maxhept; k++) {
				indextranslate[i][j][k] = counter++;
				faceswithmergepaths[i][j][k] = 0;
			}
		}
	}

	//initialize mergepathlist
	mergepathlist = malloc(sizeof(struct mergepathlists));
	if (mergepathlist == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	mergepathlist->next = NULL;
	mergepathlist->previous = NULL;
	mergepathlist->list = malloc(P * sizeof(struct mergepath**));
	if (mergepathlist->list == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	mergepathlist->indexToFaces = malloc(P * sizeof(facecount));
	if (mergepathlist->indexToFaces == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}

	right_mergepaths = malloc(P * sizeof(struct mergepath**));
	if (right_mergepaths == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	left_mergepaths = malloc(P * sizeof(struct mergepath**));
	if (left_mergepaths == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < P; i++) {
		left_mergepaths[i] = malloc(MERGEPATH_SIZE * sizeof(struct mergepath*));
		right_mergepaths[i] = malloc(MERGEPATH_SIZE* sizeof(struct mergepath*));
		for (j = 0; j < MERGEPATH_SIZE; j++) {
			left_mergepaths[i][j] = NULL;
			right_mergepaths[i][j] = NULL;
		}
	}

	//initialize used linked lists
	new_patches = NULL;
	current_patches = NULL;

	//initialize iteration 0 patches (pentagon, hexagon, heptagon)
	pentagon = malloc(sizeof(struct bu_patch));
	if (pentagon == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	pentagon->border[0] = 5;
	for (i = 1; i <= pentagon->border[0]; i++) {
		pentagon->border[i] = 0;
	}
	pentagon->lp = NULL;
	pentagon->rp = NULL;

	hexagon = malloc(sizeof(struct bu_patch));
	if (hexagon == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	hexagon->border[0] = 6;
	for (i = 1; i <= hexagon->border[0]; i++) {
		hexagon->border[i] = 0;
	}
	hexagon->lp = NULL;
	hexagon->rp = NULL;

	heptagon = malloc(sizeof(struct bu_patch));
	if (heptagon == NULL) {
		fprintf(stderr, "ERROR: malloc returned NULL\n");
		exit(EXIT_FAILURE);
	}
	heptagon->border[0] = 7;
	for (i = 1; i <= heptagon->border[0]; i++) {
		heptagon->border[i] = 0;
	}
	heptagon->lp = NULL;
	heptagon->rp = NULL;

	pentagon->internalvertices = 0;
	hexagon->internalvertices = 0;
	heptagon->internalvertices = 0;

	if (maxpent > 0) {
		addPatchToNewList(pentagon, 1, 0, 0, &new_patches);
	} else {
		free(pentagon);
	}
	if (maxhex > 0) {
		addPatchToNewList(hexagon, 0, 1, 0, &new_patches);
	} else {
		free(hexagon);
	}
	if (maxhept > 0) {
		addPatchToNewList(heptagon, 0, 0, 1, &new_patches);
	} else {
		free(heptagon);
	}

	nextiteration();

	/*
	CLEANUP
	*/
	counter = 0;
	for (i=0; i < P; i++) {
		for (j = 0; j < MERGEPATH_SIZE; j++) {
			while (left_mergepaths[i][j] != NULL) {
				free_mergepath(&left_mergepaths[i][j]);
				counter++;
			}
			counter = 0;
			while (right_mergepaths[i][j] != NULL) {
				free_mergepath(&right_mergepaths[i][j]);
				counter++;
			}
		}
		free(left_mergepaths[i]);
		free(right_mergepaths[i]);
	}

	//
	//All patches to free are now in new_patches[0]
	//
	while (new_patches != NULL) {
		free(new_patches->patch);
		removeHead(&(new_patches));
	}

	free(left_mergepaths);
	free(right_mergepaths);

	while (mergepathlist != NULL) {
		free(mergepathlist->indexToFaces);
		free(mergepathlist->list);
		removeMergepathListHead(&mergepathlist);
	}

	for (i = 0; i <= maxpent; i++) {
		for (j = 0; j <= maxhex; j++) {
			free(indextranslate[i][j]);
		}
		free(indextranslate[i]);
	}
	free(indextranslate);

}