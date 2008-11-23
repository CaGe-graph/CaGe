/*
 *  cone.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/09/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _CONE_H //if not defined
#define _CONE_H

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

typedef int boolean;

#define OUTSIDE 1
#define UNSET 0

EDGE *getNewEdge();

EDGE *getNextBreakEdge(EDGE *breakEdge);
EDGE *createBoundary(int sside, int symmetric, int pentagons, int *vertexCounter);
EDGE *getStraightPath(EDGE **start, int length, int *vertexCounter, int rightFace, int leftFace);
int constructFaceToRight(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded);
int constructFaceToRightNeighbourRestricted(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded, int illegalNeighbour);
void setFaceSizeToRight(int size, EDGE *start);
int patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter);

int fillBoundary(EDGE *boundaryStart, EDGE *currentStart, int pentagonsLeft, int *vertexCounter, boolean IPR, boolean mirror, int pentagons, int sside, boolean symmetric, int *spiralCode, int numberOfStructures, EDGE **startPoints, int numberOfStartPoints, EDGE **mirrorStartPoints, int numberOfMirrorStartPoints);
boolean isSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices);
boolean isMirrorSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices);

void mirroredSpiralCode(int *oldSpiralCode, int *newSpiralCode, int sside, int pentagons, boolean symmetric);
void shiftedSpiralCode(int *oldSpiralCode, int *newSpiralCode, int sside, int pentagons, boolean symmetric);

void exportPlanarGraphCode(EDGE *start, int maxVertex);
void exportPlanarGraphTable(EDGE *start, int maxVertex);
#endif // end if not defined, and end the header file
