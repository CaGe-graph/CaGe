/*
 *  util.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/09/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "util.h"

/* int parseNumber(char *argv[]) */
/*
	parses a number in a string. Stops at the first space. returns 0 in case of an error.
    A - at the beginning is allowed.
*/
int parseNumber(char *argv[]){
	int c, stop=0, number = 0, sign = 1;
    while(!stop && (c = *(argv[0])++)){
		switch (c) {
			case '0':
				number = number*10;
				break;
			case '1':
				number = number*10 + 1;
				break;
			case '2':
				number = number*10 + 2;
				break;
			case '3':
				number = number*10 + 3;
				break;
			case '4':
				number = number*10 + 4;
				break;
			case '5':
				number = number*10 + 5;
				break;
			case '6':
				number = number*10 + 6;
				break;
			case '7':
				number = number*10 + 7;
				break;
			case '8':
				number = number*10 + 8;
				break;
			case '9':
				number = number*10 + 9;
				break;
			case ' ':
				stop=1;
				break;
			case '-':
				if(!number){
					sign = -1;
				} else {
					stop = 1;
					number = 0;
				}
				break;
			default:
				fprintf(stderr, "Error while parsing number: %c\n", c);
				stop = 1;
				number = 0;
				break;
		}
	}
	return number*sign;
}

int halfCeil(int x){
	if(x % 2 == 0)
		return x/2;
	else
		return (x-1)/2 + 1;
}

int halfFloor(int x){
	if(x % 2 == 0)
		return x/2;
	else
		return (x-1)/2;
}

void printArray(int *array, int length){
	int i;
	for(i=0;i<length;i++)
		fprintf(stderr, "%d ", array[i]);
	fprintf(stderr, "\n");
}
