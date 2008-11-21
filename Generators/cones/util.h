/*
 *  util.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/09/08.
 *
 */

#ifndef _UTIL_H //if not defined
#define _UTIL_H

#include <stdio.h>

int parseNumber(char *argv[]);
int halfCeil(int x);
int halfFloor(int x);
void printArray(int *array, int length);

#endif // end if not defined, and end the header file
