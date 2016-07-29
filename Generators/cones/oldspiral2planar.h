/* 
 * File:   oldspiral2planar.h
 * Author: nvcleemp
 *
 * Created on August 12, 2009, 9:31 AM
 */

#ifndef _OLDSPIRAL2PLANAR_H
#define	_OLDSPIRAL2PLANAR_H

#include "util.h"
#include "pseudoconvex.h"

struct _edge {
	int from;              /* the vertex from where the edge starts */
	int to;                /* the vertex to where the edge goes */
	struct _edge *left;    /* the edge to the left when arriving in to */
	struct _edge *right;   /* the edge to the right when arriving in to */
	struct _edge *inverse; /* the inverse edge */
	int face_to_right;     /* the size of the face to the right of this edge*/
	                       /* special values are used for OUTSIDE and UNSET*/

	int mark;			   /* mark */
	int temp;			   /* free field */
};

typedef struct _edge EDGE;

EDGE *edgesArray;
int currentFreeEdge;
int edgesArraySize;

#define OUTSIDE 1
#define UNSET 0

long getMaxVertices_impl();
long getMinVertices_impl();
void exportStatistics_impl(PATCH *patch);

void exportSpiralCode_impl(PATCH *patch, boolean humanReadable, boolean header);

void exportPlanarGraphCode_old(EDGE *start, int maxVertex, boolean header);
void exportPlanarGraphTable_old(EDGE *start, int maxVertex);

EDGE *getNewEdge();
void freeAllEdges();

EDGE *createBoundary(int sside, boolean symmetric, int pentagons, int *vertexCounter);
EDGE *createBoundaryMoreSymmetric(int sside, int *vertexCounter);
EDGE *getStraightPath(EDGE **start, int length, int *vertexCounter, int rightFace, int leftFace);
int constructFaceToRight(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded);
int constructFaceToRightNeighbourRestricted(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded, int illegalNeighbour);
void setFaceSizeToRight(int size, EDGE *start);
boolean patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter);

void initEdges(int sside, boolean symmetric, int pentagons, int hexagonLayers);
void initEdges2PentagonsMoreSymmetric(int sside, int hexagonLayers);

#endif	/* _OLDSPIRAL2PLANAR_H */

