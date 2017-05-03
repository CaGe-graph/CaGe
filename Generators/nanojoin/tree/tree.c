
#include <stdio.h>
#include <stdlib.h>

#include "../layer.h"
#include "tree.h"

void initialize_tree() {
	treeroot = malloc(sizeof(struct node));
	treeroot->children[0] = NULL;
	treeroot->children[1] = NULL;
	treeroot->stop = NULL;
}

struct node* go_nexthard(struct node *current, unsigned char direction) {
	struct node *next;
	next = current->children[direction];
	if (next == NULL) {
		next = malloc(sizeof(struct node));
		next->children[0] = NULL;
		next->children[1] = NULL;
		next->stop = NULL;
		current->children[direction] = next;
	}
	return next;
}

struct node* go_nextsoft(struct node* current, unsigned char direction) {
	struct node *next;
	next = current->children[direction];
	return next;
}

int minindex(char *border) {
	int i, j, k, size;
	unsigned char found, ismaxmin;
	//transform face to bordercode
	size = border[0];
	ismaxmin = 0;
	i = 0;
	while (i < size && !ismaxmin) {
		j = 1;
		found = 0;
		while(!found && j < size) {
			k = 0;
			while(k < size && border[(i+k) % size + 1] == border[(i+j+k) % size + 1]) {
				k++;
			}
			if (k != size && border[(i+k) % size + 1] > border[(i+j+k) % size + 1]) {
				i = i+j;
				found = 1;
			}
			j++;
		}
		if (j == size) {
			ismaxmin = 1;
		}
	}
	return i;
}


/*
You can add normal becuase of mirror images
*/
void tree_add(char *border, int pent, int hex, int hept, int internalvertices) {
	int i, j, size, digit;
	struct node *current;
	struct stopnode *stop;
	current = treeroot;

	/*
	mindex = minindex(border);
	current = go_nexthard(current, 1);
	size = border[0];
	for (i=0; i < size; i++) {
		digit = border[(mindex + i) % size + 1];
		for (j=0; j <= digit; j++) {
			if (i != size - 1 || j != digit) {
				current = go_nexthard(current, 1);
				if (j != digit) {
					current = go_nexthard(current, 0);
				}
			}
		}
	}
	*/

	size = border[0];
	for (i=0; i < size; i++) {
		digit = border[i+1];
		for (j=0; j < digit; j++) {
			if (i != 0 || j != 0) {
				current = go_nexthard(current, 1);
			}
			current = go_nexthard(current, 0);
		}
		if (i != 0 || digit != 0) {
			current = go_nexthard(current, 1);
		}
	}
	current = go_nexthard(current, 1);

	stop = current->stop;
	if (stop == NULL) {
		stop = malloc(sizeof(struct stopnode));
		stop->minpent = pent;
		stop->minhex = hex;
		stop->minhept = hept;
		stop->mininternal = internalvertices;
		current->stop = stop;
	} else {
		if (stop->minpent > pent) {
			stop->minpent = pent;
		}
		if (stop->minhex > hex) {
			stop->minhex = hex;
		}
		if (stop->minhept > hept) {
			stop->minhept = hept;
		}
		if (stop->mininternal > internalvertices) {
			stop->mininternal = internalvertices;
		}
	}
}

unsigned char tree_lookup(unsigned char *border, int **result) {
	struct node *current;
	struct stopnode *stop;
	int i;
	
	current = treeroot;	
	for(i=1; i <= border[0]; i++) {
		current = go_nextsoft(current, border[i]);
		if (current == NULL) {
			return 0;
		}
	}
	stop = current->stop;
	if (stop == NULL) {
		return 0;
	} else {
		(*result)[0] = stop->minpent;
		(*result)[1] = stop->minhex;
		(*result)[2] = stop->minhept;
		(*result)[3] = stop->mininternal;
	}
	return 1;
}