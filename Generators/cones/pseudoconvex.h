/*
 *  pseudoconvex.h
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#ifndef _PSEUDOCONVEX_H //if not defined
#define _PSEUDOCONVEX_H

#include "util.h"

/*========== DATA STRUCTURES ===========*/

struct _innerspiral {
	int length;     /* the length of the code (i.e. the number of pentagons in the patch) */
	int position;   /* the current possition in the spiral code */
	int *code;      /* the array containing the actual code */
                        /* this code stores the number of hexagons between the pentagons */

};

typedef struct _innerspiral INNERSPIRAL;

/* data structure for an extended inner spiral fragment (implemented as a doubly-linked-list) */
struct frag {
	int faces;                     /* The number of faces in this fragment */
	boolean endsWithPentagon;      /* Is the last face in this fragment a pentagon */
	
	boolean isEnd;
	boolean isLayersFragment;
	
	struct frag *next;             /* The next fragment in the spiral */
	struct frag *prev;             /* The previous fragment in the spiral */
};

typedef struct frag FRAGMENT;

/* data structure for a shell (implemented as a doubly-linked-list) */
struct _shell {
	int size;
	FRAGMENT *start;
	
	boolean isActive;
	
	struct _shell *next;
	struct _shell *prev;
	
	int nrOfBreakEdges;
	int nrOfPossibleStartingPoints; //the number of extra possible starting points in clockwise direction

	//Usually there will be very few shells per case, but they will be reused quite often.
	//Therefore we already fix the following arrays to there maximum length. Otherwise they
	//will often need to be freed and reallocated.

	//there are at most five break-edges in a pseudoconvex patch
	 //we store the number of the face in the shell
	int breakEdge2FaceNumber[5];
	
	//there are at most five break-edges, so at most 4 extra starting points
	int startingPoint2BreakEdge[4];
	int startingPoint2FaceNumber[4]; //we store the number of the face in the shell
	
	int nrOfPossibleMirrorStartingPoints;
	
	//there are at most five break-edges, so at most 5 mirror starting points
	//these starting points are stilled stored in clockwise order
	int mirrorStartingPoint2BreakEdge[5];
	int mirrorStartingPoint2FaceNumber[5];
	
	int nrOfPentagons; //the number of pentagons that belong to this shell

        boolean nonCyclicShell;
};

typedef struct _shell SHELL;

/* data structure for a pseudoconvex patch */

struct _patch {
	int numberOfPentagons;
	int *boundary;
	INNERSPIRAL *innerspiral;
	FRAGMENT *firstFragment;
	SHELL *outershell;
        int numberOfLayers;
};

typedef struct _patch PATCH;

INNERSPIRAL *getNewSpiral(int numberOfPentagons);
FRAGMENT *addNewFragment(FRAGMENT *currentFragment);
FRAGMENT *createLayersFragment(FRAGMENT *currentFragment, int faces);
void freeFragment(FRAGMENT *fragment);
SHELL *addNewShell(SHELL *currentShell, int size, FRAGMENT *start);
void freeShell(SHELL *shell);

/*========== EXPORT ===========*/
void exportStatistics(PATCH *patch);
long getMaxVertices();
long getMinVertices();
void exportPlanarGraphCode(PATCH *patch, boolean includeHeader);
void exportPlanarGraphTable(PATCH *patch);
void exportInnerSpiral(PATCH *patch);
void exportExtendedInnerSpiral(PATCH *patch);
void exportShells(SHELL *shell);

/*========== CANONICITY ============*/
/**
 * Check whether this shell is canonical. Returns 0 when this shell is not canonical
 * and 1 if it is. If the shell is canonical, the possible startpoints for the next
 * shell are set.
 */
boolean checkShellCanonicity(PATCH *patch, SHELL *shell, SHELL *nextShell, int nrOfBreakEdges, int* boundarySides, int offset);
boolean checkNonCyclicShellCanonicity(SHELL *shell);

/*========== CONSTRUCTION ==========*/
void setIPRMode(boolean flag);
/**
 * Fills a patch with respectively 6-p break edges. The number of threes between each of the consequent break edges
 * is given by the parameters k*. A reference to the complete patch is passed by the parameter patch.
 * current contains a pointer to the new fragment of the extended spiral that needs to be determined by this method.
 * The integer shellCounter contains the number of faces that still need to be added to complete the current shell.
 * currentShell contains a pointer to the current shell that is being filled.
 * The integer shellStart indicates which of the breakedges corresponds to the first break edge that was started from
 * to start filling the current shell. (0 means the break edge before k1, ...)
 *
 * The booleans p(_i) are used for IPR reasons. It indicates whether a pentagon is allowed at the ith break-edge.
 * In the case of 0 pentagons left, we can't add anymore pentagons, so there is no need to keep track of this at that point.
 * The boolean lastWasPi is also used for IPR reasons. It indicates whether the previous operation was a P(i) operation.
 */
void fillPatch_5PentagonsLeft(int k, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, boolean p);
void fillPatch_4PentagonsLeft(int k1, int k2, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean lastWasPi);
void fillPatch_3PentagonsLeft(int k1, int k2, int k3, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean p3, boolean lastWasPi);
void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean p3, boolean p4, boolean lastWasPi);
void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean p3, boolean p4, boolean p5, boolean lastWasPi);
void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart);

#endif // end if not defined, and end the header file
