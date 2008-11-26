/*
 *  cone.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/09/08.
 *  
 *  Usage: cone (# pentagons) (shortest 'side') (n/s) [options]
 *  
 *  n gives you a nearsymmetric cone (only for 2,3 or 4 pentagons)
 *  s gives you a symmetric cone
 * 
 *  with the option -m mirror images of cones are considered to
 *  be non-isomorphic
 *
 *  returns a the number of canonical cones or a negative number in case of an error
 *
 * compile with gcc cone.c util.c twopentagons.c -o cone -Wall
 */
/*
   1 --> too few parameters
   3 --> illegal option
   4 --> too few or too many pentagons
   5 --> a negative side length
   6 --> 1 or 5 + nearsymetric
*/

#include "cone.h"
#include "util.h"
#include "twopentagons.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*
Some utility macros
*/
int markValue = 0;
#define MARKED(e) (e->mark == markValue)
#define MARK(e) (e->mark = markValue)
#define RESET_MARK(e, i) {if(markValue == INT_MAX){setAllMarksTo(0,e,i);markValue=1;}else{markValue++;}}

//set to 1 when only the number of structures needs to be counted
int onlyCount = 0;

/*
The minimum length of the (shortest) side for which there exists a
canonical cone patch.
*/
int symmetricMinima[5] = {0,1,1,2,5};
int symmetricMinimaIPR[5] = {0,1,2,4,9};
int nearsymmetricMinima[3] = {0,1,2};
int nearsymmetricMinimaIPR[3] = {1,2,4};

/* void setAllMarksTo(int value, EDGE *start, int maxVertex) */
/*
	traverses the complete structure starting from start and sets all the
	marks to value. maxVertex must contain the number of vertices (or more)
	so that a stack of the correct size can be made.
*/
void setAllMarksTo(int value, EDGE *start, int maxVertex){
	int marker[maxVertex+1], i, j;
	
	for(i=0;i<maxVertex+1;i++) marker[i]=0;
	
	EDGE *stack[maxVertex*3];
	int stacksize;
	
	j=0;
	stack[0] = start;
	stacksize = 1;
	marker[start->from]=1;
	start->mark = value;
	if(start->inverse->left!=NULL){
		start->inverse->left->mark = value;
		stack[stacksize++] = start->inverse->left;
	}
	if(start->inverse->right!=NULL){
		start->inverse->right->mark = value;
		stack[stacksize++] = start->inverse->left;
	}
	
	while(stacksize>0){
		EDGE *current = stack[--stacksize];
		if(!marker[current->to]){
			j=0;
			marker[current->to]=1;
			current->mark = value;
			if(current->left!=NULL){
				current->left->mark = value;
				if(!marker[current->left->to]){
					stack[stacksize++] = current->left;
				}
			}
			if(current->right!=NULL){
				current->right->mark = value;
				if(!marker[current->right->to]){
					stack[stacksize++] = current->right;
				}
			}
		}
	}

}

/* int *EDGEsToPlanarGraph(EDGE *start, int maxVertex) */
/*
	translates the structure to which start belongs to the adjacency
	matrix of the graph. The result is a pointer to an array, containing
	4 entries for each vertex. At position 4*x will be the degree of the
	vertex x. And the neighbours of x will be at 4*x + i where i goes from
	1 to degree(x).
*/
int *EDGEsToPlanarGraph(EDGE *start, int maxVertex){
	int* graph = malloc(sizeof(int)*4*(maxVertex+1));
	int marker[maxVertex+1], i, j;
	
	for(i=0;i<maxVertex+1;i++) marker[i]=0;
	
	EDGE *stack[maxVertex*3];
	int stacksize;
	
	j=0;
	stack[0] = start;
	stacksize = 1;
	marker[start->from]=1;
	graph[(start->from * 4) + ++j] = start->to;
	if(start->inverse->left!=NULL){
		graph[(start->from * 4) + ++j] = start->inverse->left->to;
		stack[stacksize++] = start->inverse->left;
	}
	if(start->inverse->right!=NULL){
		graph[(start->from * 4) + ++j] = start->inverse->right->to;
		stack[stacksize++] = start->inverse->left;
	}
	graph[(start->from * 4) + 0] = j; //degree of start->from
	
	while(stacksize>0){
		EDGE *current = stack[--stacksize];
		//fprintf(stderr, "current edge (%d) \n", current);
		//fprintf(stderr, "from %2d to %2d \n", current->from, current->to);
		//fprintf(stderr, "inverse : %d\n", current->inverse);
		//fprintf(stderr, "right   : %d\n", current->right);
		//fprintf(stderr, "left    : %d\n", current->left);
		if(!marker[current->to]){
			j=0;
			marker[current->to]=1;
			graph[(current->to * 4) + ++j] = current->from;
			if(current->left!=NULL){
				graph[(current->to * 4) + ++j] = current->left->to;
				if(!marker[current->left->to]){
					stack[stacksize++] = current->left;
				}
			}
			if(current->right!=NULL){
				graph[(current->to * 4) + ++j] = current->right->to;
				if(!marker[current->right->to]){
					stack[stacksize++] = current->right;
				}
			}
			graph[(current->to * 4) + 0] = j;
		}
	}

	return graph;
}

/* void computePlanarCode(unsigned char code[], int *length, EDGE *start, int maxVertex) */
/*
	fills the array code with the planar code of the structure to which start belongs.
	length will contain the length of the code. The maximum number of vertices is limited
	to 255.
*/
void computePlanarCode(unsigned char code[], int *length, EDGE *start, int maxVertex){
	int *graph = EDGEsToPlanarGraph(start, maxVertex);
	int i, j;
	unsigned char *codeStart;

	codeStart = code;
	*code = (unsigned char)(maxVertex);
	code++;
	for(i=1; i<maxVertex+1; i++){
		for(j=1; j<=graph[i*4 + 0]; j++){
			*code = (unsigned char)(graph[i*4 + j]);
			code++;
		}
		*code = 0;
		code++;
	}
	*length = code - codeStart;
	return;
}

/* void computePlanarCodeShort(unsigned short code[], int *length, EDGE *start, int maxVertex) */
/*
	fills the array code with the planar code of the structure to which start belongs.
	length will contain the length of the code. The maximum number of vertices is limited
	to 65535.
*/
void computePlanarCodeShort(unsigned short code[], int *length, EDGE *start, int maxVertex){
	int *graph = EDGEsToPlanarGraph(start, maxVertex);
	int i, j;
	unsigned short *codeStart;

	codeStart = code;
	*code = (unsigned short)(maxVertex);
	code++;
	for(i=1; i<maxVertex+1; i++){
		for(j=1; j<=graph[i*4 + 0]; j++){
			*code = (unsigned short)(graph[i*4 + j]);
			code++;
		}
		*code = 0;
		code++;
	}
	*length = code - codeStart;
	return;
}

/* void exportPlanarGraphCode(EDGE *start, int maxVertex) */
/*
	exports the given structure as planarcode on stdout.
*/
void exportPlanarGraphCode(EDGE *start, int maxVertex){
	if(onlyCount)
		return;
	int length;
	//TODO: better size for arrays?
	unsigned char code[maxVertex*4 + 1];
	unsigned short codeShort[maxVertex*4 + 1];
    static int first=1;
	
    if (first) { fprintf(stdout,">>planar_code<<"); first=0; }
		
	if(maxVertex+1 <= 255){
		computePlanarCode(code, &length, start, maxVertex);
		if (fwrite(code,sizeof(unsigned char),length,stdout) != length){
			fprintf(stderr,"fwrite() failed -- exiting!\n");
			exit(-1);
		}
	} else {
		computePlanarCodeShort(codeShort, &length, start, maxVertex);
	    putc(0,stdout);
		if (fwrite(codeShort,sizeof(unsigned short),length,stdout) != length){
			fprintf(stderr,"fwrite() failed -- exiting!\n");
			exit(-1);
		}
	}
}

#ifdef _DEBUG
/* void exportPlanarGraphTable(EDGE *start, int maxVertex) */
/*
	exports the given structure as a human-readable table on stderr.
*/
void exportPlanarGraphTable(EDGE *start, int maxVertex){
	int *graph = EDGEsToPlanarGraph(start, maxVertex);
	int i, j;

	fprintf(stderr, "Order is %d\n", (unsigned short)(maxVertex));
	
	for(i=1; i<maxVertex+1; i++){
		fprintf(stderr, " %3d (%2d) |", i, graph[i*4 + 0]);
		for(j=1; j<=graph[i*4 + 0]; j++){
			fprintf(stderr, " %3d |", graph[i*4 + j]);
		}
		fprintf(stderr, "\n");
	}
}
#endif

/* EDGE *getNewEdge() */
/*
	returns a pointer to a new edge. Allocates them in blocks of 1.
*/
EDGE *getNewEdge(){
	static EDGE *edge;
	static int available = 0;
	
	if(available==0){
		edge = (EDGE *)malloc(sizeof(EDGE)*1);
		available = 1;
	}
	
	available--;
	edge->inverse = NULL;
	edge->left = NULL;
	edge->right = NULL;
	edge->face_to_right = UNSET;
	edge->mark = 0;
	//fprintf(stderr, "returning new pointer %p\n", edge);
	return edge++;
}

/* EDGE *getNextBreakEdge(EDGE *breakEdge) */
/*
	returns the next break-edge in this boundary. No checks are made whether this is actually a closed boundary
*/
EDGE *getNextBreakEdge(EDGE *breakEdge){
	EDGE *temp = breakEdge->right;
	while(temp->left!=NULL)
		temp = temp->left->right;
	return temp;
}


/* EDGE *createBoundary(int sside, int symmetric, int pentagons, int *vertexCounter) */
/*
	creates a new nearsymmetric or symmetric boundary with shortest
	side sside and 6 - pentagons breakedges. The values are not checked.
*/
EDGE *createBoundary(int sside, int symmetric, int pentagons, int *vertexCounter){
	int i;

	//fprintf(stderr, "in boundary\n");

	EDGE *boundaryStart;
	EDGE *toConnect = getStraightPath(&boundaryStart, sside, vertexCounter, UNSET, OUTSIDE);
	int j = 0;
	if(!symmetric) j = 1;

	//fprintf(stderr, "after first path\n");
	
	for(i=0; i<6-pentagons-1; i++){
		//fprintf(stderr, "start of %d\n", i);
		EDGE *tempStart;
		(*vertexCounter)--; //the first vertex of this path is the last of the previous
		//fprintf(stderr, "before path\n");
		EDGE *tempConnect = getStraightPath(&tempStart, sside + j, vertexCounter, UNSET, OUTSIDE);
		//fprintf(stderr, "after path\n");
		//fprintf(stderr, "tempStart                : %d\n", tempStart);
		//fprintf(stderr, "tempStart->inverse       : %d\n", tempStart->inverse);
		toConnect->right = tempStart;
		tempStart->inverse->left = toConnect->inverse;
		toConnect = tempConnect;
		//fprintf(stderr, "end of %d\n", i);
	}

	toConnect->right = boundaryStart;
	boundaryStart->inverse->left = toConnect->inverse;
	(*vertexCounter)--; //the last vertex is removed, because the last vertex of the boundary is the first
	toConnect->to = boundaryStart->from;
	toConnect->inverse->from = boundaryStart->from;

	return boundaryStart;
}

/* EDGE *getStraightPath(EDGE **start, int length, int *vertexCounter) */
/*
	creates a straight path, starting with a right turn.
	start will contain the first edge of the path.
	length is the number of right turns or the number
	of left turns in the path. (These two are equal.)
*/
EDGE *getStraightPath(EDGE **start, int length, int *vertexCounter, int rightFace, int leftFace){
	int i;
	//fprintf(stderr, "start straight path\n");
	*start = getNewEdge();
	(*start)->face_to_right = rightFace;
	(*start)->inverse = getNewEdge();
	(*start)->inverse->face_to_right = leftFace;
	//fprintf(stderr, "added edge %d    (inverse: %d)\n", *start, (*start)->inverse);
	(*start)->inverse->inverse = *start;
	
	(*vertexCounter)++;
	(*start)->from = *vertexCounter;
	(*start)->inverse->to = *vertexCounter;
	(*vertexCounter)++;
	(*start)->to = *vertexCounter;
	(*start)->inverse->from = *vertexCounter;
	
	EDGE *last = *start;
	
	for(i=0; i<length; i++){
		EDGE *rightTurn = getNewEdge();
		rightTurn->face_to_right = rightFace;
		rightTurn->inverse = getNewEdge();
		rightTurn->inverse->face_to_right = leftFace;
		//fprintf(stderr, "added edge %d    (inverse: %d)\n", rightTurn, rightTurn->inverse);
		rightTurn->inverse->inverse = rightTurn;
		EDGE *leftTurn = getNewEdge();
		leftTurn->face_to_right = rightFace;
		leftTurn->inverse = getNewEdge();
		leftTurn->inverse->face_to_right = leftFace;
		//fprintf(stderr, "added edge %d    (inverse: %d)\n", leftTurn, leftTurn->inverse);
		leftTurn->inverse->inverse = leftTurn;
		last->right = rightTurn;
		rightTurn->left = leftTurn;
		leftTurn->inverse->right = rightTurn->inverse;
		rightTurn->inverse->left = last->inverse;
		rightTurn->from = last->to;
		rightTurn->inverse->to = last->to;
		(*vertexCounter)++;
		rightTurn->to = *vertexCounter;
		rightTurn->inverse->from = *vertexCounter;
		leftTurn->from = *vertexCounter;
		leftTurn->inverse->to = *vertexCounter;
		(*vertexCounter)++;
		leftTurn->to = *vertexCounter;
		leftTurn->inverse->from = *vertexCounter;
		last = leftTurn;
	}
	
	return last;
}

/* int constructFaceToRight(int size, EDGE *start, int *vertexCounter, EDGE *lastAdded) */
/*
	constructs a face of given size to the right of this edge
	returns 1 if such a face was made or already existed
	returns 0 if such a face cannot be made.
*/
int constructFaceToRight(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded){
	int i = 0;
	
	//First return to the first edge that has to be part of this face
	EDGE *temp = start->inverse;
	*lastAdded = NULL;

	while(i++<size && temp->left!=NULL){
		temp = temp->left;
	}

	/*
		we can already stop if we found too many faces
		TODO: check and re-add this control
		
	if(i==size+1 && temp!=start->inverse) {
		fprintf(stderr, "Error while constructing face\n");
		return 0;
	} else if (i==size+1) {
		fprintf(stderr, "Succes");
		return 1;
	}

	*/

	//start by progressing along the connection that are already made
	i = 0;
	temp = temp->inverse;
	EDGE *newStart = temp;
	int startVertex = temp->from;
	while(i++<size && temp->right!=NULL && temp->right != newStart){
		temp = temp->right;
	}
	
	if(temp->right == newStart){ //we have found a complete face
		if(i == size){ //check whether face has correct size
			setFaceSizeToRight(size, newStart);
			return 1;
		} else {
			//there is a smaller face here already
#ifdef _DEBUG
			fprintf(stderr, "Error: there is already a face here of wrong size.\n");
#endif
			return 0;
		}
	} else if(i>=size) {
		//i == size + 1 -> Only a larger face can be placed here
		//i == size -> there is a NULL, but we already have enough edges.
#ifdef _DEBUG
		fprintf(stderr, "Error while constructing face: face of size %d not possible here.\n", size);
#endif
		return 0;
	} else if (temp->left == newStart) {
		//There is a NULL but we have a face with a bridge to the inside
		/*    \_______/
		      /		  \
		   __/         \__
		     \  /      /
		      \/______/
		              \ 
		*/
#ifdef _DEBUG
		fprintf(stderr, "Error while constructing face: face with innerbridge.\n");
#endif
		return 0;
	}
	
	//now add the remaining edges
	
	for(;i<size-1;i++){
		temp->right = getNewEdge();
		temp->right->face_to_right = size;
		temp->right->inverse = getNewEdge();
		temp->right->inverse->left = temp->inverse;
		temp->right->inverse->right= temp->left;
		temp->right->inverse->inverse = temp->right;
		if(temp->left!=NULL){
			temp->left->inverse->left = temp->right;
		}
		temp->right->from = temp->to;
		temp->right->inverse->to = temp->to;
		(*vertexCounter)++;
		temp->right->to = *vertexCounter;
		temp->right->inverse->from = *vertexCounter;
		temp = temp->right;
	}
	
	//add the last edge and connect it to the start
	temp->right = getNewEdge();
	temp->right->face_to_right = size;
	temp->right->inverse = getNewEdge();
	temp->right->inverse->inverse = temp->right;
	temp->right->inverse->left = temp->inverse;
	temp->right->inverse->right = temp->left;
	if(temp->left!=NULL)
		temp->left->inverse->left = temp->right;
	temp->right->right = newStart;
	newStart->inverse->left = temp->right->inverse;
	if(newStart->inverse->right!=NULL){
		temp->right->left = newStart->inverse->right;
		newStart->inverse->right->inverse->right = temp->right->inverse;
	}
	temp->right->from = temp->to;
	temp->right->inverse->to = temp->to;
	temp->right->to = startVertex;
	temp->right->inverse->from = startVertex;
	*lastAdded = temp->right;

	setFaceSizeToRight(size, newStart);
	return 1;
}

/* int constructFaceToRightNeighbourRestricted(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded, int illegalNeighbour) */
/*
	constructs a face of given size to the right of this edge only if it doesn't have a neighbour of size illegalNeighbour.
	returns 1 if such a face was made or already existed
	returns 0 if such a face cannot be made.
*/
int constructFaceToRightNeighbourRestricted(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded, int illegalNeighbour){
	int i = 0;
	
	//First return to the first edge that has to be part of this face
	EDGE *temp = start->inverse;
	*lastAdded = NULL;

	while(i++<size && temp->left!=NULL){
		temp = temp->left;
	}

	/*
		we can already stop if we found too many faces
		TODO: check and re-add this control
		
	if(i==size+1 && temp!=start->inverse) {
		fprintf(stderr, "Error while constructing face\n");
		return 0;
	} else if (i==size+1) {
		fprintf(stderr, "Succes");
		return 1;
	}

	*/

	//start by progressing along the connection that are already made
	i = 0;
	temp = temp->inverse;
	if(temp->inverse->face_to_right==illegalNeighbour)
		return 0;
	EDGE *newStart = temp;
	int startVertex = temp->from;
	while(i++<size && temp->right!=NULL && temp->right != newStart){
		temp = temp->right;
		if(temp->inverse->face_to_right==illegalNeighbour)
			return 0;
	}
	
	if(temp->right == newStart){ //we have found a complete face
		if(i == size){ //check whether face has correct size
			setFaceSizeToRight(size, newStart);
			return 1;
		} else {
			//there is a smaller face here already
#ifdef _DEBUG
			fprintf(stderr, "Error: there is already a face here of wrong size.\n");
#endif
			return 0;
		}
	} else if(i>=size) {
		//i == size + 1 -> Only a larger face can be placed here
		//i == size -> there is a NULL, but we already have enough edges.
#ifdef _DEBUG
		fprintf(stderr, "Error while constructing face: face of size %d not possible here.\n", size);
#endif
		return 0;
	}
	
	//now add the remaining edges
	
	for(;i<size-1;i++){
		temp->right = getNewEdge();
		temp->right->face_to_right = size;
		temp->right->inverse = getNewEdge();
		temp->right->inverse->left = temp->inverse;
		temp->right->inverse->right= temp->left;
		temp->right->inverse->inverse = temp->right;
		if(temp->left!=NULL){
			temp->left->inverse->left = temp->right;
		}
		temp->right->from = temp->to;
		temp->right->inverse->to = temp->to;
		(*vertexCounter)++;
		temp->right->to = *vertexCounter;
		temp->right->inverse->from = *vertexCounter;
		temp = temp->right;
	}
	
	//add the last edge and connect it to the start
	temp->right = getNewEdge();
	temp->right->face_to_right = size;
	temp->right->inverse = getNewEdge();
	temp->right->inverse->inverse = temp->right;
	temp->right->inverse->left = temp->inverse;
	temp->right->inverse->right = temp->left;
	if(temp->left!=NULL)
		temp->left->inverse->left = temp->right;
	temp->right->right = newStart;
	newStart->inverse->left = temp->right->inverse;
	if(newStart->inverse->right!=NULL){
		temp->right->left = newStart->inverse->right;
		newStart->inverse->right->inverse->right = temp->right->inverse;
	}
	temp->right->from = temp->to;
	temp->right->inverse->to = temp->to;
	temp->right->to = startVertex;
	temp->right->inverse->from = startVertex;
	*lastAdded = temp->right;

	setFaceSizeToRight(size, newStart);
	return 1;
}

/* void setFaceSizeToRight(int size, EDGE *start) */
/*
	sets the size of the face to the right of this edge
	doesn't check whether there actually is such a face
	this should be done before calling this method.
	If there is no face to the right this method may/will
	fail.
 */
void setFaceSizeToRight(int size, EDGE *start){
    start->face_to_right = size;
    EDGE *temp = start->right;
	while(temp!=start){
		temp->face_to_right = size;
		temp = temp->right;
	}
}


/* int patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter) */
/*
	fills the given boundary according to the spiral code. At the end the remainder is filled with
	hexagons. If there occurs an error, 0 is returned. Otherwise 1 is returned.
*/
int patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter){
	int i, j;
	EDGE *temp = boundaryStart; //temp needs to contain a valid pointer in case there are no hexagons added.
	EDGE *localStart = boundaryStart;
	int previousPentagon = 0;
	for(i=0; i<pentagons; i++){
		for(j=0; j<*(code+i) - previousPentagon - 1; j++){
			if(!constructFaceToRight(6,localStart, vertexCounter, &temp)){
#ifdef _DEBUG
				fprintf(stderr, "Error: hexagon could not be added\n");
#endif
				return 0;
			}
			temp = temp->inverse;
			while(temp->right==NULL){
				temp = temp->left;
			}
			localStart = temp;
		}
		//-----------Add pentagon
		if(!constructFaceToRight(5,localStart, vertexCounter, &temp)){
#ifdef _DEBUG
			fprintf(stderr, "Error: pentagon could not be added\n");
#endif
			return 0;
		}
		//temp is last added edge
		//return to an edge that contains a left neighbour
		//but only if temp is not NULL
		if(temp!=NULL){
			temp = temp->inverse;
			while(temp->right==NULL){
				temp = temp->left;
			}
			localStart = temp;
		}
		previousPentagon = *(code+i);
	}
	//------------Fill remainder with hexagons
	while(temp!=NULL){
		if(!constructFaceToRight(6,localStart, vertexCounter, &temp)){
#ifdef _DEBUG
			fprintf(stderr, "Error: hexagon could not be added\n");
#endif
			return 0;
		}
		//temp is last added edge
		//return to an edge that contains a left neighbour
		if(temp!=NULL){
			temp = temp->inverse;
			while(temp->right==NULL){
				temp = temp->left;
			}
			localStart = temp;
		}
	}
	return 1;
}

/* void removeLastFace(EDGE *currentStart, int *vertexCounter) */
/*
	Removes the last face. currentStart is the first edge of the next face.
*/
void removeLastFace(EDGE *currentStart, int *vertexCounter){
	EDGE *temp;

	//first set size to UNSET
	setFaceSizeToRight(UNSET, currentStart->inverse);

	EDGE *toRemove = currentStart->inverse;
	currentStart->left->inverse->right = NULL;
	currentStart->right->inverse->left=NULL;
	
	while(toRemove->left==NULL){
		//store the next edge, remove the edge and proceed with next edge
		temp = toRemove->right;
		free(toRemove->inverse);
		free(toRemove);
		(*vertexCounter)--; //we also remove the last vertex
		toRemove = temp;
	}
	
	toRemove->left->inverse->right = NULL;
	toRemove->right->inverse->left = NULL;
	free(toRemove->inverse);
	free(toRemove);
}

/* int fillBoundary(EDGE *boundaryStart, EDGE *currentStart, int pentagonsLeft, int *vertexCounter, boolean IPR,
                    boolean mirror, int pentagons, int sside, boolean symmetric, int *spiralCode,
                    int numberOfStructures, EDGE **startPoints, int numberOfStartPoints, EDGE **mirrorStartPoints,
					int numberOfMirrorStartPoints){
*/
/*
	tries to fill the boundary by adding a pentagon or a hexagon and then calling itself.
*/
int fillBoundary(EDGE *boundaryStart, EDGE *currentStart, int pentagonsLeft, int *vertexCounter,
                 boolean IPR, boolean mirror, int pentagons, int sside, boolean symmetric, int *spiralCode,
                 int numberOfStructures, EDGE **startPoints, int numberOfStartPoints, EDGE **mirrorStartPoints,
				 int numberOfMirrorStartPoints){
	//check some bound criteria

	//first pentagon is checked seperately because this cuts larg parts early
	if(pentagonsLeft == pentagons){
		if(symmetric){
			if(!mirror && spiralCode[0]>halfFloor(sside)){
				return numberOfStructures;
			} else if(mirror && spiralCode[0] >= sside){
				return numberOfStructures;
			}
		} else if(!symmetric){
			if(!mirror && spiralCode[0]>halfCeil(sside + (sside+1)*(6-pentagons-1)) + halfFloor(sside)){
				return numberOfStructures;
			} else if(mirror && spiralCode[0] >= sside + (sside+1)*(6-pentagons-1)){
				//stop when first pentagon doesn't belong to boundary
				return numberOfStructures;
			}
		}
	} else if(pentagonsLeft == pentagons - 1 && !symmetric && !mirror){
		//this case can only be checked after the first pentagon in placed
		if(spiralCode[0] > halfFloor(sside) && spiralCode[0]<=sside)
			return numberOfStructures;
	}
	
	//as soon as one pentagon is placed we start to check whether the spiral code is canonical
	if(pentagonsLeft!=pentagons){
		int i;
		for(i = 0; i < numberOfStartPoints; i++){
			if(isSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft, startPoints[i], *vertexCounter))
				return numberOfStructures;
		}
		for(i = 0; i < numberOfMirrorStartPoints; i++){
			if(isMirrorSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft, mirrorStartPoints[i], *vertexCounter))
				return numberOfStructures;
		}
	}
	
	/*
	if(pentagonsLeft == pentagons - 1){
		if(!symmetric){
			if(spiralCode[0]>halfFloor(sside) && spiralCode[0] <= sside){
				return numberOfStructures;
			} else if (spiralCode[0]>2*sside+1) {
				return numberOfStructures;
			}
		}
	}
	if(pentagonsLeft == pentagons - 2){
		if(symmetric){
			if(spiralCode[0]==0){
				if(spiralCode[1]>halfFloor(sside*(6-pentagons)) && spiralCode[1]<sside*(6-pentagons)){
					return numberOfStructures;
				}
			} else {
				if(!(spiralCode[1]<sside-spiralCode[0] || spiralCode[1]>=sside*(6-pentagons) ||
				     (spiralCode[1]>=sside+spiralCode[0] && spiralCode[1]<2*sside)))
					return numberOfStructures;
			}
		}
	}
	*/

	//do the branching
	EDGE *temp;
	if(pentagonsLeft>0){
		if(IPR){
			if(constructFaceToRightNeighbourRestricted(5, currentStart, vertexCounter, &temp, 5)){
#ifdef _DEBUG
				fprintf(stderr, "Try pentagon\n");
#endif
				if(temp==NULL){
					//check whether this is a canonical patch
					int i;
					for(i = 0; i < numberOfStartPoints; i++){
						if(isSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft+1, startPoints[i], *vertexCounter)){
							setFaceSizeToRight(UNSET, currentStart);
							return numberOfStructures;
						}
					}
					for(i = 0; i < numberOfMirrorStartPoints; i++){
						if(isMirrorSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft+1, mirrorStartPoints[i], *vertexCounter)){
							setFaceSizeToRight(UNSET, currentStart);
							return numberOfStructures;
						}
					}
					//output, because this was the last face			
					printArray(spiralCode, pentagons);
					exportPlanarGraphCode(boundaryStart, *vertexCounter);			
#ifdef _DEBUG
					fflush(stdout);
#endif
					//remove the face
					setFaceSizeToRight(UNSET, currentStart);
					return numberOfStructures + 1;
				}
				temp = temp->inverse;
				while(temp->right==NULL){
					temp = temp->left;
				}
			
				//set next position of spiral code if this wasn't the last pentagon
				if(pentagonsLeft > 1)
					spiralCode[pentagons-pentagonsLeft + 1] = spiralCode[pentagons-pentagonsLeft] + 1;

				numberOfStructures = fillBoundary(boundaryStart, temp, pentagonsLeft - 1, vertexCounter,
													IPR, mirror, pentagons, sside, symmetric, spiralCode,
													numberOfStructures, startPoints, numberOfStartPoints,
													mirrorStartPoints, numberOfMirrorStartPoints);
				removeLastFace(temp, vertexCounter);
			}
		} else {
#ifdef _DEBUG
			fprintf(stderr, "Try to add pentagon (%d pentagons left)\n", pentagonsLeft);
#endif
			if(constructFaceToRight(5, currentStart, vertexCounter, &temp)){
#ifdef _DEBUG
				fprintf(stderr, "Succeeded to add pentagon (%d pentagons left)\n", pentagonsLeft-1);
#endif
				if(temp==NULL){
					//check whether this is a canonical patch
					int i;
					for(i = 0; i < numberOfStartPoints; i++){
						if(isSpiralCodeSmaller(spiralCode, pentagons, startPoints[i], *vertexCounter)){
							setFaceSizeToRight(UNSET, currentStart);
							return numberOfStructures;
						}
					}
					for(i = 0; i < numberOfMirrorStartPoints; i++){
						if(isMirrorSpiralCodeSmaller(spiralCode, pentagons, mirrorStartPoints[i], *vertexCounter)){
							setFaceSizeToRight(UNSET, currentStart);
							return numberOfStructures;
						}
					}
					//output, because this was the last face
					exportPlanarGraphCode(boundaryStart, *vertexCounter);
#ifdef _DEBUG
					fflush(stdout);
#endif
					//remove the face
					setFaceSizeToRight(UNSET, currentStart);
					return numberOfStructures + 1;
				}
				temp = temp->inverse;
				while(temp->right==NULL){
					temp = temp->left;
				}
			
				//set next position of spiral code if this wasn't the last pentagon
				if(pentagonsLeft > 1)
					spiralCode[pentagons-pentagonsLeft + 1] = spiralCode[pentagons-pentagonsLeft] + 1;

				numberOfStructures = fillBoundary(boundaryStart, temp, pentagonsLeft - 1, vertexCounter,
													IPR, mirror, pentagons, sside, symmetric, spiralCode,
													numberOfStructures, startPoints, numberOfStartPoints,
													mirrorStartPoints, numberOfMirrorStartPoints);
				removeLastFace(temp, vertexCounter);
			}
		}
#ifdef _DEBUG
		fprintf(stderr, "Try to add hexagon (%d pentagons left)\n", pentagonsLeft);
#endif
		if(constructFaceToRight(6, currentStart, vertexCounter, &temp)){
#ifdef _DEBUG
			fprintf(stderr, "Succeeded to add hexagon (%d pentagons left)\n", pentagonsLeft);
#endif
			if(temp==NULL){
				//error: there are still pentagons left
				//remove the face
				setFaceSizeToRight(UNSET, currentStart);
				return numberOfStructures;
			}
			temp = temp->inverse;
			while(temp->right==NULL){
				temp = temp->left;
			}

			//increase current position of spiral code
			spiralCode[pentagons-pentagonsLeft]++;
			
			numberOfStructures = fillBoundary(boundaryStart, temp, pentagonsLeft, vertexCounter, 
												IPR, mirror, pentagons, sside, symmetric, spiralCode,
												numberOfStructures, startPoints, numberOfStartPoints,
												mirrorStartPoints, numberOfMirrorStartPoints);
			removeLastFace(temp, vertexCounter);
		}
		return numberOfStructures;
	} else {
#ifdef _DEBUG
		fprintf(stderr, "There are no pentagons left: try to add hexagon\n");
#endif
		//no branching left: fill with hexagons
		if(constructFaceToRight(6, currentStart, vertexCounter, &temp)){
#ifdef _DEBUG
			fprintf(stderr, "Succeeded to add hexagon\n");
#endif
			if(temp==NULL){
				//check whether this is a canonical patch
				int i;
				for(i = 0; i < numberOfStartPoints; i++){
					if(isSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft, startPoints[i], *vertexCounter)){
						setFaceSizeToRight(UNSET, currentStart);
						return numberOfStructures;
					}
				}
				for(i = 0; i < numberOfMirrorStartPoints; i++){
					if(isMirrorSpiralCodeSmaller(spiralCode, pentagons - pentagonsLeft, mirrorStartPoints[i], *vertexCounter)){
						setFaceSizeToRight(UNSET, currentStart);
						return numberOfStructures;
					}
				}
				exportPlanarGraphCode(boundaryStart, *vertexCounter);			
#ifdef _DEBUG
				fflush(stdout);
#endif
				//remove the face
				setFaceSizeToRight(UNSET, currentStart);
				return numberOfStructures + 1;
			}
			temp = temp->inverse;
			while(temp->right==NULL){
				temp = temp->left;
			}
			numberOfStructures = fillBoundary(boundaryStart, temp, pentagonsLeft, vertexCounter,
												IPR, mirror, pentagons, sside, symmetric, spiralCode,
												numberOfStructures, startPoints, numberOfStartPoints,
												mirrorStartPoints, numberOfMirrorStartPoints);
			removeLastFace(temp, vertexCounter);
		}
		return numberOfStructures;
	}
}

/* boolean isSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices) */
/*
	Returns 1 if the spiralcode obtained by starting at alternateStart is smaller then the given spiralcode.
*/
boolean isSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices){
	RESET_MARK(alternateStart, numberOfVertices)
	int currentPosition = 0, currentValue=0;
	EDGE *edge = alternateStart;
	
	while(edge->face_to_right!=UNSET){
		//mark face
		EDGE *temp = edge;
		MARK(temp);
		temp = temp->right;
		//while(temp!=edge && temp!=NULL){
		while(temp!=edge){
			MARK(temp);
			temp = temp->right;
		}

		if(temp==NULL)
			return 0;

		//check to see if we found a pentagon
		if(edge->face_to_right==5){
			//does this mean we have found a smaller spiral code?
			if(spiralCode[currentPosition]>currentValue)
				return 1;
			//we found a pentagon so advance one position
			currentPosition++;
			//check to see if there are still pentagons left
			if(currentPosition >= currentLength)
				return 0;
		}
		//we advanced a face so the current value must increase by one
		currentValue++;

		if(spiralCode[currentPosition]<currentValue)
			//it is no longer possible to find a smaller spiral code in this case
			return 0;
		
		
		//progress to next edge
		//temp is equal to edge at this point
		temp = temp->right;
		while((temp->inverse->face_to_right==OUTSIDE || MARKED(temp->inverse)) && temp!=edge) temp = temp->right;
		
		// we should never have that temp is equal to edge,
		// because then we reached the last face and thus 
		// also the last pentagon and so we would have stopped
		// earlier. UNSET is possible because the last layer
		// need not be complete at this point.
		if(temp==edge || temp->inverse->face_to_right == UNSET)
			return 0;
		else
			edge = temp->inverse;
	}
	
	return 0;
}

/* boolean isMirrorSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices) */
/*
	Returns 1 if the spiralcode obtained by starting at alternateStart and going in counterclockwise direction
	is smaller then the given spiralcode.
*/
boolean isMirrorSpiralCodeSmaller(int *spiralCode, int currentLength, EDGE *alternateStart, int numberOfVertices){
	RESET_MARK(alternateStart, numberOfVertices)
	int currentPosition = 0, currentValue=0;
	EDGE *edge = alternateStart;
	
	while(edge->inverse->face_to_right!=UNSET){
		//mark face
		EDGE *temp = edge;
		MARK(temp);
		temp = temp->left;
		//while(temp!=edge && temp!=NULL){
		while(temp!=edge){
			MARK(temp);
			temp = temp->left;
		}

		if(temp==NULL)
			return 0;

		//check to see if we found a pentagon
		
		if(edge->inverse->face_to_right==5){
			//does this mean we have found a smaller spiral code?
			if(spiralCode[currentPosition]>currentValue)
				return 1;
			//we found a pentagon so advance one position
			currentPosition++;
			//check to see if there are still pentagons left
			if(currentPosition >= currentLength)
				// if length is x, then we must stop whencurrentPosition == x,
				// because the indices of the array run from 0
				return 0;
		}
		//we advanced a face so the current value must increase by one
		currentValue++;
		
		if(spiralCode[currentPosition]<currentValue)
			//it is no longer possible to find a smaller spiral code in this case
			return 0;
		
		//progress to next edge
		//temp is equal to edge at this point
		temp = temp->left;
		while((temp->face_to_right==OUTSIDE || MARKED(temp->inverse)) && temp!=edge) temp = temp->left;
		
		// we should never have that temp is equal to edge,
		// because then we reached the last face and thus 
		// also the last pentagon and so we would have stopped
		// earlier. UNSET is possible because the last layer
		// need not be complete at this point.
		if(temp==edge || temp->face_to_right == UNSET)
			return 0;
		else
			edge = temp->inverse;
	}
	
	return 0;
}

/*
//TODO: remove because incorrect
void mirroredSpiralCode(int *oldSpiralCode, int *newSpiralCode, int sside, int pentagons, boolean symmetric){
	int currentLayerSize, previousLayersSize = 0;
	int last_i = 0;
	int i, j;
	
	if(symmetric)
		currentLayerSize = sside*(6-pentagons);
	else
		;
		
	while(last_i<pentagons){
		i = last_i;

		//face at start of layer remains at the start
		if(oldSpiralCode[i] == previousLayersSize){
			newSpiralCode[i] = oldSpiralCode[i];
			i++;
			last_i = i;
		}

		while(i < pentagons && oldSpiralCode[i] < currentLayerSize + previousLayersSize) i++;
		
		for(j = i - 1; j >= last_i;j--){
			newSpiralCode[last_i+i-j-1]=previousLayersSize + 
						((currentLayerSize - (oldSpiralCode[j]-previousLayersSize))%currentLayerSize);
		}
		last_i = i;
		previousLayersSize += currentLayerSize;
		currentLayerSize -= (6 - (pentagons-last_i)); //(pentagons-last_i) is the number of remaining pentagons
		//TODO: last layer can be wrong size!!!
		//But, the result should be correct: check this
	}
}

//TODO: remove because incorrect
void shiftedSpiralCode(int *oldSpiralCode, int *newSpiralCode, int sside, int pentagons, boolean symmetric){
	int currentLayerSize, previousLayersSize = 0;
	int last_i = 0;
	int i, j;
	int currentShift;
	
	if(symmetric)
		currentLayerSize = sside*(6-pentagons);
	else
		;
		
	currentShift = sside;
	
	while(last_i<pentagons){
		i = last_i;

		while(i < pentagons && oldSpiralCode[i] < currentLayerSize + previousLayersSize) i++;
		
		int pentagonsInShift = 0;
		for(j = last_i; j < i;j++){
			newSpiralCode[j]=previousLayersSize + 
						((oldSpiralCode[j] - previousLayersSize + currentShift)%currentLayerSize);
			if(oldSpiralCode[j] - previousLayersSize < currentShift) pentagonsInShift++;
		}
		last_i = i;
		previousLayersSize += currentLayerSize;
		currentLayerSize -= (6 - (pentagons-last_i)); //(pentagons-last_i) is the number of remaining pentagons
		currentShift -= pentagonsInShift + 1;
		//TODO: last layer can be wrong size!!!
		//But, the result should be correct: check this
	}
}
*/

/* int main(int argc, char *argv[]) */
/*
	reads the command line arguments and starts the generation process
*/
int main(int argc, char *argv[])
{
	int c;
	int pentagons, sside;
	boolean symmetric, ipr=0, error = 0;
    int mirror = 0;
    char *name = argv[0];

	if(argc < 4) {
		fprintf(stderr,"Usage: %s (# pentagons) (shortest 'side') (n/s) [options] \n",name);
		fprintf(stderr,"For more information type: %s 0 0 s -h \n",name);
		exit(1);
	}

	pentagons = parseNumber(++argv);
	sside = parseNumber(++argv);
	switch (*((++argv)[0])) {
		case 's':
			symmetric = 1;
			break;
		case 'n':
			symmetric = 0;
			break;
		default:
			fprintf(stderr,"Usage: %s (# pentagons) (shortest 'side') (n/s) [options] \n",name);
			fprintf(stderr,"For more information type: %s 0 0 s -h \n",name);
			exit(2);
	}
	
	while (--argc > 3 && (*++argv)[0] == '-'){
		while ((c = *++argv[0]))
			switch (c) {
			case 'm':
				mirror = 1;
				break;
			case 'i':
				ipr = 1;
				break;
			case 'c':
				onlyCount = 1;
				break;
			case 'h':
				//print help
				fprintf(stderr, "The program %s calculates canonical conepatches.\n", name);
				fprintf(stderr, "Usage: %s (# pentagons) (shortest 'side') (n/s) [options] \n\n", name);
				fprintf(stderr, "Valid options:\n");
				fprintf(stderr, "  -h          : Print this help and return.\n");
				fprintf(stderr, "  -m          : Mirror-images are considered nonisomorphic.\n");
				fprintf(stderr, "  -i          : Use IPR-rule.\n");
				fprintf(stderr, "  -c          : Only count structures don't export them.\n");
				return 0;
			default:
				fprintf(stderr, "%s: illegal option %c\n", name, c);
				argc = 0;
				error = 1;
				break;
			}
	}
	
	if(error || argc != 3){
		fprintf(stderr,"Usage: %s (# pentagons) (shortest 'side') (n/s) [options] \n",name);
		fprintf(stderr,"For more information type: %s 0 0 s -h\n",name);
		return 3;
	}

	if(pentagons < 1 || pentagons > 5){
		fprintf(stderr, "A cone needs to have between 1 and 5 pentagons.\n");
		return 4;
	}

	if(sside < 0){
		fprintf(stderr, "The shortest side needs to have a positive length.\n");
		return 5;
	}

	if(!symmetric && (pentagons ==1 || pentagons == 5)){
		fprintf(stderr, "A cone with 1 or 5 pentagons cannot be nearsymmetric.\n");
		return 6;
	}

	//check the fixed minima lengths for sside
	if(symmetric){
		if(ipr){
			if(symmetricMinimaIPR[pentagons-1]>sside)
				return 0;
		} else {
			if(symmetricMinima[pentagons-1]>sside)
				return 0;
		}
	} else {
		if(ipr){
			if(nearsymmetricMinimaIPR[pentagons-2]>sside)
				return 0;
		} else {
			if(nearsymmetricMinima[pentagons-2]>sside)
				return 0;
		}
	}

	//in case of 1 pentagon there is also an upperbound
	if(pentagons == 1 && sside > 0){
		fprintf(stderr, "There are no canonical patches.\n");
		return 0;
	} else if (pentagons == 1) {
		//There is only one canonical cone patch with 1 pentagon and that is the pentagon itself
		fprintf(stderr, "There is one canonical patch.\n");
		int vertexCounter = 0;
		EDGE *boundaryStart = createBoundary(sside, symmetric, pentagons, &vertexCounter);
		fprintf(stderr, "vertices: %d\n", vertexCounter);
		exportPlanarGraphCode(boundaryStart, vertexCounter);
		fprintf(stderr, "Generated a total of 1 canonical cone patch.\n");
		return 0;
	}

	//in case of two pentagons we don't need the branch and bound
	if(pentagons == 2){
		int numberOfStructures = getTwoPentagonsPatch(sside,symmetric, mirror);
		fprintf(stderr, "Generated a total of %d canonical cone patches.\n", numberOfStructures);
		return 0;
	}

	//in all other cases we start the recursion
	int vertexCounter = 0;
	int spiralCode[pentagons];
	spiralCode[0] = 0;
	EDGE *boundaryStart = createBoundary(sside, symmetric, pentagons, &vertexCounter);
	
	//search all possible begin positions
	int variations;
	EDGE **startPoints;
	if(symmetric){
		variations = 6-pentagons-1;
		if(variations > 0){
			startPoints = (EDGE **)malloc(sizeof(EDGE *)*variations);
			int i;
			startPoints[0] = getNextBreakEdge(boundaryStart);
			for(i = 1; i<variations; i++){
				startPoints[i] = getNextBreakEdge(startPoints[i-1]);
			}
		}
	} else {
		variations = 0;
	}

	int mirrorVariations;
	EDGE **mirrorStartPoints;
	
	if(mirror){
		mirrorVariations = 0;
	} else {
		if(symmetric){
			mirrorVariations = 6-pentagons;
			mirrorStartPoints = (EDGE **)malloc(sizeof(EDGE *)*mirrorVariations);
			mirrorStartPoints[0] = boundaryStart->inverse;
			int i;
			for(i=0;i<mirrorVariations-1; i++)
				mirrorStartPoints[i+1] = startPoints[i]->inverse;
		} else {
			mirrorVariations = 1;
			mirrorStartPoints = (EDGE **)malloc(sizeof(EDGE *)*mirrorVariations);
			mirrorStartPoints[0] = getNextBreakEdge(boundaryStart)->inverse;
		}
	}
	int numberOfStructures = fillBoundary(boundaryStart, boundaryStart, pentagons, &vertexCounter, ipr,
											mirror, pentagons, sside, symmetric, spiralCode, 0, startPoints,
											variations, mirrorStartPoints, mirrorVariations);
	fprintf(stderr, "Generated a total of %d canonical cone patches.\n", numberOfStructures);
	return 0;
}
