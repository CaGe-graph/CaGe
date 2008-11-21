/*
 *  twopentagons.c
 *  
 *
 *  Created by Nico Van Cleemput on 25/09/08.
 *
 */

#include "twopentagons.h"
#include "cone.h"
#include "util.h"
#include <stdio.h>

/* int getTwoPentagonsPatch(int sside, int symmetric, int mirror) */
/*
	generates the canonical cone patches with two pentagons and the given boundary
*/
int getTwoPentagonsPatch(int sside, int symmetric, int mirror){
	int i;

	if(symmetric){
		//sside patches with spiral code i, 2*sside + i (i=0,...,sside-1)
		int upperbound = sside;
		if(!mirror){
			upperbound = halfFloor(sside) + 1;
		}
		for(i=0;i<upperbound;i++){
			fprintf(stderr,"\nnew patch with first pentagon on position %d\n", i+1);
			int vertexCounter = 0;
			//first create the boundary
			EDGE *boundaryStart = createBoundary(sside, symmetric, 2, &vertexCounter);
			int code[2];
			code[0] = i + 1;
			code[1] = 2*sside+ i + 1;
			if(patchFromSpiralCode(boundaryStart, code, 2, &vertexCounter)){
				//---------
				fprintf(stderr, "vertices: %d\n", vertexCounter);
				exportPlanarGraphCode(boundaryStart, vertexCounter);
				//exportPlanarGraphTable(boundaryStart, vertexCounter);
				fprintf(stderr, "\n");
			}
		}
		return upperbound;
	} else {
		int lowerbound = sside+1;
		int upperbound = 3*sside+1;
		if(!mirror){
			lowerbound += halfFloor(sside+1);
			upperbound -= halfFloor(sside);
		}
		for(i=lowerbound;i<=upperbound;i++){
			fprintf(stderr,"\nnew patch with first pentagon on position %d\n", i+1);
			int vertexCounter = 0;
			//first create the boundary
			EDGE *boundaryStart = createBoundary(sside, symmetric, 2, &vertexCounter);
			int code[2];
			code[0] = i + 1;
			code[1] = 2*sside+ i + 1 + 1;
			if(patchFromSpiralCode(boundaryStart, code, 2, &vertexCounter)){
				//---------
				fprintf(stderr, "vertices: %d\n", vertexCounter);
				exportPlanarGraphCode(boundaryStart, vertexCounter);
				//exportPlanarGraphTable(boundaryStart, vertexCounter);
				fprintf(stderr, "\n");
			}
		}
		return upperbound - lowerbound + 1;
	}
}

