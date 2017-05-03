#ifndef LAYER_H
#define LAYER_H

int spent;
int shex;
int shept;

void tree_add(char *orig_border, int pent, int hex, int hept, int internalvertices);
unsigned char tree_lookup(unsigned char *border, int **result);

#endif /* LAYER_H */