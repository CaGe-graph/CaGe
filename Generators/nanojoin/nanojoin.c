#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "topdown/td_nanojoin.h"
#include "bottomup/bu_nanojoin.h"
#include "tree/tree.h"
#include "layer.h"

void swap(int newl, int newm, int** outsideparameters, int** insideparameters, int i) {
	(*insideparameters)[i] = (*outsideparameters)[0];
	(*insideparameters)[i+1] = (*outsideparameters)[1]; 
	(*outsideparameters)[0] = newl;
	(*outsideparameters)[1] = newm;
}

unsigned char bigger(int firstl, int firstm, int secondl, int secondm) {
    unsigned char cond1, cond2, cond3;
	int firstsum, secondsum, value;
	firstsum = firstl + firstm;
	secondsum = secondl + secondm;
	
	cond1 = secondsum > firstsum ? 2: secondsum == firstsum;
	cond2 = secondl > firstl ? 2 : secondl == firstl;
	cond3 = secondm > firstm ? 2: secondm == firstm;

	value = 9*cond1 + 3*cond2 + cond3;

	return value >= 14;
}

void run_algorithm(unsigned char pent, unsigned char hex, unsigned char hept, unsigned char nrofnanocap, int** nanocapparameters, int rings, unsigned char exactfaces) {

	int i, internalvertices, l, m;


	initialize_tree();

	if (pent <= 3) {
		spent = pent;
	} else {
		spent = 3;
	}
	shex = hex ;
	shept = hept;

	/* Initiating global variables for topdown */
	internalvertices = (5*pent+6*hex+7*hept)/3;
	nrofnanocaps = nrofnanocap;
	outsideparameters = malloc(2*sizeof(int));
	insideparameters = malloc((1+(nrofnanocap-1)*2)*sizeof(int));

	outsideparameters[0] = (*nanocapparameters)[0];
	outsideparameters[1] = (*nanocapparameters)[1];
	internalvertices -= outsideparameters[0] + outsideparameters[1];
	insideparameters[0] = nrofnanocap-1;

	/* Choosing right outer face (non symmetrical - longer - l parameter - m paramter)*/
	for (i=2; i < 2*nrofnanocap; i+=2) {
		l = (*nanocapparameters)[i];
		m = (*nanocapparameters)[i+1];

		if (l == 0) {
			l = m;
			m = 0;
		}

		if (outsideparameters[1] == 0 && m != 0) {
			swap(l, m, &outsideparameters, &insideparameters, i-1);
		} else if (bigger(outsideparameters[0], outsideparameters[1], l, m)) {
			swap(l, m, &outsideparameters, &insideparameters, i-1);
		} else {
			insideparameters[i-1] = (*nanocapparameters)[i];
			insideparameters[i] = (*nanocapparameters)[i+1]; 
		}
		internalvertices -= (*nanocapparameters)[i] + (*nanocapparameters)[i+1];

	}

	run_bottomup(spent, shex, shept, internalvertices);
	run_topdown(pent, hex, hept, nrofnanocap, internalvertices, rings, exactfaces);
}

int main(int argc, char const *argv[]){	
	int i, pent, hex, hept, counter, rings;
	unsigned char exactfaces;
	int* nanocapparameters;
	pent = hex = hept = -1;
	rings = 0;
	exactfaces = 0;
	if (argc >= 9) {
		nanocapparameters = malloc((argc - 7) * sizeof(int));
		if (nanocapparameters == NULL) {
			fprintf(stderr, "ERROR: malloc returned NULL\n");
			exit(EXIT_FAILURE);
		}
		counter = 0;
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-pent") == 0) {
				i++;
				pent = atoi(argv[i]);
			} else if (strcmp(argv[i], "-hex") == 0) {
				i++;
				hex = atoi(argv[i]);
			} else if (strcmp(argv[i], "-hept") == 0) {
				i++;
				hept = atoi(argv[i]);
			} else if (strcmp(argv[i], "-r") == 0) {
				i++;
				rings = atoi(argv[i]);
			} else if (strcmp(argv[i], "-e") == 0) {
				exactfaces = 1;
			} else {
				nanocapparameters[counter++] = atoi(argv[i]);
			}
		}
		if (pent != -1 && hex != -1 && hept != -1) {
			if ((argc - 7) / 2 > 9 || (rings != 0 && (argc-9)/2 > 9) ) {
				fprintf(stderr, "There are maximum 9 nanocaps allowed");
				exit(EXIT_FAILURE);
			} else {
				run_algorithm(pent, hex, hept, counter / 2, &nanocapparameters, rings, exactfaces);
			}
		} else {
			fprintf(stderr, "Wrong parameters, correct format is ./nanojoin -pent <nr of pentagons> -hex <nr of hexagons> - hept <nr of heptagons> <first parameter first nanocap> <second parameter first nanocap> <first paramater second nanocap> <second parameter second nanocap> ...\n");
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "Wrong parameters, correct format is ./nanojoin -pent <nr of pentagons> -hex <nr of hexagons> - hept <nr of heptagons> <first parameter first nanocap> <second parameter first nanocap> <first paramater second nanocap> <second parameter second nanocap> ...\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
