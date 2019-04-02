/*
 * patchtograph.h
 *
 *  Created on: Nov 21, 2012
 *      Author: Dieter
 */

#ifndef PATCHTOGRAPH_H_
#define PATCHTOGRAPH_H_

#include "bu_common.h"
#include <stdio.h>

typedef unsigned char vertextype;

//
//Build adjacency for simple pentagon, hexagon or heptagon
//
void bu_getSingleFaceAdjacency(int facesize, vertextype*** result);

/*
 * Get next/previous vertex from face
 * For nextbordervertex: if there is a face with border (a,b,c,d,e) and you want to know the sucessor vertex of b:
 * 	prevvertex = a
 * 	adjacencyrow = adjacencylist of b (such that adjaceny_list_b(i+1) = successor(adjacency_list_b(i)))
 * 	result = c
*/
vertextype bu_getNextBorderVertex(vertextype** adjacencyrow, vertextype prevvertex);
vertextype bu_getPreviousBorderVertex(vertextype** adjacencyrow, vertextype nextvertex);

/*
 * Merge one or two patches according to a translatetable
 * Translatetable[i] contains the new vertex identifier for the vertex with identifier i+1
 */
int bu_mergeOne(vertextype*** patch, vertextype** translatetable);
void bu_mergeTwo(vertextype*** left, vertextype*** result, vertextype** translatetable);

/*
 * Right: rcurrent = vertex that is located roffset vertices from the endvertex of the mark in the patch (clockwise), rprev = vertex before rcurrent (clockwise)
 * Left: lcurrent = vertex that is located loffset vertices from the endvertex of the mark in the patch (counterclockwise), lnext = vertex after lcurrent (clockwise)
 */
int bu_gotoStartLeft(vertextype*** patch, vertextype* lcurrent, vertextype* lnext, int loffset);
int bu_gotoStartRight(vertextype*** patch, vertextype* rcurrent, vertextype* rprev, int roffset);

/*
 * Move one vertex counterwise in the right patch, and one vertex counterclockwise in the left patch
 */
void bu_updateCurrentNext(vertextype*** left, vertextype*** right, vertextype* lcurrent, vertextype* lnext, vertextype* rprev, vertextype* rcurrent);

/*
 * Build translatetable for mergepad that starts at rcurrent in the right patch and lcurrent in the left patch
 * Translatetable[i] will contain the new vertex identifier for the vertex with identifier i+1
 *
 * Translatetable one -> paste a patch to itself (folding operation), there is only one patch
 * Translatetable two -> paste two patch onto eachother.
 *
 * For buildtranslatetableTwo, only translatetable of left patch is build, translatetable of right patch is identity function
 *
 * These methods also reset the mark of the patches
 */
int bu_buildTranslatetableOne(vertextype*** patch, vertextype lcurrent, vertextype lnext, vertextype rprev, vertextype rcurrent, vertextype** translatetable);
int bu_buildTranslatetableTwo(vertextype*** left, vertextype*** right, vertextype lcurrent, vertextype lnext, vertextype rprev, vertextype rcurrent, vertextype** translatetable);

/*
 * Build adjacencymatrix from patch
 * First row of the adjacencymatrix contains 3 elements: (nr_of_vertices, start_top_of_mark, end_top_of_mark)
 * Row i contains adjacencylist for vertex with identifier i, such that adjacencymatrix[i][j+1] = successor(adjacencymatrix[i][j])
 */
int bu_getPatchAdjacency(struct bu_patch* p, vertextype*** result);


void bu_adjacencyToFile(vertextype*** result, FILE* fp);
void bu_writeHeaderToFile(FILE* fp);
int bu_writeSingleToFile(struct bu_patch* patch, FILE* fp);

void bu_test_patchtoAdjacency();


#endif /* PATCHTOGRAPH_H_ */
