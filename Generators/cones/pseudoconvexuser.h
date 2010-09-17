/*
 *  pseudoconvexuser.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _PSEUDOCONVEXUSER_H //if not defined
#define _PSEUDOCONVEXUSER_H

#include "util.h"

/* This method gets called each time a completed structure is found and right before processStructure is called.
 */
boolean validateStructure(PATCH *patch);

/* This method gets called each time a valid structure is found.
 */
void processStructure(PATCH *patch, SHELL *shell);

#endif // end if not defined, and end the header file
