#ifndef BU_MERGEPATHS_H_
#define BU_MERGEPATHS_H_

#include "bu_common.h"

typedef unsigned char facecount[3];

struct mergepath {
	struct bu_patch* patch;
	unsigned char offset;
	struct mergepath* next;
};

struct mergepathlists {
	struct mergepath*** list;
	facecount* indexToFaces;
	struct mergepathlists* previous;
	struct mergepathlists* next;
};

void addNewMergePath(struct patcheslist_element* currentpatch, int offset, int length, struct mergepath*** list);
void getmergepathlists(unsigned char pent, unsigned char hex, unsigned char hept, struct mergepath*** type);
void free_mergepath(struct mergepath** list);

void ismergepathleft(struct patcheslist_element* patch, int start, unsigned char dfs);
void ismergepathright(struct patcheslist_element* patch, int start, unsigned char dfs);

void combineleft(struct patcheslist_element* patch, int start, int length, unsigned char dfs);
void combineright(struct patcheslist_element* patch, int start, int length, unsigned char dfs);


#endif /* BU_MERGEPATHS_H_ */