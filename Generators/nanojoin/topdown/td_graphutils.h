#ifndef TD_GRAPHUTILS_H
#define TD_GRAPHUTILS_H

struct edge* adddangling(struct edge* edgebefore);
struct edge* addnewvertex(struct td_patch* patch, struct edge* edgebefore);
struct edge* addedge(struct edge* edgebefore1, struct edge* edgebefore2);

int endvertex_degree(struct edge* edge);
struct edge* getNextInFace(struct edge* sedge);
struct edge* getPreviousInFace(struct edge* sedge);

unsigned char get_bordercode(struct edge *startedge, int nrofvertices, unsigned char **bordercode);
void cannonical_edge(struct ufaces* face);
struct edge* cannonical_edge_simple(struct td_patch* patch, struct edge* startedge, unsigned char maxmin);
struct edge* cannonical_edge_simple_large(struct td_patch* patch, struct edge* startedge, unsigned char maxmin);
void print_bordercode(struct edge *startedge, int nrofvertices);
unsigned char isValidUface(struct td_patch* patch, struct ufaces *uface, int** arr);
unsigned char areConnected(struct edge *first, struct edge *second);

#endif /* TD_GRAPHUTILS_H */