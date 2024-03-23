#ifndef TD_NANOJOIN_H_
#define TD_NANOJOIN_H_



 #include <stdio.h>
 
 #include "td_common.h"

void dfs(struct td_patch* patch);
void run_topdown(unsigned char pent, unsigned char hex, unsigned char hept, unsigned char nrofnanocap, int iv, int rings, unsigned char exactfaces);


#endif /* TD_NANOJOIN_H_ */