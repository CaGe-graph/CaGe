/*
 *  twopentagons.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#include "twopentagons.h"
#include "cone.h"
#include "util.h"
#include "pseudoconvex.h"
#include "pseudoconvexuser.h"

#include <stdlib.h>

void getTwoPentagonsConesShortestSide1SymmetricPatch(PATCH *patch, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *frag;

    frag = addNewFragment(currentFragment);
    frag->endsWithPentagon=1;
    frag->faces=1;
    currentShell = addNewShell(currentShell, 4, frag);
    frag = addNewFragment(frag);
    frag->endsWithPentagon=0;
    frag->faces=1;
    frag = addNewFragment(frag);
    frag->endsWithPentagon=1;
    frag->faces=1;
    frag = addNewFragment(frag);
    frag->endsWithPentagon=0;
    frag->faces=1;

    is->code[0]+=0;
    is->code[1]=1;

    processStructure(patch, currentShell);
}

void getTwoPentagonsConesShortestSide0NearSymmetricPatch(PATCH *patch, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *frag;

    frag = addNewFragment(currentFragment);
    frag->endsWithPentagon=0;
    frag->faces=1;
    currentShell = addNewShell(currentShell, 3, frag);
    frag = addNewFragment(frag);
    frag->endsWithPentagon=1;
    frag->faces=1;
    frag = addNewFragment(frag);
    frag->endsWithPentagon=1;
    frag->faces=1;

    is->code[0]+=1;
    is->code[1]=0;

    processStructure(patch, currentShell);
}

void getTwoPentagonsConesNoMirrorSymmetricPatches(PATCH *patch, int side, FRAGMENT *currentFragment, SHELL *currentShell) {
    int i;
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *fragmentWithFirstPentagon = addNewFragment(currentFragment);
    fragmentWithFirstPentagon->endsWithPentagon = 1;
    FRAGMENT *fragmentAfterFirstPentagon = addNewFragment(fragmentWithFirstPentagon);
    fragmentAfterFirstPentagon->endsWithPentagon = 0;
    FRAGMENT *secondSide = addNewFragment(fragmentAfterFirstPentagon);
    secondSide->faces = side;
    FRAGMENT *fragment4 = addNewFragment(secondSide);
    FRAGMENT *fragment5 = addNewFragment(fragment4);
    fragment5->endsWithPentagon = 0;
    FRAGMENT *fragment6 = addNewFragment(fragment5);
    currentShell = addNewShell(currentShell, 4 * side, fragmentWithFirstPentagon);

    int upperbound = HALFFLOOR(side) + 1;

    //first we handle the case with spiral code (0, 2*side)
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    is->code[0] += 0;
    is->code[1] = 2 * side - 1;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    fragmentWithFirstPentagon->faces = 1;
    fragmentAfterFirstPentagon->faces = side;
    secondSide->endsWithPentagon = 1;
    fragment4->endsWithPentagon = 0;
    fragment4->faces = side;
    fragment5->faces = side - 1;
    fragment5->isEnd = 1;
    processStructure(patch, currentShell);
    fragment5->isEnd = 0;
    fragment6->isEnd = 1;

    is->code[0]++;

    //sside patches with spiral code i, 2*sside (i=0,...,sside-1)
    secondSide->endsWithPentagon = 0;
    fragment4->endsWithPentagon = 1;
    fragment6 = addNewFragment(fragment5);
    fragment6->faces = side - 1;
    fragment6->endsWithPentagon = 0;

    for (i = 1; i < upperbound; i++) {
        fragmentWithFirstPentagon->faces = i + 1;
        fragmentAfterFirstPentagon->faces = side - i;
        fragment4->faces = i;
        fragment5->faces = side - i;
        processStructure(patch, currentShell);
        is->code[0]++;
    }
}

void getTwoPentagonsConesMirrorsAllowedSymmetricPatches(PATCH *patch, int side, FRAGMENT *currentFragment, SHELL *currentShell) {
    int i;
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *fragmentWithFirstPentagon = addNewFragment(currentFragment);
    fragmentWithFirstPentagon->endsWithPentagon = 1;
    FRAGMENT *fragmentAfterFirstPentagon = addNewFragment(fragmentWithFirstPentagon);
    fragmentAfterFirstPentagon->endsWithPentagon = 0;
    FRAGMENT *secondSide = addNewFragment(fragmentAfterFirstPentagon);
    secondSide->faces = side;
    FRAGMENT *fragment4 = addNewFragment(secondSide);
    FRAGMENT *fragment5 = addNewFragment(fragment4);
    fragment5->endsWithPentagon = 0;
    FRAGMENT *fragment6 = addNewFragment(fragment5);
    currentShell = addNewShell(currentShell, 4 * side, fragmentWithFirstPentagon);

    //first we handle the case with spiral code (0, 2*side)
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    is->code[0] += 0;
    is->code[1] = 2 * side - 1;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    fragmentWithFirstPentagon->faces = 1;
    fragmentAfterFirstPentagon->faces = side;
    secondSide->endsWithPentagon = 1;
    fragment4->endsWithPentagon = 0;
    fragment4->faces = side;
    fragment5->faces = side - 1;
    fragment5->isEnd = 1;
    processStructure(patch, currentShell);
    fragment5->isEnd = 0;
    fragment6->isEnd = 1;

    is->code[0]++;

    //sside patches with spiral code i, 2*sside (i=0,...,sside-1)
    secondSide->endsWithPentagon = 0;
    fragment4->endsWithPentagon = 1;
    fragment6 = addNewFragment(fragment5);
    fragment6->faces = side - 1;
    fragment6->endsWithPentagon = 0;
    
    for (i = 1; i < side; i++) {
        fragmentWithFirstPentagon->faces = i + 1;
        fragmentAfterFirstPentagon->faces = side - i;
        fragment4->faces = i;
        fragment5->faces = side - i;
        processStructure(patch, currentShell);
        is->code[0]++;
    }
}

void getTwoPentagonsConesNoMirrorNearSymmetricPatches(PATCH *patch, int shortestSide, FRAGMENT *currentFragment, SHELL *currentShell) {
    //int longestSide = shortestSide + 1;
    int i;
    INNERSPIRAL *is = patch->innerspiral;
    int startSpiral = is->code[0];

    FRAGMENT *fragment1 = addNewFragment(currentFragment);
    fragment1->faces = shortestSide + 1;
    fragment1->endsWithPentagon = 0;
    currentShell = addNewShell(currentShell, 4 * shortestSide + 3, fragment1);
    FRAGMENT *fragment2 = addNewFragment(fragment1);
    FRAGMENT *fragment3 = addNewFragment(fragment2);
    FRAGMENT *fragment4 = addNewFragment(fragment3);
    FRAGMENT *fragment5 = addNewFragment(fragment4);
    FRAGMENT *fragment6 = addNewFragment(fragment5);

    //PART 1: pentagon on second side

    //first we handle the case with spiral code (shortestSide + 1, 3*shortestSide + 2)
    fragment2->endsWithPentagon=1;
    fragment2->faces=1;
    fragment3->endsWithPentagon=0;
    fragment3->faces=shortestSide;
    fragment4->endsWithPentagon=1;
    fragment4->faces=shortestSide+1;
    fragment5->endsWithPentagon=0;
    fragment5->faces=shortestSide;
    fragment5->isEnd=1;
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    is->code[0] += shortestSide+1;
    is->code[1] = 2*shortestSide;
    processStructure(patch, currentShell);
    fragment5->isEnd=0;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    //next we handle the cases with spiral code (shortestSide + 1 + i, 3*shortestSide + 2 + i)
    //with 0 < i < shortestSide
    fragment6->isEnd=1;
    fragment4->endsWithPentagon=0;
    fragment5->endsWithPentagon=1;
    int upperbound = HALFFLOOR(shortestSide);
    for(i=1;i<=upperbound;i++){
        fragment2->faces=i+1;
        fragment3->faces=shortestSide-i;
        fragment5->faces=i;
        fragment6->faces=shortestSide-i;
        is->code[0]++;
        processStructure(patch, currentShell);
    }

    //PART 2: pentagon on third side
    //outer shell
    fragment2->endsWithPentagon=0;
    fragment2->faces=shortestSide+1;
    fragment3->endsWithPentagon=1;
    fragment5->endsWithPentagon=0;
    fragment5->faces=shortestSide;

    //next shell
    fragment6->endsWithPentagon=1;
    currentShell = addNewShell(currentShell, 4 * shortestSide - 2, fragment6);
    //TODO: this shell is not complete. Is it necessary to fill it? Use fillPatch_0Pentagons...?
    //if we include the other pieces of this shell, the last case of this for-loop
    //will need to be handled separately

    is->code[0] = startSpiral;
    is->code[0] += 2*shortestSide + 1;

    upperbound = shortestSide - HALFFLOOR(shortestSide);
    for(i=0; i<upperbound; i++){
        fragment3->faces=i+1;
        fragment4->faces=shortestSide-i;
        fragment6->faces=i+1;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
}

void getTwoPentagonsConesNoMirrorMoreSymmetricPatches(PATCH *patch, int shortestSide, FRAGMENT *currentFragment, SHELL *currentShell) {
    //int longestSide = shortestSide + 1;
    int i;
    INNERSPIRAL *is = patch->innerspiral;
    int startSpiral = is->code[0];

    FRAGMENT *fragment1 = addNewFragment(currentFragment);
    currentShell = addNewShell(currentShell, 4 * shortestSide + 2, fragment1);
    FRAGMENT *fragment2 = addNewFragment(fragment1);
    FRAGMENT *fragment3 = addNewFragment(fragment2);
    FRAGMENT *fragment4 = addNewFragment(fragment3);
    FRAGMENT *fragment5 = addNewFragment(fragment4);

    //PART 1: pentagon on second side (now first side)

    //first we handle the case with spiral code (shortestSide + 1, 3*shortestSide + 2)
    fragment1->endsWithPentagon=1;
    fragment1->faces=1;
    fragment2->endsWithPentagon=0;
    fragment2->faces=shortestSide;
    fragment3->endsWithPentagon=1;
    fragment3->faces=shortestSide+1;
    fragment4->endsWithPentagon=0;
    fragment4->faces=shortestSide;
    fragment4->isEnd=1;
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    //is->code[0] += 0;
    is->code[1] = 2*shortestSide;
    processStructure(patch, currentShell);
    fragment4->isEnd=0;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    //next we handle the cases with spiral code (shortestSide + 1 + i, 3*shortestSide + 2 + i)
    //with 0 < i < shortestSide
    fragment5->isEnd=1;
    fragment3->endsWithPentagon=0;
    fragment4->endsWithPentagon=1;
    int upperbound = HALFFLOOR(shortestSide);
    for(i=1;i<=upperbound;i++){
        fragment1->faces=i+1;
        fragment2->faces=shortestSide-i;
        fragment4->faces=i;
        fragment5->faces=shortestSide-i;
        is->code[0]++;
        processStructure(patch, currentShell);
    }

    //PART 2: pentagon on third side (now second side)
    //outer shell
    fragment1->endsWithPentagon=0;
    fragment1->faces=shortestSide+1;
    fragment2->endsWithPentagon=1;
    fragment4->endsWithPentagon=0;
    fragment4->faces=shortestSide;

    //next shell
    fragment5->endsWithPentagon=1;

    is->code[0] = startSpiral;
    is->code[0] += shortestSide;

    upperbound = shortestSide - HALFFLOOR(shortestSide);
    for(i=0; i<upperbound; i++){
        fragment2->faces=i+1;
        fragment3->faces=shortestSide-i;
        fragment5->faces=i+1;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
}

void getTwoPentagonsConesMirrorsAllowedNearSymmetricPatches(PATCH *patch, int shortestSide, FRAGMENT *currentFragment, SHELL *currentShell) {
    //int longestSide = shortestSide + 1;
    int i;
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *fragment1 = addNewFragment(currentFragment);
    fragment1->faces = shortestSide + 1;
    fragment1->endsWithPentagon = 0;
    currentShell = addNewShell(currentShell, 4 * shortestSide + 3, fragment1);
    FRAGMENT *fragment2 = addNewFragment(fragment1);
    FRAGMENT *fragment3 = addNewFragment(fragment2);
    FRAGMENT *fragment4 = addNewFragment(fragment3);
    FRAGMENT *fragment5 = addNewFragment(fragment4);
    FRAGMENT *fragment6 = addNewFragment(fragment5);

    //PART 1: pentagon on second side

    //first we handle the case with spiral code (shortestSide + 1, 3*shortestSide + 2)
    fragment2->endsWithPentagon=1;
    fragment2->faces=1;
    fragment3->endsWithPentagon=0;
    fragment3->faces=shortestSide;
    fragment4->endsWithPentagon=1;
    fragment4->faces=shortestSide+1;
    fragment5->endsWithPentagon=0;
    fragment5->faces=shortestSide;
    fragment5->isEnd=1;
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    is->code[0] += shortestSide+1;
    is->code[1] = 2*shortestSide;
    processStructure(patch, currentShell);
    fragment5->isEnd=0;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    //next we handle the cases with spiral code (shortestSide + 1 + i, 3*shortestSide + 2 + i)
    //with 0 < i < shortestSide
    fragment6->isEnd=1;
    fragment4->endsWithPentagon=0;
    fragment5->endsWithPentagon=1;
    for(i=1;i<shortestSide;i++){
        fragment2->faces=i+1;
        fragment3->faces=shortestSide-i;
        fragment5->faces=i;
        fragment6->faces=shortestSide-i;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
    fragment6->isEnd=0;

    //then we handle the case with spiral code (2*shortestSide + 1, 4*shortestSide + 2)
    fragment5->isEnd=1;
    fragment2->faces=shortestSide+1;
    fragment3->faces=1;
    fragment4->faces=shortestSide;
    fragment5->faces=shortestSide;
    is->code[0]++;
    processStructure(patch, currentShell);
    fragment5->isEnd=0;

    //PART 2: pentagon on third side
    //outer shell
    fragment2->endsWithPentagon=0;
    fragment2->faces=shortestSide+1;
    fragment3->endsWithPentagon=1;
    fragment5->endsWithPentagon=0;
    fragment5->faces=shortestSide;

    //next shell
    fragment6->endsWithPentagon=1;
    fragment6->isEnd=1;
    currentShell = addNewShell(currentShell, 4 * shortestSide - 2, fragment6);
    //TODO: this shell is not complete. Is it necessary to fill it? Use fillPatch_0Pentagons...?
    //if we include the other pieces of this shell, the last case of this for-loop
    //will need to be handled separately
    for(i=0; i<shortestSide; i++){
        fragment3->faces=i+1;
        fragment4->faces=shortestSide-i;
        fragment6->faces=i+1;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
}

void getTwoPentagonsConesMirrorsAllowedMoreSymmetricPatches(PATCH *patch, int shortestSide, FRAGMENT *currentFragment, SHELL *currentShell) {
    //int longestSide = shortestSide + 1;
    int i;
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *fragment1 = addNewFragment(currentFragment);
    currentShell = addNewShell(currentShell, 4 * shortestSide + 2, fragment1);
    FRAGMENT *fragment2 = addNewFragment(fragment1);
    FRAGMENT *fragment3 = addNewFragment(fragment2);
    FRAGMENT *fragment4 = addNewFragment(fragment3);
    FRAGMENT *fragment5 = addNewFragment(fragment4);

    //PART 1: pentagon on second side (now first side)

    //first we handle the case with spiral code (shortestSide + 1, 3*shortestSide + 2)
    fragment1->endsWithPentagon=1;
    fragment1->faces=1;
    fragment2->endsWithPentagon=0;
    fragment2->faces=shortestSide;
    fragment3->endsWithPentagon=1;
    fragment3->faces=shortestSide+1;
    fragment4->endsWithPentagon=0;
    fragment4->faces=shortestSide;
    fragment4->isEnd=1;
    //the innerspiral stores the number of hexagons between the pentagons
    //thus this gives:
    //is->code[0] += 0;
    is->code[1] = 2*shortestSide;
    processStructure(patch, currentShell);
    fragment4->isEnd=0;

    //in the rest of this code the distance between the two pentagons doesn't change
    //so we only need to adjust is->code[0]

    //next we handle the cases with spiral code (shortestSide + 1 + i, 3*shortestSide + 2 + i)
    //with 0 < i < shortestSide
    fragment5->isEnd=1;
    fragment3->endsWithPentagon=0;
    fragment4->endsWithPentagon=1;
    for(i=1;i<shortestSide;i++){
        fragment1->faces=i+1;
        fragment2->faces=shortestSide-i;
        fragment4->faces=i;
        fragment5->faces=shortestSide-i;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
    fragment5->isEnd=0;

    //then we handle the case with spiral code (2*shortestSide + 1, 4*shortestSide + 2)
    fragment4->isEnd=1;
    fragment1->faces=shortestSide+1;
    fragment2->faces=1;
    fragment3->faces=shortestSide;
    fragment4->faces=shortestSide;
    is->code[0]++;
    processStructure(patch, currentShell);
    fragment4->isEnd=0;

    //PART 2: pentagon on third side (now second side)
    //outer shell
    fragment1->endsWithPentagon=0;
    fragment1->faces=shortestSide+1;
    fragment2->endsWithPentagon=1;
    fragment4->endsWithPentagon=0;
    fragment4->faces=shortestSide;

    //next shell
    fragment5->endsWithPentagon=1;
    fragment5->isEnd=1;

    for(i=0; i<shortestSide; i++){
        fragment2->faces=i+1;
        fragment3->faces=shortestSide-i;
        fragment5->faces=i+1;
        is->code[0]++;
        processStructure(patch, currentShell);
    }
}

/* void getTwoPentagonsPatch(int sside, int symmetric, int mirror) */
/*
        generates the canonical cone patches with two pentagons and the given boundary
 */
void getTwoPentagonsCones(PATCH *patch, int sside, boolean symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;
    if (is->length != 2) exit(1);

    if(symmetric) {
        if(sside==1){
            getTwoPentagonsConesShortestSide1SymmetricPatch(patch, currentFragment, currentShell);
        } else if(mirror){
            getTwoPentagonsConesMirrorsAllowedSymmetricPatches(patch, sside, currentFragment, currentShell);
        } else {
            getTwoPentagonsConesNoMirrorSymmetricPatches(patch, sside, currentFragment, currentShell);
        }
    } else {
        if(sside==0){
            getTwoPentagonsConesShortestSide0NearSymmetricPatch(patch, currentFragment, currentShell);
        } else if(mirror){
            getTwoPentagonsConesMirrorsAllowedNearSymmetricPatches(patch, sside, currentFragment, currentShell);
        } else {
            getTwoPentagonsConesNoMirrorNearSymmetricPatches(patch, sside, currentFragment, currentShell);
        }
    }
}

void getTwoPentagonsConesMoreSymmetricSide0(PATCH *patch, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;

    FRAGMENT *frag;

    frag = addNewFragment(currentFragment);
    frag->endsWithPentagon=1;
    frag->faces=1;
    currentShell = addNewShell(currentShell, 2, frag);
    frag = addNewFragment(frag);
    frag->endsWithPentagon=1;
    frag->faces=1;

    //is->code[0]+=0;
    is->code[1]=0;

    processStructure(patch, currentShell);
}

void getTwoPentagonsConesMoreSymmetric(PATCH *patch, int sside, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell) {
    INNERSPIRAL *is = patch->innerspiral;
    if (is->length != 2) exit(1);
    
    if(sside==0){
        getTwoPentagonsConesMoreSymmetricSide0(patch, currentFragment, currentShell);
    } else if(mirror){
        getTwoPentagonsConesMirrorsAllowedMoreSymmetricPatches(patch, sside, currentFragment, currentShell);
    } else {
        getTwoPentagonsConesNoMirrorMoreSymmetricPatches(patch, sside, currentFragment, currentShell);
    }
}

int getTwoPentagonsConesCount(int sside, boolean symmetric, boolean mirror) {
    if (symmetric) {
        if (mirror) {
            return sside;
        } else {
            return HALFFLOOR(sside) + 1;
        }
    } else {
        if(mirror) {
            return 2*sside +1;
        } else {
            return sside + 1;
        }
    }
}
