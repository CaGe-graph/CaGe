#ifndef BU_NANOJOIN_H_
#define BU_NANOJOIN_H_

#include "bu_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern struct patcheslist_element* new_patches;
extern struct patcheslist_element* current_patches;

void addPatchToNewList(struct bu_patch* newpatch, unsigned char pent, unsigned char hex, unsigned char hept, struct patcheslist_element** list);
void addPatchToList(struct bu_patch* newpatch, struct patcheslist_element** list);
void removeHead(struct patcheslist_element** list);

void processdfs(struct bu_patch* patch, unsigned char pent, unsigned char hex, unsigned char hept);
void processpatch(struct patcheslist_element* element, unsigned char dfs);
void nextiteration();

void run_bottomup(unsigned char pent, unsigned char hex, unsigned char hept, int maxivertices);


#endif /* BU_NANOJOIN_H_ */