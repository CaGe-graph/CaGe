/*
 * isomorphismcheck.h
 *
 *  Created on: Mar 15, 2013
 *      Author: Dieter
 */

#ifndef ISOMORPHISMCHECK_H_
#define ISOMORPHISMCHECK_H_


#include "td_common.h"

void prepareIsomorphism(int maxvertices);
void finishUpIsomorphism();

unsigned char checkJoin(struct td_patch* patch);


#endif /* ISOMORPHISMCHECK_H_ */
