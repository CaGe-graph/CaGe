/*
 * isomorphismcheck.h
 *
 *  Created on: Mar 15, 2013
 *      Author: Dieter
 */

#ifndef ISOMORPHISMCHECK_H_
#define ISOMORPHISMCHECK_H_

#include "td_patchAdjacency.h"

unsigned char doisomorphismcheck(vertextype*** adjacencyTable);

int getString(vertextype a, vertextype b, vertextype*** adjacencyTable, unsigned char** result, unsigned char type, unsigned char** current, int currentlength);

vertextype getSuccessor(vertextype** adjacencyrow, vertextype currentendvertex);
vertextype getPredecessor(vertextype** adjacencyrow, vertextype currentendvertex);


#endif /* ISOMORPHISMCHECK_H_ */
