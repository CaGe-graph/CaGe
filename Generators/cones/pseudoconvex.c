/*
 *  pseudoconvex.c
 *  
 *
 *  Created by Nico Van Cleemput on 20/05/09.
 *
 */

#include "pseudoconvex.h"
#include "pseudoconvexuser.h"
#include "oldspiral2planar.h"

#include <stdio.h>
#include <stdlib.h>

/*========== DATA STRUCTURES ===========*/

INNERSPIRAL *getNewSpiral(int numberOfPentagons) {
    INNERSPIRAL *is = (INNERSPIRAL *) malloc(sizeof (INNERSPIRAL));
    is->code = malloc(sizeof (int) *(numberOfPentagons + 1)); //one extra for safety: fix later
    is->length = numberOfPentagons;
    is->position = 0;
    return is;
}

FRAGMENT *addNewFragment(FRAGMENT *currentFragment) {
    if (currentFragment == NULL) {
        FRAGMENT *fragment = (FRAGMENT *) malloc(sizeof (FRAGMENT));
        fragment->prev = fragment->next = NULL;
        fragment->isEnd = 0;
        fragment->isLayersFragment = 0;
        return fragment;
    } else {
        if (currentFragment->next == NULL) {
            FRAGMENT *fragment = (FRAGMENT *) malloc(sizeof (FRAGMENT));
            fragment->next = NULL;
            fragment->prev = currentFragment;
            currentFragment->next = fragment;
            fragment->isEnd = 0;
            fragment->isLayersFragment = 0;
            return fragment;
        } else {
            return currentFragment->next;
        }
    }
}

FRAGMENT *createLayersFragment(FRAGMENT *currentFragment, int faces) {
    FRAGMENT *fragment = addNewFragment(currentFragment);
    fragment->isLayersFragment = 1;
    fragment->faces = faces;
    fragment->endsWithPentagon = 0;
    return fragment;
}

void freeFragment(FRAGMENT *fragment) {
    if (fragment == NULL) {
        return;
    } else {
        freeFragment(fragment->next);
        free(fragment);
    }
}

SHELL *addNewShell(SHELL *currentShell, int size, FRAGMENT *start) {
    if (currentShell == NULL) {
        SHELL *shell = (SHELL *) malloc(sizeof (SHELL));
        shell->prev = shell->next = NULL;
        shell->size = size;
        shell->start = start;
        shell->isActive = 1;
        shell->nrOfPentagons = 0;
        shell->nonCyclicShell = 0;
        return shell;
    } else if (currentShell->next == NULL) {
        SHELL *shell = (SHELL *) malloc(sizeof (SHELL));
        shell->next = NULL;
        shell->prev = currentShell;
        currentShell->next = shell;
        shell->size = size;
        shell->start = start;
        shell->isActive = 1;
        shell->nrOfPentagons = 0;
        shell->nonCyclicShell = 0;
        return shell;
    } else {
        SHELL *shell = currentShell->next;
        shell->size = size;
        shell->start = start;
        shell->isActive = 1;
        shell->nrOfPentagons = 0;
        shell->nonCyclicShell = 0;
        return shell;
    }
}

void freeShell(SHELL *shell) {
    if (shell == NULL) {
        return;
    } else {
        freeShell(shell->next);
        free(shell);
    }
}

/*========== EXPORT ===========*/
void exportStatistics(PATCH *patch) {
    exportStatistics_impl(patch);
}

long getMaxVertices(){
    return getMaxVertices_impl();
}

long getMinVertices(){
    return getMinVertices_impl();
}

void exportPlanarGraphCode(PATCH *patch, boolean includeHeader) {
    exportSpiralCode_impl(patch, 0, includeHeader);
}

void exportPlanarGraphTable(PATCH *patch) {
    exportSpiralCode_impl(patch, 1, FALSE);
}

void exportOuterSpiral(PATCH *patch) {
    INNERSPIRAL* is = patch->innerspiral;
    if (is->length == 0) {
        fprintf(stdout, "patch without pentagons\n");
        return;
    }
    int i;
    for (i = 0; i < is->length - 1; i++)
        fprintf(stdout, "%d, ", is->code[i]);
    fprintf(stdout, "%d\n", is->code[is->length - 1]);
}

void exportExtendedOuterSpiral_impl(FRAGMENT* xis) {
    if (xis->isLayersFragment)
        fprintf(stdout, "|%d|", xis->faces - (xis->endsWithPentagon ? 1 : 0));
    else if (xis->faces - (xis->endsWithPentagon ? 1 : 0) != 0)
        fprintf(stdout, "(%d)", xis->faces - (xis->endsWithPentagon ? 1 : 0));
    if (xis->endsWithPentagon) fprintf(stdout, "(P) ");
    if (xis->next == NULL || xis->isEnd)
        fprintf(stdout, "\n");
    else
        exportExtendedOuterSpiral_impl(xis->next);
}

void exportExtendedOuterSpiral(PATCH *patch) {
    exportExtendedOuterSpiral_impl(patch->firstFragment);
}

void exportShells(SHELL *shell) {
    if (shell == NULL) return;

    SHELL *current = shell;
    while (current->prev != NULL)
        current = current->prev;

    while (current != shell) {
        if (current->start->isLayersFragment) {
            fprintf(stderr, "[%d]", current->size);
        } else {
            if (current->nonCyclicShell)
                fprintf(stderr, "[* %d:", current->size);
            else
                fprintf(stderr, "[%d:", current->size);
            int faces = current->size;
            FRAGMENT *fragment = current->start;
            while (faces > 0) {
                faces -= fragment->faces;
                if (fragment->faces - (fragment->endsWithPentagon ? 1 : 0) != 0)
                    fprintf(stderr, "(%d)", fragment->faces - (fragment->endsWithPentagon ? 1 : 0));
                if (fragment->endsWithPentagon) fprintf(stderr, "(P) ");
                fragment = fragment->next;
            }
            if (faces < 0) fprintf(stderr, "\n\nERROR\n\n");
            fprintf(stderr, "]");
        }
        current = current->next;
    }
    //export last shell
    if (current->size > 0) {
        fprintf(stderr, "[%d:", current->size);
        int faces = current->size;
        FRAGMENT *fragment = current->start;
        while (faces > 0 && fragment != NULL && (fragment->prev == NULL || !fragment->prev->isEnd)) {
            faces -= fragment->faces;
            if (fragment->faces - (fragment->endsWithPentagon ? 1 : 0) != 0)
                fprintf(stderr, "(%d)", fragment->faces - (fragment->endsWithPentagon ? 1 : 0));
            if (fragment->endsWithPentagon) fprintf(stderr, "(P) ");
            fragment = fragment->next;
        }
        if (faces < 0) fprintf(stderr, "\n\nERROR\n\n");

        fprintf(stderr, "]\n");
    } else {
        fprintf(stderr, "\n");
    }
}

/*========== CONSTRUCTION ==========*/

//should the ipr rule be applied
boolean iprMode = FALSE;

void setIPRMode(boolean flag){
    iprMode = flag;
}

#define HEXFRAG(frag, size) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 0;

#define PENTFRAG(frag, size, shell) \
	(frag)->faces = (size); \
	(frag)->endsWithPentagon = 1; \
	(shell)->nrOfPentagons++;

boolean checkShellCanonicity(PATCH *patch, SHELL *shell, SHELL *nextShell, int nrOfBreakEdges, int* boundarySides, int offset) {
    if (shell->nonCyclicShell) {
        return checkNonCyclicShellCanonicity(shell);
    }

    int i, j; //counter variables
    nextShell->nrOfBreakEdges = nrOfBreakEdges;

    //we start by calculating the breakEdge2FaceNumber for the next shell
    nextShell->breakEdge2FaceNumber[0] = 0;
    for (i = 1; i < nrOfBreakEdges; i++) {
        nextShell->breakEdge2FaceNumber[i] = nextShell->breakEdge2FaceNumber[i - 1] + boundarySides[i - 1];
    }
    if (nextShell->nonCyclicShell) {
        for (i = 1; i < nrOfBreakEdges; i++) {
            if (nextShell->breakEdge2FaceNumber[i] >= nextShell->size)
                nextShell->breakEdge2FaceNumber[i] = 2 * nextShell->size - 2 - nextShell->breakEdge2FaceNumber[i];
        }
    }

    //Then we handle some special cases in which we now that the shell is canonical
    if (shell->nrOfPentagons == 0) {
        //a shell without pentagons is always canonical
        nextShell->nrOfPossibleStartingPoints = shell->nrOfPossibleStartingPoints;
        DEBUGCONDITIONALMSG(nextShell->nrOfPossibleStartingPoints>nrOfBreakEdges,
                "Error in checkShellCanonicity: more starting points than break-edges")
        for (i = 0; i < shell->nrOfPossibleStartingPoints; i++) {
            nextShell->startingPoint2BreakEdge[i] = (shell->startingPoint2BreakEdge[i] + offset)%nrOfBreakEdges;
            DEBUGCONDITIONALMSG(nextShell->startingPoint2BreakEdge[i] > nrOfBreakEdges,
                    "Error in checkShellCanonicity: reference to non-existing break-edge")
            nextShell->startingPoint2FaceNumber[i] = nextShell->breakEdge2FaceNumber[nextShell->startingPoint2BreakEdge[i]];
        }
        nextShell->nrOfPossibleMirrorStartingPoints = shell->nrOfPossibleMirrorStartingPoints;
        DEBUGCONDITIONALMSG(nextShell->nrOfPossibleMirrorStartingPoints>nrOfBreakEdges,
                "Error in checkShellCanonicity: more mirror starting points than break-edges")
        for (i = 0; i < shell->nrOfPossibleMirrorStartingPoints; i++) {
            nextShell->mirrorStartingPoint2BreakEdge[i] = (shell->mirrorStartingPoint2BreakEdge[i] + offset)%nrOfBreakEdges;
            DEBUGCONDITIONALMSG(nextShell->mirrorStartingPoint2BreakEdge[i] > nrOfBreakEdges,
                    "Error in checkShellCanonicity: reference to non-existing break-edge")
            nextShell->mirrorStartingPoint2FaceNumber[i] = nextShell->breakEdge2FaceNumber[nextShell->mirrorStartingPoint2BreakEdge[i]];
        }
        return 1;
    } else if (shell->nrOfPossibleStartingPoints + shell->nrOfPossibleMirrorStartingPoints == 0) {
        //a shell without alternate starting points is also canonical
        nextShell->nrOfPossibleStartingPoints = nextShell->nrOfPossibleMirrorStartingPoints = 0;
        //no further values need to be set
        return 1;
    }
    //in all other cases we need to check if the current shell is canonical

    //First construct the code corresponding with the current shell
    int code[shell->nrOfPentagons];
    FRAGMENT *frag = shell->start;
    for (i = 0; i < shell->nrOfPentagons; i++) {
        code[i] = 0;
        while (!frag->endsWithPentagon) {
            code[i] += frag->faces;
            frag = frag->next;
        }
        code[i] += frag->faces - 1; //-1 because we don't count the pentagon
        frag = frag->next;
    }

    //a shell is canonical if the shell code is as large as possible
    //larger shell code means an early pentagon
    int shellCode[shell->size];

    for (i = 0; i < shell->size; i++) {
        shellCode[i] = 0;
    }

    int prevPentagon = 0;
    for (i = 0; i < shell->nrOfPentagons; i++) {
        shellCode[code[i] + prevPentagon] = 1;
        prevPentagon += code[i] + 1; // +1 because we also count the pentagon
    }

    //Start with checking all alternate starting points in clockwise direction
    boolean newPossibleStartingPoints[shell->nrOfPossibleStartingPoints];
    //stores which starting points are still possible for the next shell
    int newNrOfStartingPoints = 0; //nr of starting points for the next shell
    int startAt = 0;
    for (i = 0; i < shell->nrOfPossibleStartingPoints; i++) {
        startAt = shell->startingPoint2FaceNumber[i];

        int j;
        for (j = 0; j < shell->size; j++) {
            if (shellCode[j] < shellCode[(j + startAt) % shell->size]) {
                //we found a starting points which gives a larger shell code
                //i.e. the shell is not canonical
                return 0;
            } else if (shellCode[j] > shellCode[(j + startAt) % shell->size]) {
                //we found a starting points which gives a smaller shell code
                //i.e. we can disable this starting point for the next shell
                //and jump out of the for
                newPossibleStartingPoints[i] = 0;
                break;
            }
        }
        if (j == shell->size) {
            //we found neither a smaller nor a larger code
            newPossibleStartingPoints[i] = 1;
            newNrOfStartingPoints++;
        }
    }

    //Continue with checking all starting points in counterclockwise direction
    boolean newPossibleMirrorStartingPoints[shell->nrOfPossibleMirrorStartingPoints];
    //stores which starting points are still possible for the next shell
    int newNrOfMirrorStartingPoints = 0; //nr of starting points for the next shell
    startAt = 0;
    for (i = 0; i < shell->nrOfPossibleMirrorStartingPoints; i++) {
        startAt = shell->mirrorStartingPoint2FaceNumber[i];
        int j;
        for (j = 0; j < shell->size; j++) {
            if (shellCode[j] < shellCode[(-j + startAt + shell->size) % shell->size]) {
                //we found a starting points which gives a larger shell code
                //i.e. the shell is not canonical
                return 0;
            } else if (shellCode[j] > shellCode[(-j + startAt + shell->size) % shell->size]) {
                //we found a starting points which gives a smaller shell code
                //i.e. we can disable this starting point for the next shell
                //and jump out of the for
                newPossibleMirrorStartingPoints[i] = 0;
                break;
            }
        }
        if (j == shell->size) {
            //we found neither a smaller nor a larger code
            newPossibleMirrorStartingPoints[i] = 1;
            newNrOfMirrorStartingPoints++;
        }
    }


    //if we reach this, we didn't find a smaller code, so the shell is canonical
    //so set the possible starting points for the next shell and return 1.

    nextShell->nrOfPossibleStartingPoints = newNrOfStartingPoints;
    nextShell->nrOfPossibleMirrorStartingPoints = newNrOfMirrorStartingPoints;
    //we first calculate the relation between the old break-edges and the new
    //breakedges based on the position of the pentagons in this shell and the
    //position of the break-edges.
    int currentPentagonPosition = code[0];
    int currentPentagonCounter = 1;
    int extraBreakEdges = 0;
    int oldBreakEdge2NewBreakEdge[shell->nrOfBreakEdges];
    oldBreakEdge2NewBreakEdge[0] = 0 + offset;
    for (i = 1; i < shell->nrOfBreakEdges; i++) {
        while (currentPentagonPosition < shell->breakEdge2FaceNumber[i] &&
                currentPentagonCounter <= shell->nrOfPentagons) {
            extraBreakEdges++;
            if (currentPentagonCounter < shell->nrOfPentagons)
                currentPentagonPosition += code[currentPentagonCounter] + 1;
            //+1 because the code only counts the number of hexagons in between
            //the pentagons
            currentPentagonCounter++;
        }
        oldBreakEdge2NewBreakEdge[i] = (i + extraBreakEdges + offset)%nrOfBreakEdges;
    }

    //Next we store the new breakedges for the starting points that remain.
    j = 0;
    for (i = 0; i < shell->nrOfPossibleStartingPoints; i++) {
        if (newPossibleStartingPoints[i]) {
            nextShell->startingPoint2BreakEdge[j++] =
                    oldBreakEdge2NewBreakEdge[shell->startingPoint2BreakEdge[i]];
            DEBUGASSERT(j < newNrOfStartingPoints)
        }
    }
    j = 0;
    for (i = 0; i < shell->nrOfPossibleMirrorStartingPoints; i++) {
        if (newPossibleMirrorStartingPoints[i]) {
            nextShell->mirrorStartingPoint2BreakEdge[j++] =
                    oldBreakEdge2NewBreakEdge[shell->mirrorStartingPoint2BreakEdge[i]];
            DEBUGASSERT(j < newNrOfMirrorStartingPoints)
        }
    }

    //Finally we also calculate the maps startingPoint2FaceNumber and mirrorStartingPoint2FaceNumber
    for (i = 0; i < shell->nrOfPossibleStartingPoints; i++) {
        nextShell->startingPoint2FaceNumber[i] =
                nextShell->breakEdge2FaceNumber[nextShell->startingPoint2BreakEdge[i]];
    }
    for (i = 0; i < shell->nrOfPossibleMirrorStartingPoints; i++) {
        nextShell->mirrorStartingPoint2FaceNumber[i] =
                nextShell->breakEdge2FaceNumber[nextShell->mirrorStartingPoint2BreakEdge[i]];
    }
    return 1;
}

//a non cyclic shell is a shell whose inner face dual contains no cycle
boolean checkNonCyclicShellCanonicity(SHELL *shell) {
    int i; //counter variables

    //because this is a non cyclic shell there will be no next shell

    //Then we handle some special cases in which we now that the shell is canonical
    if (shell->nrOfPentagons == 0 || shell->nrOfPossibleStartingPoints + shell->nrOfPossibleMirrorStartingPoints == 0) {
        //a shell without pentagons or a shell alternate starting points is also canonical
        return 1;
    }
    //in all other cases we need to check if the current shell is canonical

    //First construct the code corresponding with the current shell
    int code[shell->nrOfPentagons];
    FRAGMENT *frag = shell->start;
    for (i = 0; i < shell->nrOfPentagons; i++) {
        code[i] = 0;
        while (!frag->endsWithPentagon) {
            code[i] += frag->faces;
            frag = frag->next;
        }
        code[i] += frag->faces - 1; //-1 because we don't count the pentagon
        frag = frag->next;
    }

    //a shell is canonical if the shell code is as large as possible
    //larger shell code means an early pentagon
    int shellCode[shell->size];

    for (i = 0; i < shell->size; i++) {
        shellCode[i] = 0;
    }

    int prevPentagon = 0;
    for (i = 0; i < shell->nrOfPentagons; i++) {
        shellCode[code[i] + prevPentagon] = 1;
        prevPentagon += code[i] + 1; // +1 because we also count the pentagon
    }

    //A non-cyclic shell only has two possible ways of considering its code:
    //the code itself and its reverse. We now check whether the reverse is greater.

    boolean reverseIsGreater = 0;
    i = 0;
    while (i < shell->size && shellCode[i] == shellCode[shell->size - 1 - i]) i++;
    reverseIsGreater = (i < shell->size && shellCode[i] < shellCode[shell->size - 1 - i]);


    //Start with checking all alternate starting points in clockwise direction
    for (i = 0; i < shell->nrOfPossibleStartingPoints; i++) {
        if (reverseIsGreater && shell->startingPoint2FaceNumber[i] == shell->size)
            return 0;
    }

    //Continue with checking all starting points in counterclockwise direction
    for (i = 0; i < shell->nrOfPossibleMirrorStartingPoints; i++) {
        if (reverseIsGreater && shell->mirrorStartingPoint2FaceNumber[i] == shell->size)
            return 0;
    }

    //if we reach this, we didn't find a smaller code, so the shell is canonical
    return 1;
}

void fillPatch_5PentagonsLeft(int k, PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, boolean p) {
    DEBUGDUMP(k, "%d")
    DEBUGMSG("=======")
    if (k <= 0)
        return;

    //shell handling
    if (shellCounter == 0) {
        currentShell = addNewShell(currentShell, shellCounter = k, current);
        int sides[1];
        sides[0] = k;
        if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 1, sides, 0))
            return;
    }

    INNERSPIRAL *is = patch->innerspiral;

    //add a side of hexagons
    is->code[is->position] += k;

    HEXFRAG(current, k)

    fillPatch_5PentagonsLeft(k - 1, patch, addNewFragment(current),
            shellCounter - k, currentShell, 1);
    is->code[is->position] -= k;

    //pentagon after i hexagons
    int i;
    int iStart = 0;
    if(iprMode && !p){
        //in case of ipr we check whether a pentagon may be placed at the break-edge
        iStart = 1;
    }
    for (i = iStart; i < k - 1; i++) {
        is->code[is->position] += i;
        is->position++;
        is->code[is->position] = 0;

        PENTFRAG(current, i + 1, currentShell)

        fillPatch_4PentagonsLeft(k - 2 - i, 1 + i, patch, addNewFragment(current),
                shellCounter - i - 1, currentShell,1, 0, 1, TRUE);
        currentShell->nrOfPentagons--;
        is->position--;
        is->code[is->position] -= i;
    }

    //pentagon after k-1 hexagons
    is->code[is->position] += k - 1;
    is->position++;
    is->code[is->position] = 0;

    PENTFRAG(current, k, currentShell)

    fillPatch_4PentagonsLeft(0, k - 2, patch, addNewFragment(current),
            shellCounter - k, currentShell,1, 0, 0, FALSE);
    currentShell->nrOfPentagons--;
    is->position--;
    is->code[is->position] -= k - 1;
}

void fillPatch_4PentagonsLeft(int k1, int k2, PATCH *patch, FRAGMENT *current,
        int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean lastWasPi) {
    DEBUGDUMP(k1, "%d")
    DEBUGDUMP(k2, "%d")
    DEBUGMSG("=======")
    if (k1 < 0 || k2 < 0 || (k1 == 0 && k2 == 0))
        return;

    //shell handling
    if (shellCounter == 0) {
        currentShell = addNewShell(currentShell, shellCounter = k1 + k2, current);
        int sides[2];
        sides[0] = k1;
        sides[1] = k2;
        if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 2,
                sides, shellStart))
            return;
        shellStart=0; //the new shell starts at the current position
    }

    INNERSPIRAL *is = patch->innerspiral;

    if (k1 == 0) {
        //only possibility: place a hexagon
        is->code[is->position] += 1;

        HEXFRAG(current, 1)

        fillPatch_4PentagonsLeft(k2 - 2, 1, patch, addNewFragment(current), 
                shellCounter - 1, currentShell, (shellStart + 1)%2, 1,
                !lastWasPi, FALSE);
        is->code[is->position] -= 1;
    } else if (k2 == 0) {
        //in this case it is not possible to have a pentagon at the break edge, so no IPR check is needed

        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_4PentagonsLeft(0, k1 - 3, patch, addNewFragment(current),
                shellCounter - k1 - 1, currentShell, (shellStart + 1)%2, 1,
                !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        for (i = 1; i < k1 - 1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_3PentagonsLeft(k1 - 2 - i, 0, i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 2 : shellStart, 0, 1, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }
    } else if (k2 == 1) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_4PentagonsLeft(0, k1 - 1, patch, addNewFragment(current), 
                shellCounter - k1 - 1, currentShell, (shellStart + 1)%2, 1,
                !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1 - 1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_3PentagonsLeft(k1 - 1 - i, 0, i + 1, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 2 : shellStart, 0, p2, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //a pentagon after k1-1 or k1 hexagons is not possible
    } else {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_4PentagonsLeft(k2 - 2, k1 + 1, patch, addNewFragment(current),
                shellCounter - k1 - 1, currentShell, (shellStart + 1)%2, 1, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_3PentagonsLeft(k1 - 1 - i, k2 - 1, 1 + i, patch,
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 2 : shellStart, 0, p2, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_3PentagonsLeft(0, k2 - 2, k1, patch, addNewFragment(current),
                    shellCounter - k1 - 1, currentShell,  shellStart==0 ? 2 : shellStart,
                    0, 0, !lastWasPi, FALSE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    }
}

void fillPatch_3PentagonsLeft(int k1, int k2, int k3, PATCH *patch, FRAGMENT *current,
        int shellCounter, SHELL *currentShell, int shellStart, boolean p1, boolean p2, boolean p3, boolean lastWasPi) {
    DEBUGDUMP(k1, "%d")
    DEBUGDUMP(k2, "%d")
    DEBUGDUMP(k3, "%d")
    DEBUGMSG("=======")
    if (k1 < 0 || k2 < 0 || k3 < 0)
        return;
    int zeroes = 0;
    if (k1 == 0) zeroes++;
    if (k2 == 0) zeroes++;
    if (k3 == 0) zeroes++;
    if (zeroes > 1) return;

    //shell handling
    if (shellCounter == 0) {
        currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + k3, current);
        int sides[3];
        sides[0] = k1;
        sides[1] = k2;
        sides[2] = k3;
        if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 3,
                sides, shellStart))
            return;
        shellStart=0; //the new shell starts at the current position
    }

    INNERSPIRAL *is = patch->innerspiral;

    if (k1 == 0) {
        //only possibility: place a hexagon
        is->code[is->position] += 1;

        HEXFRAG(current, 1)

        fillPatch_3PentagonsLeft(k2 - 1, k3 - 1, 1, patch, addNewFragment(current),
                shellCounter - 1, currentShell, (shellStart + 2)%3, 1, p3,
                !lastWasPi, FALSE);
        is->code[is->position] -= 1;
    } else if (k2 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_3PentagonsLeft(0, k3 - 2, k1, patch, addNewFragment(current), 
                shellCounter - k1 - 1, currentShell, (shellStart + 2)%3, 1, 1,
                !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1 - 1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_2PentagonsLeft(k1 - 1 - i, k2, k3 - 1, 1 + i, patch,
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 3 : shellStart, 0, p2, p3, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //a pentagon after k1-1 hexagons will lead to a patch boundary that cannot be filled (2 consecutive sides of length 0)
        //a pentagon after k1 hexagons is not possible
    } else if (k3 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_3PentagonsLeft(k2 - 2, k3, k1, patch, addNewFragment(current),
                shellCounter - k1 - 1, currentShell, (shellStart + 2)%3, 1, 1,
                !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        //in this case it is not possible to have a pentagon at the first (or last) break edge, so no IPR check is needed
        int i;
        for (i = 1; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_2PentagonsLeft(k1 - 1 - i, k2 - 1, k3, i, patch,
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 3 : shellStart, 0, p2, 1, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_2PentagonsLeft(0, k2 - 2, k3, k1 - 1, patch, 
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 3 : shellStart, 0, 0, 1, !lastWasPi, FALSE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    } else {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_3PentagonsLeft(k2 - 1, k3 - 1, k1 + 1, patch,
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 2)%3, 1, p3, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_2PentagonsLeft(k1 - 1 - i, k2, k3 - 1, 1 + i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 3 : shellStart, 0, p2, p3, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_2PentagonsLeft(0, k2 - 1, k3 - 1, k1, patch, 
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 3 : shellStart, 0, 0, p3, !lastWasPi, FALSE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    }
}

void specialCase0X0X(int X, PATCH *patch, FRAGMENT *current, SHELL *currentShell,
        boolean startAt0, int shellStart, boolean p1, boolean p2, boolean p3, boolean p4) {
    //This method is called when we have a shell with as boundary 0 X 0 X (X>1)
    //This type of shell can be non cyclic in which case it is a strip of hexagons
    //bounded by a pentagon at each side. In the other case it is a non cyclic
    //shell that arrives from performing Endo-Kroto-C2 insertion operations
    //on the first case.
    //As example the case 0 2 0 2 is shown below:
    //there are two possible fillings: one with 3 faces that is noncyclic
    //and one with 4 faces that is cyclic.
    //       /\               //
    //   __ /  \ __           //
    //  /  |    |  \          //
    //  \__|    |__/          //
    //      \  /              //
    //       \/               //
    // or
    //       /\               //
    //   __ /  \ __           //
    //  /   \__/   \          //
    //  \__ /  \ __/          //
    //      \  /              //
    //       \/               //

    SHELL *nextShell;
    FRAGMENT *secondFragment;

    int sides[4];
    if(startAt0){
        sides[0] = 0;
        sides[1] = X;
        sides[2] = 0;
        sides[3] = X;
    } else {
        sides[0] = X;
        sides[1] = 0;
        sides[2] = X;
        sides[3] = 0;
    }

    //This method is only called when shellCounter is 0, so we need to start a
    //new shell. First we handle the non cyclic shell
    //The non cyclic shell is in IPR-mode only possible if a pentagon is allowed at each break-edge
    INNERSPIRAL *is = patch->innerspiral;

    if(!iprMode || (p1 && p2 && p3 && p4)){
        nextShell = addNewShell(currentShell, X + 1, current);
        nextShell->nonCyclicShell = 1;

        if (!checkShellCanonicity(patch, currentShell, nextShell, 4, sides, shellStart))
            return;
        is->position++;
        is->code[is->position] = 0;

        PENTFRAG(current, 1, nextShell)
        secondFragment = addNewFragment(current);
        is->code[is->position] += X - 1;
        is->position++;
        is->code[is->position] = 0;
        PENTFRAG(secondFragment, X, nextShell)

        if (validateStructure(patch)) {
            secondFragment->isEnd = 1;
            processStructure(patch, nextShell);
            secondFragment->isEnd = 0;
        }

        nextShell->nrOfPentagons--;
        nextShell->nrOfPentagons--;
        is->position--;
        is->code[is->position] -= X - 1;
        is->position--;
    }

    //Then we handle the other cases
    //This case will never place a pentagon at any of the break-edges, so no IPR check is needed.
    nextShell = addNewShell(currentShell, 2 * X, current);

    //TODO: correct offset for canonicity check
    if (!checkShellCanonicity(patch, currentShell, nextShell, 4, sides, shellStart))
        return;

    if(startAt0){
        is->code[is->position] += 1;

        HEXFRAG(current, 1)

        fillPatch_2PentagonsLeft(X - 1, 0, X - 1, 1, patch, addNewFragment(current),
                2 * X - 1, nextShell, 3, 1, p3, p4, 1, FALSE);
        is->code[is->position] -= 1;
    } else {
        //first we add a side of hexagons
        is->code[is->position] += X+1;

        HEXFRAG(current, X+1)

        fillPatch_2PentagonsLeft(0, X - 2, 0, X-1, patch, addNewFragment(current),
                2 * X - 1, nextShell, 3, 1, 1, 1, 1, FALSE);
        is->code[is->position] -= X+1;

        //next we add a pentagon after i hexagons with 0<i<X
        //i can't be 0 because that would recreate the noncyclic case
        //i can't be X because that would be an invalid structure
        int i;
        for (i = 1; i < X; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_1PentagonLeft(X - 1 - i, 0, X - 1, 0, i, patch,
                    addNewFragment(current), 2*X - i - 1, currentShell, 4,
                    0, p2, p3, p4, 1, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }
    }
}

void fillPatch_2PentagonsLeft(int k1, int k2, int k3, int k4, PATCH *patch, 
        FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart,
        boolean p1, boolean p2, boolean p3, boolean p4, boolean lastWasPi) {
    DEBUGDUMP(k1, "%d")
    DEBUGDUMP(k2, "%d")
    DEBUGDUMP(k3, "%d")
    DEBUGDUMP(k4, "%d")
    DEBUGMSG("=======")
    if (k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0)
        return;
    if ((k1 == 0 && k2 == 0) || (k2 == 0 && k3 == 0) || (k3 == 0 && k4 == 0) || (k4 == 0 && k1 == 0))
        return;

    //shell handling
    if (shellCounter == 0) {
        if ((k1 == 0 && k3 == 0 && k2 == k4 && k4 > 1) || (k2 == 0 && k4 == 0 && k1 == k3 && k3 > 1)) {
            specialCase0X0X(k1 + k2, patch, current, currentShell, k1==0, shellStart, p1, p2, p3, p4);
            return;
        } else if ((k1 == 0 && k3 == 0 && k2 == k4) || (k2 == 0 && k4 == 0 && k1 == k3)) {
            //the size is either k1 + 1 == k3 + 1 or k2 + 1 == k4 + 1
            currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + 1, current);
            currentShell->nonCyclicShell = 1;
        } else {
            currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + k3 + k4, current);
        }
        int sides[4];
        sides[0] = k1;
        sides[1] = k2;
        sides[2] = k3;
        sides[3] = k4;
        if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 4, sides, shellStart))
            return;
        shellStart=0; //the new shell starts at the current position
    }

    INNERSPIRAL *is = patch->innerspiral;

    if (k2 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_2PentagonsLeft(0, k3 - 1, k4 - 1, k1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 3)%4, 1, 1, p4, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_1PentagonLeft(k1 - 1 - i, k2, k3, k4 - 1, 1 + i, patch,
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 4 : shellStart, 0, p2, p3, p4, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //a pentagon after k1 hexagons is not possible
    } else if (k4 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_2PentagonsLeft(k2 - 1, k3 - 1, k4, k1, patch,
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 3)%4, 1, p3, 1, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !(p1 && p4)){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            //because k4 is 0 we also need to check whether the last break-edge may lie at a pentagon
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_1PentagonLeft(k1 - 1 - i, k2, k3 - 1, k4, i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 4 : shellStart, 0, p2, p3, 1, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_1PentagonLeft(0, k2 - 1, k3 - 1, k4, k1 - 1, patch, 
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 4 : shellStart, 0, 0, p3, 1, !lastWasPi, FALSE);
            is->position--;
            is->code[is->position] -= k1;
        }
    } else {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_2PentagonsLeft(k2 - 1, k3, k4 - 1, k1 + 1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 3)%4, 1, p3, p4, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_1PentagonLeft(k1 - 1 - i, k2, k3, k4 - 1, 1 + i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 4 : shellStart, 0, p2, p3, p4, !lastWasPi, TRUE);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_1PentagonLeft(0, k2 - 1, k3, k4 - 1, k1, patch, 
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 4 : shellStart, 0, 0, p3, p4, !lastWasPi, FALSE);

            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    }
}

void fillPatch_1PentagonLeft(int k1, int k2, int k3, int k4, int k5, PATCH *patch,
        FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart,
        boolean p1, boolean p2, boolean p3, boolean p4, boolean p5, boolean lastWasPi) {
    DEBUGDUMP(k1, "%d")
    DEBUGDUMP(k2, "%d")
    DEBUGDUMP(k3, "%d")
    DEBUGDUMP(k4, "%d")
    DEBUGDUMP(k5, "%d")
    DEBUGMSG("=======")
    if (k1 < 0 || k2 < 0 || k3 < 0 || k4 < 0 || k5 < 0)
        return;

    //shell handling
    if (shellCounter == 0) {
        if ((k1 == k4 && k2 + k3 + k5 == 0) ||
                (k2 == k5 && k1 + k3 + k4 == 0) ||
                (k3 == k1 && k2 + k4 + k5 == 0) ||
                (k4 == k2 && k1 + k3 + k5 == 0) ||
                (k5 == k3 && k1 + k2 + k4 == 0)) {
            //in this case the shell is no longer cyclic
            currentShell = addNewShell(currentShell, shellCounter = (k1 + k2 + k3 + k4 + k5) / 2 + 1, current);
            currentShell->nonCyclicShell = 1;
        } else if (k1 + k2 + k3 + k4 + k5 == 0) {
            currentShell = addNewShell(currentShell, shellCounter = 1, current);
        } else {
            currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + k3 + k4 + k5, current);
        }
        int sides[5];
        sides[0] = k1;
        sides[1] = k2;
        sides[2] = k3;
        sides[3] = k4;
        sides[4] = k5;
        if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 5, sides, shellStart))
            return;
        shellStart=0; //the new shell starts at the current position
    }

    INNERSPIRAL *is = patch->innerspiral;

    if (k1 == 0 && k2 == 0 && k3 == 0 && k4 == 0 && k5 == 0) {
        if(!iprMode || (p1 && p2 && p3 && p4 && p5)){
            PENTFRAG(current, 1, currentShell)
            if (currentShell->size == 0) currentShell->size = 1;
            fillPatch_0PentagonsLeft(0, 0, 0, 0, 0, 0, patch,
                    addNewFragment(current), shellCounter - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
        }
    } else if (k1 == 0 && k2 == 0) {
        //only one possible filling in case the following is true
        if (k3 == k5 && k4 == 0) {
            is->code[is->position] += 1;
            HEXFRAG(current, 1)
            fillPatch_1PentagonLeft(0, k3 - 1, 0, k5 - 1, 0, patch, 
                    addNewFragment(current), shellCounter - 1, currentShell,
                    (shellStart + 4)%5, 1, 1, p4, p5, !lastWasPi, FALSE);
            is->code[is->position] -= 1;
        }
    } else if (k1 == 0 && k5 == 0) {
        //only one possible filling in case the following is true
        if (k2 == k4 && k3 == 0) {
            is->code[is->position] += 1;
            HEXFRAG(current, 1)
            fillPatch_1PentagonLeft(k2 - 1, 0, k4 - 1, 0, 0, patch, 
                    addNewFragment(current), shellCounter - 1, currentShell,
                    (shellStart + 4)%5, 1, p3, p4, 1, !lastWasPi, FALSE);
            is->code[is->position] -= 1;
        }
    } else if (k4 == 0 && k5 == 0) {
        //only one possible filling in case the following is true
        if (k1 == k3 && k2 == 0) {
            if (k1 > 1 && shellCounter == 1) {
                is->code[is->position] += 1;
                HEXFRAG(current, 1)
                fillPatch_1PentagonLeft(k1 - 1, 0, k1 - 1, 0, 0, patch,
                        addNewFragment(current), 0, currentShell, (shellStart + 4)%5,
                        p2, p3, 1, 1, !lastWasPi, FALSE);
                is->code[is->position] -= 1;
            } else if(!iprMode || (p2 && p3)){
                //in case of IPR-mode a pentagon at the second and third break-edge must be allowed
                is->code[is->position] += k1;
                PENTFRAG(current, k1 + 1, currentShell)
                fillPatch_0PentagonsLeft(0, 0, 0, 0, 0, 0, patch, 
                        addNewFragment(current), shellCounter - k1 - 1, currentShell,
                        shellStart==0 ? 5 : shellStart);
                currentShell->nrOfPentagons--;
                is->code[is->position] -= k1;
            }
        }
    } else if (k3 == 0 && k4 == 0) {
        //only one possible filling in case the following is true
        if (k1 == 0 && k2 == k5) {
            if(!iprMode || (p1 && p2)){
                //in case of IPR-mode a pentagon at the first and second break-edge must be allowed
                //is->code[is->position]+=0;
                PENTFRAG(current, 1, currentShell)
                fillPatch_0PentagonsLeft(k2 - 1, 0, 0, k2 - 1, 0, 0, patch, 
                        addNewFragment(current), shellCounter - 1, currentShell,
                        shellStart==0 ? 5 : shellStart);
                //is->code[is->position]-=0;
                currentShell->nrOfPentagons--;
            }
        }
    } else if (k2 == 0 && k3 == 0) {
        //only one possible filling in case the following is true
        if (k5 == 0 && k1 == k4) {
            if(!iprMode || (p1 && p5)){
                //in case of IPR-mode a pentagon at the first and last break-edge must be allowed
                //is->code[is->position]+=0;
                PENTFRAG(current, 1, currentShell)
                fillPatch_0PentagonsLeft(k1 - 1, 0, 0, k1 - 1, 0, 0, patch,
                        addNewFragment(current), shellCounter - 1, currentShell,
                        shellStart==0 ? 5 : shellStart);
                //is->code[is->position]-=0;
                currentShell->nrOfPentagons--;
            }
        }
    } else if (k2 == 0 && k5 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_1PentagonLeft(0, k3 - 1, k4 - 1, 0, k1 - 1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 4)%5, 1, 1, p4, 1, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !(p1 && p5)){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            //because k5 is 0 we also need to check whether the last break-edge may lie at a pentagon
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_0PentagonsLeft(k1 - 1 - i, k2, k3, k4 - 1, 0, i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }
    } else if (k2 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)


        fillPatch_1PentagonLeft(0, k3 - 1, k4, k5 - 1, k1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 4)%5, 1, 1, p4, p5, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_0PentagonsLeft(k1 - 1 - i, k2, k3, k4, k5 - 1, 1 + i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }
    } else if (k5 == 0) {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_1PentagonLeft(k2 - 1, k3, k4 - 1, k5, k1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 4)%5, 1, p3, p4, 1, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !(p1 && p5)){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            //because k5 is 0 we also need to check whether the last break-edge may lie at a pentagon
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_0PentagonsLeft(k1 - 1 - i, k2, k3, k4 - 1, k5, i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;
            PENTFRAG(current, k1 + 1, currentShell)
            fillPatch_0PentagonsLeft(0, k2 - 1, k3, k4 - 1, k5, k1 - 1, patch,
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    } else {
        //add a side of hexagons
        is->code[is->position] += k1 + 1;

        HEXFRAG(current, k1 + 1)

        fillPatch_1PentagonLeft(k2 - 1, k3, k4, k5 - 1, k1 + 1, patch, 
                addNewFragment(current), shellCounter - k1 - 1, currentShell,
                (shellStart + 4)%5, 1, p3, p4, p5, !lastWasPi, FALSE);
        is->code[is->position] -= k1 + 1;

        //pentagon after i hexagons
        int i;
        int iStart = 0;
        if(iprMode && !p1){
            //in case of ipr we check whether a pentagon may be placed at the break-edge
            iStart = 1;
        }
        for (i = iStart; i < k1; i++) {
            is->code[is->position] += i;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, i + 1, currentShell)

            fillPatch_0PentagonsLeft(k1 - 1 - i, k2, k3, k4, k5 - 1, 1 + i, patch, 
                    addNewFragment(current), shellCounter - i - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= i;
        }

        //pentagon after k1 hexagons
        if(!iprMode || p2){
            //only placed when we are not in IPR-mode or a pentagon is allowed at the second break-edge
            if(iprMode && (k1==0 && !p1))
                return;
            //if k1 is equal to 0 then p1 also plays a role here
            is->code[is->position] += k1;
            is->position++;
            is->code[is->position] = 0;

            PENTFRAG(current, k1 + 1, currentShell)

            fillPatch_0PentagonsLeft(0, k2 - 1, k3, k4, k5 - 1, k1, patch, 
                    addNewFragment(current), shellCounter - k1 - 1, currentShell,
                    shellStart==0 ? 5 : shellStart);
            currentShell->nrOfPentagons--;
            is->position--;
            is->code[is->position] -= k1;
        }
    }
}

void fillPatch_0PentagonsLeft(int k1, int k2, int k3, int k4, int k5, int k6,
        PATCH *patch, FRAGMENT *current, int shellCounter, SHELL *currentShell, int shellStart) {
    DEBUGDUMP(k1, "%d")
    DEBUGDUMP(k2, "%d")
    DEBUGDUMP(k3, "%d")
    DEBUGDUMP(k4, "%d")
    DEBUGDUMP(k5, "%d")
    DEBUGDUMP(k6, "%d")
    DEBUGMSG("=======")
            //check to see if the boundary is closed in the hexagonal lattice
            int x = 2 * k1 + k2 - k3 - 2 * k4 - k5 + k6;
    int y = k1 + 2 * k2 + k3 - k4 - 2 * k5 - k6;

    if (x == 0 && y == 0) {
        //shell handling
        if (shellCounter == 0) {
            if (!(k1 + k2 + k3 + k4 + k5 + k6 == 0) &&
                    ((k1 == k4 && k2 + k3 + k5 + k6 == 0) ||
                    (k2 == k5 && k1 + k3 + k4 + k6 == 0) ||
                    (k3 == k6 && k1 + k2 + k4 + k5 == 0) ||
                    (k4 == k1 && k2 + k3 + k5 + k6 == 0) ||
                    (k5 == k2 && k1 + k3 + k4 + k6 == 0) ||
                    (k6 == k3 && k1 + k2 + k4 + k5 == 0))) {
                //in this case the shell is no longer cyclic
                currentShell = addNewShell(currentShell, shellCounter = (k1 + k2 + k3 + k4 + k5 + k6) / 2 + 1, current);
                currentShell->nonCyclicShell = 1;
            } else {
                currentShell = addNewShell(currentShell, shellCounter = k1 + k2 + k3 + k4 + k5 + k6, current);
            }
            int sides[6];
            sides[0] = k1;
            sides[1] = k2;
            sides[2] = k3;
            sides[3] = k4;
            sides[4] = k5;
            sides[5] = k6;
            if (!checkShellCanonicity(patch, currentShell->prev, currentShell, 6, sides, shellStart))
                return;
            if (validateStructure(patch)) {
                current->prev->isEnd = 1;
                processStructure(patch, currentShell);
                current->prev->isEnd = 0;
            }
        } else {
            //complete shell
            HEXFRAG(current, k1 + 1)
                    //no need to add something to the code: where already past the last pentagon
            if (k1 == k4 && k2 + k3 + k5 + k6 == 0) {
                fillPatch_0PentagonsLeft(0, 0, 0, 0, 0, 0, patch,
                        addNewFragment(current), shellCounter - k1 - 1, currentShell,
                        (shellStart + 5)%6);
            } else {
                fillPatch_0PentagonsLeft(k2 - 1, k3, k4, k5, k6 - 1, k1 + 1, patch,
                        addNewFragment(current), shellCounter - k1 - 1, currentShell,
                        (shellStart + 5)%6);
            }
        }

    }

}

