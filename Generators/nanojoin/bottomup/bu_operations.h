/*
 * operations.h
 *
 *  Created on: Dec 24, 2012
 *      Author: Dieter
 */

#ifndef BU_OPERATIONS_H_
#define BU_OPERATIONS_H_

#include "bu_common.h"

int combine(struct bu_patch *rp, struct bu_patch *lp, int roffset, int loffset, int mergesize, struct bu_patch *combined);



#endif /* BU_OPERATIONS_H_ */