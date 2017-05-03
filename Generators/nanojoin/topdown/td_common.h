#ifndef TD_COMMON_H
#define TD_COMMON_H

#include <stdio.h>

typedef int vertextype;

struct edge {
	vertextype start;
	vertextype end;
	struct edge* next;
	struct edge* prev;
	struct edge* inv;
};

struct vertex {
	unsigned char number;
	unsigned char degree;
};

struct td_patch {
	vertextype nrofvertices;
	struct edge* firstedge;
	struct edge* mark;
	unsigned char toborderbuilt;

	struct ufaces* ufaces;
	int maxinternalvertices;
	int* facesleft;
	int* facesused;
};

struct ufaces {
	struct edge* current;
	struct ufaces* next;
	int toborderbuiltnr;
	int faceleftres;
	int pentres;
	int hexres;
	int heptres;
	int ivres;

	struct edge *minedge;
	struct edge *maxedge;
	struct edge *minres;
	struct edge *maxres;
};


/*
	GLOBAL VARIABLES
*/
//unsigned char insideborderfinished;
unsigned char insidenanocaps;
int* outsideparameters;
int* insideparameters;
unsigned char *isovectors;


int* statistics;

unsigned char checknormal;
unsigned char checkinverse;

FILE* output;
FILE* test;
//table that converts (k,l,m) to single index
int*** indextranslate;

int FOUND_JOINS;
int ISO_JOINS;
int ALL_JOINS;

int RINGSTOADD;
unsigned char EXACTFACES;

#endif /* TD_COMMON_H */