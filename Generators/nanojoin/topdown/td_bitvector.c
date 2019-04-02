#include <stdlib.h>
#include <stdio.h>
#include "td_bitvector.h"

void print_vector(struct bitvector *vector);

void bit_add(struct bitvector *vector, unsigned char bit) {
	unsigned char rest1, rest2;
	int i;
	if (vector->maxbit == lastbit) {
		vector->maxbit = 1;
		vector->lastused++;
		vector->bits[vector->lastused] = 0;
	} else {
		vector->maxbit = vector->maxbit << 1;
	}
	rest1 = bit;
	rest2 = 0;
	for(i = 0; i < vector->lastused; i++) {
		if (vector->bits[i] & lastbit) {
			rest2 = 1;
		} else {
			rest2 = 0;
		}
		vector->bits[i] = vector->bits[i] << 1 | rest1;
		rest1 = rest2;
	}
	vector->bits[vector->lastused] = vector->bits[vector->lastused] << 1 | rest1;
}

void bit_rotate(struct bitvector *vector) {
	unsigned char rest1, rest2;
	int i;
	rest1 = 0;
	for (i=vector->lastused;  i >= 0; i--) {
		rest2 = vector->bits[i] % 2;
		vector->bits[i] = (vector->bits[i] >> 1);
		if (rest1) {
			vector->bits[i] = vector->bits[i] | lastbit;
		}
		rest1 = rest2;
	}
	if(rest1) {
		vector->bits[vector->lastused] = vector->bits[vector->lastused] | vector->maxbit;
	}
}

/*
Bitvectors should have same length
*/
char bit_compare(struct bitvector *vector1, struct bitvector *vector2) {
	int i;
	/*
	COMMENTED OUT FOR SPEEDUP
	if (vector1->lastused != vector2->lastused || vector1->lastindex != vector2->lastindex) {
		fprintf(stderr, "Bitvectors should have same length for comparison");
		exit(EXIT_FAILURE);
	}*/
	for(i = vector1->lastused; i >= 0; i--) {
		if (vector1->bits[i] > vector2->bits[i]) {
			return 1;
		} else if (vector1->bits[i] < vector2->bits[i]) {
			return -1;
		}
	}
	return 0;
}

void print_vector(struct bitvector *vector) {
	int i;
	unsigned long long currentmask;
	currentmask = vector->maxbit;
	for (i = vector->lastused; i >= 0; i--) {
		while (currentmask != 0) {
			if (currentmask & vector->bits[i]) {
				printf("1");
			} else {
				printf("0");
			}
			currentmask = currentmask >> 1;
		}
		currentmask = lastbit;
	}
	printf("\n");
}