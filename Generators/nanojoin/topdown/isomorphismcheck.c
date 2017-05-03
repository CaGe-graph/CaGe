/*
 * isomorphismcheck.c
 *
 *  Created on: Mar 15, 2013
 *      Author: Dieter
 */

#include <stdlib.h>
#include <string.h>
#include "isomorphismcheck.h"
#include "td_common.h"

unsigned char doisomorphismcheck(vertextype*** adjacencyTable) {
    int i, j, count, length;
    vertextype previousvertex, temp, currentvertex;
    unsigned char valid, isgoodmark;
    unsigned char* goodmark;
    unsigned char* isomorphismstring;
    unsigned char* tempstring;
    goodmark = malloc(2 * (outsideparameters[0] + outsideparameters[1]) + 1);
    isomorphismstring = malloc((4 * (*adjacencyTable)[0][0] + 1) * sizeof(unsigned char));
    tempstring = malloc((4 * (*adjacencyTable)[0][0] + 1) * sizeof(unsigned char));
    if (tempstring == NULL || goodmark == NULL || isomorphismstring == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }
    count = 0;
    //goodmark is one for positions with a three
    goodmark[count++] = 2 * (outsideparameters[0] + outsideparameters[1]);
    if (outsideparameters[1] != 0) {
        goodmark[count++] = 0;
        goodmark[count++] = 1;
        for (i = 0; i < outsideparameters[0]; i++) {
            goodmark[count++] = 1;
            goodmark[count++] = 0;
        }
        for (i = 0; i < outsideparameters[1] - 1; i++) {
            goodmark[count++] = 0;
            goodmark[count++] = 1;
        }
    } else {
        for (i = 0; i < outsideparameters[0]; i++) {
            goodmark[count++] = 0;
            goodmark[count++] = 1;
        }
    }

    //last argument zero -> current string will not be used
    length = getString((*adjacencyTable)[0][1], (*adjacencyTable)[0][2], adjacencyTable, &isomorphismstring, 0, &isomorphismstring, 0);
    
    //normal
    valid = 1;
    if (checknormal) {
        //iterate over all vertices with degree 2
        for (j=0; j <= 1; j++) {
        for (i = 1; (i <= (*adjacencyTable)[0][0]) && valid; i++) {
            if ((*adjacencyTable)[i][2] == 0 && i != (*adjacencyTable)[0][1]) {
                //check if mark can be put here
                previousvertex = i;
                currentvertex = (*adjacencyTable)[i][j];
                count = 2;
                isgoodmark = 1;
                while (isgoodmark && currentvertex != i) {
                    if (goodmark[count] == ((*adjacencyTable)[currentvertex][2] != 0)) {
                        count++;
                        temp = currentvertex;
                        currentvertex = getSuccessor(&((*adjacencyTable)[currentvertex]), previousvertex);
                        previousvertex = temp;
                    } else {
                        isgoodmark = 0;
                    }
                }
                if (count - 1 == goodmark[0] && isgoodmark) {
                    //mark can be put here
                    valid = getString(i, (*adjacencyTable)[i][j], adjacencyTable, &tempstring, 0, &isomorphismstring, length) == 0;
                }
            }
        }}
    }

    if (valid == 1 && checkinverse) {
        //iterate over all vertices with degree 2
        for (j=0; j <= 1; j++) {
        for (i = 1; (i <= (*adjacencyTable)[0][0]) && valid; i++) {
            if ((*adjacencyTable)[i][2] == 0) {
                //check if mark can be put here
                previousvertex = i;
                currentvertex = (*adjacencyTable)[i][j];
                count = 2;
                isgoodmark = 1;
                while (isgoodmark && currentvertex != i) {
                    if (goodmark[count] == ((*adjacencyTable)[currentvertex][2] != 0)) {
                        count++;
                        temp = currentvertex;
                        currentvertex = getPredecessor(&((*adjacencyTable)[currentvertex]), previousvertex);
                        previousvertex = temp;
                    } else {
                        isgoodmark = 0;
                    }
                }
                if (count - 1 == goodmark[0] && isgoodmark) {
                    //mark can be put here
                    valid = getString(i, (*adjacencyTable)[i][j], adjacencyTable, &tempstring, 1, &isomorphismstring, length) == 0;
                }
            }
        }}
    }

    free(goodmark);
    free(tempstring);
    free(isomorphismstring);
    return valid;
}

/**
 * returns length of minstring
 */
int getMinString(vertextype*** adjacencyTable, unsigned char** result) {
    vertextype i;
    int templength;
    int length = 0;
    unsigned char* tempresult;
    (*result) = malloc((4 * (*adjacencyTable)[0][0] + 1) * sizeof(unsigned char));
    if ((*result) == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }
    tempresult = malloc((4 * (*adjacencyTable)[0][0] + 1) * sizeof(unsigned char));
    if (tempresult == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }
    for (i = 1; i <= (*adjacencyTable)[0][0]; i++) {
        //only have to start with vertices of degree 2 => string will start with (2,3,0), if degree 3 string would start with (2,3,4,0)
        if ((*adjacencyTable)[i][2] == 0) {
            templength = getString(i, (*adjacencyTable)[i][0], adjacencyTable, &tempresult, 0, result, length);
            if (templength != 0) {
                //found new minimal string => have to copy to current result
                memcpy(*result, tempresult, 4 * (*adjacencyTable)[0][0] + 1);
                length = templength;
            }
            templength = getString(i, (*adjacencyTable)[i][1], adjacencyTable, &tempresult, 0, result, length);
            if (templength != 0) {
                //found new minimal string => have to copy to current result
                memcpy(*result, tempresult, 4 * (*adjacencyTable)[0][0] + 1);
                length = templength;
            }
            templength = getString(i, (*adjacencyTable)[i][0], adjacencyTable, &tempresult, 1, result, length);
            if (templength != 0) {
                //found new minimal string => have to copy to current result
                memcpy(*result, tempresult, 4 * (*adjacencyTable)[0][0] + 1);
                length = templength;
            }
            templength = getString(i, (*adjacencyTable)[i][1], adjacencyTable, &tempresult, 1, result, length);
            if (templength != 0) {
                //found new minimal string => have to copy to current result
                memcpy(*result, tempresult, 4 * (*adjacencyTable)[0][0] + 1);
                length = templength;
            }
        }
    }
    free(tempresult);
    return length;
}

/*
 * Assume vertices are number 1...n => 0 is used as separation sign (smaller than all other numbers)
 * Type tells us if to use successor (type == 0) or succesor^(-1) (type == 1)
 */
int getString(vertextype a, vertextype b, vertextype*** adjacencyTable, unsigned char** result, unsigned char type, unsigned char** current, int currentlength) {
    int i;
    int v, y;
    int proceed, first;
    unsigned char* numbers;
    unsigned char* firstedge;
    unsigned char* verticeslist;
    int resultindex = 0, vlcindex = 0, vlendindex = 0;
    //0 will be used as infinity
    int nextnumber = 2;

    numbers = malloc((*adjacencyTable)[0][0] * sizeof(unsigned char));
    if (numbers == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }
    firstedge = malloc((*adjacencyTable)[0][0] * sizeof(unsigned char));
    if (firstedge == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < (*adjacencyTable)[0][0]; i++) {
        numbers[i] = 0;
    }

    verticeslist = malloc((*adjacencyTable)[0][0] * sizeof(unsigned char));
    if (verticeslist == NULL) {
        fprintf(stderr, "ERROR: malloc returned NULL\n");
        exit(EXIT_FAILURE);
    }

    verticeslist[vlendindex++] = a;
    firstedge[a - 1] = b;
    numbers[a - 1] = 1;

    proceed = 2;

    if (currentlength == 0) {
        proceed = 3;
    }

    //go further untill we processed all vertices from verticeslist and current result is not bigger than previous result (previous result is empty or it is not greater or we already had a smaller string (*current)[0] will have been set to zero)
    while (proceed >= 2 && vlendindex > vlcindex) {
        v = verticeslist[vlcindex++];
        y = firstedge[v - 1];
        first = 1;
        while (proceed >= 1 && (y != firstedge[v - 1] || first)) {
            first = 0;
            if (numbers[y - 1] == 0) {
                verticeslist[vlendindex++] = y;
                numbers[y - 1] = nextnumber++;
                firstedge[y - 1] = v;
            }
            (*result)[resultindex++] = numbers[y - 1];

            //check minimality
            if (proceed == 2 && (currentlength < resultindex - 1 || (*result)[resultindex - 1] < (*current)[resultindex - 1])) {
                proceed = 1;
            } else if (proceed == 2 && (*result)[resultindex - 1] > (*current)[resultindex - 1]) {
                proceed = 0;
            }

            if (!type) {
                y = getSuccessor(&((*adjacencyTable)[v]), y);
            } else {
                y = getPredecessor(&((*adjacencyTable)[v]), y);
            }
        }
        (*result)[resultindex++] = 0;
        //check minimality
        if (proceed == 2 && (currentlength < resultindex - 1 || (*result)[resultindex - 1] < (*current)[resultindex - 1])) {
            proceed = 1;
        } else if (proceed == 2 && (*result)[resultindex - 1] > (*current)[resultindex - 1]) {
            proceed = 0;
        }
    }
    free(verticeslist);
    free(numbers);
    free(firstedge);
    resultindex--;
    if (proceed == 1 || proceed == 2) {
        resultindex = 0;
    }
    return resultindex;
}

vertextype getSuccessor(vertextype** adjacencyrow, vertextype currentendvertex) {
    int i = 0;
    while (i < 3 && (*adjacencyrow)[i] != currentendvertex) {
        i++;
    }
    if (i > 2) {
        fprintf(stderr, "Error in isomorphismcheck (successor) endvertex is no neighbour of startvertex\n");
    } else {
        i = (i + 1) % 3;
        if ((*adjacencyrow)[i] == 0) {
            //startvertex has degree 2 and endvertex is last vertex in row => next vertex is at position 0
            i = 0;
        }
    }
    return (*adjacencyrow)[i];
}

vertextype getPredecessor(vertextype** adjacencyrow, vertextype currentendvertex) {
    int i = 0;
    while (i < 3 && (*adjacencyrow)[i] != currentendvertex) {
        i++;
    }
    if (i > 2) {
        fprintf(stderr, "Error in isomorphismcheck (predecessor) endvertex is no neighbour of startvertex\n");
    } else {
        //minus 1 %3 = + 2
        i = (i + 2) % 3;
        if ((*adjacencyrow)[i] == 0) {
            //startvertex has degree 2 and endvertex is first vertex in row => previous vertex is at position 1 (since vertex has degree 2)
            i = 1;
        }
    }
    return (*adjacencyrow)[i];
}

