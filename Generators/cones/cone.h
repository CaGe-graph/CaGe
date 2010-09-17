/*
 *  cone.h
 *  
 *
 *  Created by Nico Van Cleemput on 19/05/09.
 *
 */

#ifndef _CONE_H //if not defined
#define _CONE_H

#include "pseudoconvex.h"
#include "util.h"

void start5PentagonsCone(PATCH *patch, int sside, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell);
void start4PentagonsCone(PATCH *patch, int sside, int symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell);
void start3PentagonsCone(PATCH *patch, int sside, int symmetric, boolean mirror, FRAGMENT *currentFragment, SHELL *currentShell);

#endif // end if not defined, and end the header file
