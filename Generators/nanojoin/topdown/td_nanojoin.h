/*
 * nanojoin.h
 *
 *      Author: Dieter
 */

#ifndef TD_NANOJOIN_H_
#define TD_NANOJOIN_H_



 #include <stdio.h>
 
 #include "td_common.h"

void dfs(struct td_patch* patch);
void run_topdown(unsigned char pent, unsigned char hex, unsigned char hept, int iv, int msec, int rings, unsigned char exactfaces);
int main(int argc, char const *argv[]);

#endif /* TD_NANOJOIN_H_ */