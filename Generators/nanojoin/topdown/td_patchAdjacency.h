#ifndef TD_PATCHADJACENCY_H
#define TD_PATCHADJACENCY_H

#include <stdio.h>
#include "td_common.h"

void newJoin(struct td_patch* patch);
void prepareWrite(int pent, int hex, int hept);
void finishUp();
void writePatchToFile(struct td_patch* patch);

#endif /* PATCHADJACENCY_H */