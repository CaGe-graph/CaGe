
#include "oldspiral2planar.h"
#include "util.h"
#include "pseudoconvex.h"
#include <stdlib.h>
#include <stdio.h>

long minVertices = -1;
long maxVertices = -1;

long getMaxVertices_impl(){
    return maxVertices;
}

long getMinVertices_impl(){
    return minVertices;
}

void exportStatistics_impl(PATCH *patch) {
    int pentagons = patch->numberOfPentagons;
    boolean symmetric;
    if (pentagons == 5) {
        symmetric = 1;
    } else {
        symmetric = patch->boundary[0] - patch->boundary[1] + 1;
    }
    int vertexCounter = 0;
    EDGE *start = createBoundary(patch->boundary[0] + patch->numberOfLayers, symmetric, pentagons, &vertexCounter);
    int code[pentagons];
    int i;
    code[0] = patch->innerspiral->code[0] + 1;
    for (i = 1; i < pentagons; i++) {
        code[i] = code[i - 1] + patch->innerspiral->code[i] + 1;
    }

    if (patchFromSpiralCode(start, code, pentagons, &vertexCounter)) {
        if(minVertices==-1 || minVertices>vertexCounter){
            minVertices = vertexCounter;
        }
        if(maxVertices < vertexCounter){
            maxVertices = vertexCounter;
        }
    } else {
        fprintf(stderr, "Error during export\n");
    }
    freeAllEdges();
}

void exportSpiralCode_impl(PATCH *patch, boolean humanReadable, boolean header) {
    int pentagons = patch->numberOfPentagons;
    boolean symmetric;
    if (pentagons == 5) {
        symmetric = 1;
    } else {
        symmetric = patch->boundary[0] - patch->boundary[1] + 1;
    }
    int vertexCounter = 0;
    EDGE *start = createBoundary(patch->boundary[0] + patch->numberOfLayers, symmetric, pentagons, &vertexCounter);
    int code[pentagons];
    int i;
    code[0] = patch->innerspiral->code[0] + 1;
    for (i = 1; i < pentagons; i++) {
        code[i] = code[i - 1] + patch->innerspiral->code[i] + 1;
    }

    if (patchFromSpiralCode(start, code, pentagons, &vertexCounter)) {
        if (humanReadable) {
            exportPlanarGraphTable_old(start, vertexCounter);
        } else {
            exportPlanarGraphCode_old(start, vertexCounter, header);
        }
        if(minVertices==-1 || minVertices>vertexCounter){
            minVertices = vertexCounter;
        }
        if(maxVertices < vertexCounter){
            maxVertices = vertexCounter;
        }
    } else {
        fprintf(stderr, "Error during export\n");
    }
    freeAllEdges();
}


/* int *EDGEsToPlanarGraph(EDGE *start, int maxVertex) */

/*
        translates the structure to which start belongs to the adjacency
        matrix of the graph. The result is a pointer to an array, containing
        4 entries for each vertex. At position 4*x will be the degree of the
        vertex x. And the neighbours of x will be at 4*x + i where i goes from
        1 to degree(x).
        This method also frees the memory used by the structure of EDGEs!!!
 */
int *EDGEsToPlanarGraph(EDGE *start, int maxVertex) {
    int* graph = malloc(sizeof (int) *4 * (maxVertex + 1));
    if(graph==NULL){
        fprintf(stderr, "Insufficient memory available. Aborting.\n");
        exit(1);
    }
    int marker[maxVertex + 1], i, j;

    for (i = 0; i < maxVertex + 1; i++) marker[i] = 0;

    EDGE *edges[maxVertex][3];
    for (i = 0; i < maxVertex; i++) {
        for (j=0; j<3; j++){
            edges[i][j]=NULL;
        }
    }

    EDGE * stack[maxVertex * 3];
    int stacksize;

    j = 0;
    stack[0] = start;
    stacksize = 1;
    marker[start->from] = 1;
    graph[(start->from * 4) + ++j] = start->to;
    if (start->inverse->left != NULL) {
        graph[(start->from * 4) + ++j] = start->inverse->left->to;
        stack[stacksize++] = start->inverse->left;
    }
    if (start->inverse->right != NULL) {
        graph[(start->from * 4) + ++j] = start->inverse->right->to;
        stack[stacksize++] = start->inverse->left;
    }
    graph[(start->from * 4) + 0] = j; //degree of start->from

    edges[start->from][0] = start;
    edges[start->from][1] = start->inverse->left;
    edges[start->from][2] = start->inverse->right;

    while (stacksize > 0) {
        EDGE *current = stack[--stacksize];
        //fprintf(stderr, "current edge (%d) \n", current);
        //fprintf(stderr, "from %2d to %2d \n", current->from, current->to);
        //fprintf(stderr, "inverse : %d\n", current->inverse);
        //fprintf(stderr, "right   : %d\n", current->right);
        //fprintf(stderr, "left    : %d\n", current->left);
        if (!marker[current->to]) {
            j = 0;
            marker[current->to] = 1;
            graph[(current->to * 4) + ++j] = current->from;
            if (current->left != NULL) {
                graph[(current->to * 4) + ++j] = current->left->to;
                if (!marker[current->left->to]) {
                    stack[stacksize++] = current->left;
                }
            }
            if (current->right != NULL) {
                graph[(current->to * 4) + ++j] = current->right->to;
                if (!marker[current->right->to]) {
                    stack[stacksize++] = current->right;
                }
            }
            graph[(current->to * 4) + 0] = j;

            edges[current->to][0] = current->inverse;
            edges[current->to][1] = current->left;
            edges[current->to][2] = current->right;
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
void computePlanarCode(unsigned char code[], int *length, EDGE *start, int maxVertex) {
    int *graph = EDGEsToPlanarGraph(start, maxVertex);
    int i, j;
    unsigned char *codeStart;

    codeStart = code;
    *code = (unsigned char) (maxVertex);
    code++;
    for (i = 1; i < maxVertex + 1; i++) {
        for (j = 1; j <= graph[i * 4 + 0]; j++) {
            *code = (unsigned char) (graph[i * 4 + j]);
            code++;
        }
        *code = 0;
        code++;
    }
    *length = code - codeStart;
    free(graph);
    return;
}

/* void computePlanarCodeShort(unsigned short code[], int *length, EDGE *start, int maxVertex) */

/*
        fills the array code with the planar code of the structure to which start belongs.
        length will contain the length of the code. The maximum number of vertices is limited
        to 65535.
 */
void computePlanarCodeShort(unsigned short code[], int *length, EDGE *start, int maxVertex) {
    int *graph = EDGEsToPlanarGraph(start, maxVertex);
    int i, j;
    unsigned short *codeStart;

    codeStart = code;
    *code = (unsigned short) (maxVertex);
    code++;
    for (i = 1; i < maxVertex + 1; i++) {
        for (j = 1; j <= graph[i * 4 + 0]; j++) {
            *code = (unsigned short) (graph[i * 4 + j]);
            code++;
        }
        *code = 0;
        code++;
    }
    *length = code - codeStart;
    free(graph);
    return;
}

/* void exportPlanarGraphCode(EDGE *start, int maxVertex) */

/*
        exports the given structure as planarcode on stdout.
 */
void exportPlanarGraphCode_old(EDGE *start, int maxVertex, boolean header) {
    int length;
    //TODO: better size for arrays?
    unsigned char code[maxVertex * 4 + 1];
    unsigned short codeShort[maxVertex * 4 + 1];
    static boolean first = TRUE;

    if (first && header) {
        fprintf(stdout, ">>planar_code<<");
        first = FALSE;
    }

    if (maxVertex + 1 <= 255) {
        computePlanarCode(code, &length, start, maxVertex);
        if (fwrite(code, sizeof (unsigned char), length, stdout) != length) {
            fprintf(stderr, "fwrite() failed -- exiting!\n");
            exit(-1);
        }
    } else if (maxVertex + 1 <= 65535){
        computePlanarCodeShort(codeShort, &length, start, maxVertex);
        putc(0, stdout);
        if (fwrite(codeShort, sizeof (unsigned short), length, stdout) != length) {
            fprintf(stderr, "fwrite() failed -- exiting!\n");
            exit(-1);
        }
    } else {
        fprintf(stderr, "Graph too large for planarcode -- exiting!\n");
        exit(-1);
    }
}

/* void exportPlanarGraphTable(EDGE *start, int maxVertex) */

/*
        exports the given structure as a human-readable table on stderr.
 */
void exportPlanarGraphTable_old(EDGE *start, int maxVertex) {
    int *graph = EDGEsToPlanarGraph(start, maxVertex);
    int i, j;

    fprintf(stderr, "Order is %d\n", (unsigned short) (maxVertex));

    for (i = 1; i < maxVertex + 1; i++) {
        fprintf(stderr, " %3d (%2d) |", i, graph[i * 4 + 0]);
        for (j = 1; j <= graph[i * 4 + 0]; j++) {
            fprintf(stderr, " %3d |", graph[i * 4 + j]);
        }
        fprintf(stderr, "\n");
    }
    free(graph);
}

/* EDGE *getNewEdge() */

/*
        returns a pointer to a new edge. Allocates them in blocks of 1.
 */
/*
EDGE *getNewEdge() {
    static EDGE *edge;
    static int available = 0;

    if (available == 0) {
        edge = (EDGE *) malloc(sizeof (EDGE)*1);
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
*/

EDGE *getNewEdge() {
    if(currentFreeEdge < edgesArraySize){
        EDGE *edge = edgesArray + currentFreeEdge;
        currentFreeEdge++;
        edge->inverse = NULL;
        edge->left = NULL;
        edge->right = NULL;
        edge->face_to_right = UNSET;
        edge->mark = 0;
        return edge;
    } else {
        fprintf(stderr, "Insufficient memory reserved for this amount of edges. Aborting.\n");
        fprintf(stderr, "Reserved number of edges: %d.\n", edgesArraySize);
        exit(1);
    }
}

void freeAllEdges() {
    currentFreeEdge = 0;
}

/* EDGE *createBoundary(int sside, int symmetric, int pentagons, int *vertexCounter) */

/*
        creates a new nearsymmetric or symmetric boundary with shortest
        side sside and 6 - pentagons breakedges. The values are not checked.
 */
EDGE *createBoundary(int sside, boolean symmetric, int pentagons, int *vertexCounter) {
    int i;

    //fprintf(stderr, "in boundary\n");

    EDGE *boundaryStart;
    EDGE *toConnect = getStraightPath(&boundaryStart, sside, vertexCounter, UNSET, OUTSIDE);
    int j = 0;
    if (!symmetric) j = 1;

    //fprintf(stderr, "after first path\n");

    for (i = 0; i < 6 - pentagons - 1; i++) {
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
EDGE *getStraightPath(EDGE **start, int length, int *vertexCounter, int rightFace, int leftFace) {
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

    for (i = 0; i < length; i++) {
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
int constructFaceToRight(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded) {
    int i = 0;

    //First return to the first edge that has to be part of this face
    EDGE *temp = start->inverse;
    *lastAdded = NULL;

    while (i++<size && temp->left != NULL) {
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
    //TODO: set face size and remove when face cannot be constructed
    i = 0;
    temp = temp->inverse;
    EDGE *newStart = temp;
    int startVertex = temp->from;
    while (i++<size && temp->right != NULL && temp->right != newStart) {
        temp = temp->right;
    }

    if (temp->right == newStart) { //we have found a complete face
        if (i == size) { //check whether face has correct size
            setFaceSizeToRight(size, newStart);
            return 1;
        } else {
            //there is a smaller face here already
            DEBUGMSG("Error: there is already a face here of wrong size.");
            return 0;
        }
    } else if (i >= size) {
        //i == size + 1 -> Only a larger face can be placed here
        //i == size -> there is a NULL, but we already have enough edges.
        DEBUGMSG("Error while constructing face: face of this size not possible here.");
        DEBUGDUMP(size, "%d")
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
        DEBUGMSG("Error while constructing face: face with innerbridge.");
        return 0;
    }

    //now add the remaining edges

    for (; i < size - 1; i++) {
        temp->right = getNewEdge();
        temp->right->face_to_right = size;
        temp->right->inverse = getNewEdge();
        temp->right->inverse->left = temp->inverse;
        temp->right->inverse->right = temp->left;
        temp->right->inverse->inverse = temp->right;
        if (temp->left != NULL) {
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
    if (temp->left != NULL)
        temp->left->inverse->left = temp->right;
    temp->right->right = newStart;
    newStart->inverse->left = temp->right->inverse;
    if (newStart->inverse->right != NULL) {
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
int constructFaceToRightNeighbourRestricted(int size, EDGE *start, int *vertexCounter, EDGE **lastAdded, int illegalNeighbour) {
    int i = 0;

    //First return to the first edge that has to be part of this face
    EDGE *temp = start->inverse;
    *lastAdded = NULL;

    while (i++<size && temp->left != NULL) {
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
    //TODO: set face size and remove when face cannot be constructed
    i = 0;
    temp = temp->inverse;
    if (temp->inverse->face_to_right == illegalNeighbour)
        return 0;
    EDGE *newStart = temp;
    int startVertex = temp->from;
    while (i++<size && temp->right != NULL && temp->right != newStart) {
        temp = temp->right;
        if (temp->inverse->face_to_right == illegalNeighbour)
            return 0;
    }

    if (temp->right == newStart) { //we have found a complete face
        if (i == size) { //check whether face has correct size
            setFaceSizeToRight(size, newStart);
            return 1;
        } else {
            //there is a smaller face here already
            DEBUGMSG("Error: there is already a face here of wrong size.");
            return 0;
        }
    } else if (i >= size) {
        //i == size + 1 -> Only a larger face can be placed here
        //i == size -> there is a NULL, but we already have enough edges.
        DEBUGMSG("Error while constructing face: face of this size not possible here.");
        DEBUGDUMP(size, "%d")
        return 0;
    }

    //now add the remaining edges

    for (; i < size - 1; i++) {
        temp->right = getNewEdge();
        temp->right->face_to_right = size;
        temp->right->inverse = getNewEdge();
        temp->right->inverse->left = temp->inverse;
        temp->right->inverse->right = temp->left;
        temp->right->inverse->inverse = temp->right;
        if (temp->left != NULL) {
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
    if (temp->left != NULL)
        temp->left->inverse->left = temp->right;
    temp->right->right = newStart;
    newStart->inverse->left = temp->right->inverse;
    if (newStart->inverse->right != NULL) {
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
void setFaceSizeToRight(int size, EDGE *start) {
    start->face_to_right = size;
    EDGE *temp = start->right;
    while (temp != start) {
        temp->face_to_right = size;
        temp = temp->right;
    }
}


/* int patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter) */

/*
        fills the given boundary according to the spiral code. At the end the remainder is filled with
        hexagons. If there occurs an error, 0 is returned. Otherwise 1 is returned.
 */
int patchFromSpiralCode(EDGE *boundaryStart, int *code, int pentagons, int *vertexCounter) {
    int i, j;
    EDGE *temp = boundaryStart; //temp needs to contain a valid pointer in case there are no hexagons added.
    EDGE *localStart = boundaryStart;
    int previousPentagon = 0;
    for (i = 0; i < pentagons; i++) {
        for (j = 0; j<*(code + i) - previousPentagon - 1; j++) {
            if (!constructFaceToRight(6, localStart, vertexCounter, &temp)) {
                DEBUGMSG("Error: hexagon could not be added")
                return 0;
            }
            temp = temp->inverse;
            while (temp->right == NULL) {
                temp = temp->left;
            }
            localStart = temp;
        }
        //-----------Add pentagon
        if (!constructFaceToRight(5, localStart, vertexCounter, &temp)) {
            DEBUGMSG("Error: pentagon could not be added");
            return 0;
        }
        //temp is last added edge
        //return to an edge that contains a left neighbour
        //but only if temp is not NULL
        if (temp != NULL) {
            temp = temp->inverse;
            while (temp->right == NULL) {
                temp = temp->left;
            }
            localStart = temp;
        }
        previousPentagon = *(code + i);
    }
    //------------Fill remainder with hexagons
    while (temp != NULL) {
        if (!constructFaceToRight(6, localStart, vertexCounter, &temp)) {
            DEBUGMSG("Error: hexagon could not be added\n");
            return 0;
        }
        //temp is last added edge
        //return to an edge that contains a left neighbour
        if (temp != NULL) {
            temp = temp->inverse;
            while (temp->right == NULL) {
                temp = temp->left;
            }
            localStart = temp;
        }
    }
    return 1;
}

/**
 * Reserves the memory for the maximum possible number of needed edges
 */
void initEdges(int sside, boolean symmetric, int pentagons, int hexagonLayers){
    int i;
    //calculate the number of hexagons and the length of the boundary
    int hexagons, boundary;
    if(symmetric) {
        switch (pentagons) {
            case 1:
                hexagons = 0;
                break;
            case 2:
                hexagons = (5*sside*sside+8*sside-4)/4;
                break;
            case 3:
                hexagons = (9*sside*sside+12*sside-20)/8;
                break;
            case 4:
                hexagons = (2*sside*sside+3*sside-12)/3;
                break;
            case 5:
                hexagons = (sside*sside+2*sside-28)/4;
                break;
            default:
                fprintf(stderr, "Illegal number of pentagons. Exiting\n");
                exit(-1);
        }
        //also add hexagons in extra layers
        for(i=sside+1; i<sside+1+hexagonLayers; i++){
            hexagons+=(6-pentagons)*i;
        }
        boundary = (6-pentagons)*(2*sside+1);
    } else {
        switch (pentagons) {
            case 2:
                hexagons = (5*sside*sside+18*sside+5)/4;
                break;
            case 3:
                hexagons = (9*sside*sside+24*sside-8)/8;
                break;
            case 4:
                hexagons = (2*sside*sside+5*sside-10)/3;
                break;
            default:
                fprintf(stderr, "Illegal number of pentagons. Exiting.\n");
                exit(-1);
        }
        //also add hexagons in extra layers
        for(i=sside+1; i<sside+1+hexagonLayers; i++){
            hexagons+=(6-pentagons)*(i+1)-1;
        }
        boundary = (6-pentagons)*(2*sside+3) - 2;
    }
    //calculate the number of directed edges, i.e. twice the number of edges
    int maxNumberOfEdges = 6*hexagons + 5*pentagons + boundary;
    edgesArray = (EDGE *) malloc(sizeof (EDGE)*maxNumberOfEdges);
    if(edgesArray == NULL){
        fprintf(stderr, "Insufficient memory available. Aborting.\n");
        exit(1);
    }
    currentFreeEdge = 0;
    edgesArraySize = maxNumberOfEdges;
}
