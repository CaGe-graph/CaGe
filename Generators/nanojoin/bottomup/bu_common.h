#ifndef BU_COMMON_H_
#define BU_COMMON_H_

#define BORDER_SIZE 30
#define MERGEPATH_SIZE 255

//maximum nr of faces
int maxpent;
int maxhex;
int maxhept;
int maxinternalvertices;

struct bu_patch {
	struct bu_patch *lp;
	struct bu_patch *rp;
	char border[BORDER_SIZE];
	int roffset;
	int loffset;
	int internalvertices;
};

struct patcheslist_element {
	struct bu_patch* patch;
	unsigned char pent;
	unsigned char hex;
	unsigned char hept;
	struct patcheslist_element* next;
};

/*
	GLOBALS
*/
struct mergepathlists* mergepathlist;
struct mergepath*** left_mergepaths;
struct mergepath*** right_mergepaths;

//faceswithmergepaths[i][j][k] == 1 says there is a mergepath for a patch with i pentagons, j hexagons and k heptagons
unsigned char*** faceswithmergepaths;

//table that converts (k,l,m) to single index
int*** indextranslate;

#endif /* BU_COMMON_H_ */