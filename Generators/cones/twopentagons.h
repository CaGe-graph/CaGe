/*
 *  twopentagons.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _TWOPENTAGONS_H //if not defined
#define _TWOPENTAGONS_H

#include "util.h"
#include "pseudoconvex.h"

void getTwoPentagonsCones(PATCH *patch, int sside, boolean symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell);
void getTwoPentagonsConesMoreSymmetric(PATCH *patch, int sside, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell);
int getTwoPentagonsConesCount(int sside, boolean symmetric, boolean mirror);

#endif // end if not defined, and end the header file
