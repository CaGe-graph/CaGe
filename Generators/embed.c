
/* --------------------------------------------------------------------	*
 *	graph.h					16-aug-2000  by ODF	*
 * --------------------------------------------------------------------	*/

/* CVS: $Id: graph.h,v 1.2 2001/04/20 13:00:52 delgado Exp $ */

#ifndef _h_graph
#define _h_graph 1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* --- typedefs ------------------------------------------------------- */

struct Edge {
  int number;
  int start;
  int end;
  struct Edge *prev;
  struct Edge *next;
  struct Edge *inverse;
};
typedef struct Edge EDGE;

struct Graph {
  int  size;
  EDGE **map;
};
typedef struct Graph GRAPH;

struct Positioning {
  int size;
  int dim;
  double *pos;
};
typedef struct Positioning POSITIONING;


/* --- defines, enums and global variables ---------------------------- */

#define PICK(P,n,i) ((P)->pos[(n-1)*(P)->dim+(i)])

enum err_stat
{
  OK = 0,
  BAD_ARGS,
  NO_MEMORY,
  BAD_INPUT,
  VERTEX_IN_USE,
  GRAPH_TOO_LARGE,
  SYSTEM_ERROR
};

extern int status;


/* --- function prototypes -------------------------------------------- */

extern int
write_result(GRAPH *G, POSITIONING *P, FILE *f, char output_format);

/* --- */

extern GRAPH *
new_graph(int size);

extern int
resize_graph(GRAPH *G, int size);

extern void
free_graph(GRAPH *G);

extern GRAPH *
copy_of_graph(GRAPH *G);

extern POSITIONING *
new_positioning(int size, int dim);

extern int
resize_positioning(POSITIONING *P, int size, int dim);

extern void
free_positioning(POSITIONING *P);

/* --- */

extern int
readgraph_vega(FILE *fp, GRAPH *G, POSITIONING *P);

extern int
writegraph_vega(FILE *fp, GRAPH *G, POSITIONING *P);

extern int
writegraph_planar(FILE *fp, GRAPH *G);

extern int
writegraph_pdb(FILE *fp, GRAPH *G, POSITIONING *P);

/* --- */

extern int
largest_edge_number(GRAPH *G);

extern int *
make_edge_flags (GRAPH *G, int fill);

extern int
mark_face(EDGE *e0, int *flags, int val);

extern EDGE **
edge_reps (GRAPH *G);

extern EDGE **
face_reps (GRAPH *G);

extern EDGE *
find_edge(GRAPH *G, int start, int end);

extern int
find_edge_from_contained_point(GRAPH *G, POSITIONING *P,
                              double x, double y, int *startp, int *endp);

extern int
degree_of_vertex(EDGE *e);

extern int
degree_of_face(EDGE *e);

extern int *
vertices_in_face(GRAPH *G, EDGE *e);

extern int *
neighbors_of_vertex(GRAPH *G, EDGE *e);

/* --- */

extern int
graph_has_symmetry(GRAPH *G, EDGE *a0, EDGE *b0);

extern int
symmetry_at_face(GRAPH *G, EDGE *e);

extern EDGE **
spanning_edges_breadth_first(GRAPH *G, EDGE *start, int at_face);

extern int *
vertices_breadth_first(GRAPH *G, EDGE *start, int at_face, int all);

extern int *
vertex_depths_breadth_first(GRAPH *G, EDGE *start, int at_face);

extern int
maxdepth_breadth_first(GRAPH *G, EDGE *start, int at_face);

extern double
outer_curvature(GRAPH *G, EDGE *e0);

/* --- */

extern int
bfs_renumber_graph(GRAPH *G, EDGE *start, int at_face);

extern EDGE *
new_edge_pair(int start, int end, int n1, int n2,
	      EDGE *prev_at_start, EDGE *prev_at_end);

extern int
split_edges(GRAPH *G, int *forbidden);

extern int
triangulate(GRAPH *G, int *forbidden);

/* --- */

extern void
copy_position(POSITIONING *Q, int w, POSITIONING *P, int v);

extern double
squared_dist_positions(POSITIONING *P, int v, POSITIONING *Q, int w);

extern double
squared_norm_position(POSITIONING *P, int v);

extern void
limit_dist_positions(POSITIONING *P, int v, POSITIONING *Q, int w, double t);

extern void
clear_positioning(POSITIONING *P);

extern void
copy_positioning(POSITIONING *Q, POSITIONING *P);

extern void
scale_positioning(POSITIONING *P, double f);

extern void
shift_positioning(POSITIONING *P, double *t, double f);

extern void
xz_swap_positioning(POSITIONING *P);

extern double *
center_of_positioning(POSITIONING *P);

extern int
recenter_positioning(POSITIONING *P);

extern double
squared_dist_positionings(POSITIONING *P, POSITIONING *Q);

extern double
average_edge_length(GRAPH *G, POSITIONING *P);

/* -------------------------------------------------------------------- */

#endif /* _h_graph */

/* --- EOF graph.h --- */
/* --------------------------------------------------------------------	*
 *	graph.c  				18-aug-2000  by ODF	*
 * --------------------------------------------------------------------	*/

/* CVS: $Id: graph.c,v 1.3 2001/05/10 14:12:20 delgado Exp $ */

#ifndef _h_graph
#include "graph.h"
#endif

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

int status;

/* --------------------------------------------------------------------	*/

GRAPH *
new_graph(int size)
{
  GRAPH *G;

  status = OK;

  if (size < 0) {
    status = BAD_ARGS;
    return NULL;
  }

  G = (GRAPH *) malloc(sizeof(GRAPH));
  if (G == NULL) {
    status = NO_MEMORY;
    return NULL;
  }

  G->size = size;
  G->map  = (EDGE**) calloc(size + 1, sizeof(EDGE*));
  if (G->map == NULL) {
    status = NO_MEMORY;
    free(G);
    return NULL;
  }

  return G;
}


int
resize_graph(GRAPH *G, int size)
{
  EDGE **new_map;
  int i, smaller;

  if (size == G->size)
    return 1;

  if (size < 0) {
    status = BAD_ARGS;
    return 0;
  }
    
  if (size > G->size)
    smaller = G->size;
  else {
    for (i = size + 1; i <= G->size; i++) {
      if (G->map[i]) {
	status = VERTEX_IN_USE;
	return 0;
      }
    }
    smaller = size;
  }

  new_map  = (EDGE**) calloc(size + 1, sizeof(EDGE*));
  if (new_map == NULL) {
    status = NO_MEMORY;
    return 0;
  }
  else {
    if (G->map) {
      memcpy(new_map, G->map, (smaller + 1) * sizeof(EDGE*));
      free(G->map);
    }
    G->map = new_map;
    G->size = size;
    return 1;
  }
}


static void
clear_edges(GRAPH *G)
{
  EDGE *e;
  int i;

  if (G && G->map) {
    for (i = 1; i <= G->size; i++) {
      e = G->map[i];
      if (e) {
	e->prev->next = NULL;
	while (e->next) {
	  e = e->next;
	  free(e->prev);
	}
	free(e);
      }
    }
  }
}


void
free_graph(GRAPH *G)
{
  if (G) {
    if (G->map) {
      clear_edges(G);
      free(G->map);
    }
    free(G);
  }
}


GRAPH *
copy_of_graph(GRAPH *G)
{
  GRAPH *C;
  int i, n, max_edge_nr;
  EDGE *e, *e0, *edge;
  EDGE **new_edges;

  C = new_graph(G->size);
  if (C == NULL)
    return NULL;

  max_edge_nr = 0;
  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (e->number > max_edge_nr)
	max_edge_nr = e->number;
      e = e->next;
    } while (e != e0);
  }

  new_edges = (EDGE **)calloc(max_edge_nr+1, sizeof(EDGE *));
  if (new_edges == NULL) {
    status = NO_MEMORY;
    free_graph(C);
    return NULL;
  }

  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      n = e->number;
      new_edges[n] = (EDGE *)malloc(sizeof(EDGE));
      if (new_edges[n] == NULL) {
	status = NO_MEMORY;
	free_graph(C);
	for (i = 0; i <= max_edge_nr; i++)
	  free(new_edges[i]);
	free(new_edges);
	return NULL;
      }
      e = e->next;
    } while (e != e0);
  }

  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    C->map[i] = new_edges[e->number];
    do {
      edge = new_edges[e->number];
      edge->number  = e->number;
      edge->start   = e->start;
      edge->end     = e->end;
      edge->next    = new_edges[e->next->number];
      edge->prev    = new_edges[e->prev->number];
      edge->inverse = new_edges[e->inverse->number];
      e = e->next;
    } while (e != e0);
  }

  free(new_edges);
  return C;
}


/* -------------------------------------------------------------------- */

POSITIONING *
new_positioning(int size, int dim)
{
  POSITIONING *P;

  status = OK;

  if (size < 0 || dim < 0) {
    status = BAD_ARGS;
    return NULL;
  }

  P = (POSITIONING *) malloc(sizeof(POSITIONING));
  if (P == NULL) {
    status = NO_MEMORY;
    return NULL;
  }

  P->size = size;
  P->dim  = dim;
  P->pos  = NULL;

  if (dim > 0) {
    P->pos  = (double *) calloc((size+1) * dim, sizeof(double));
    if (P->pos == NULL) {
      status = NO_MEMORY;
      free(P);
      return NULL;
    }
  }

  return P;
}


int
resize_positioning(POSITIONING *P, int size, int dim)
{
  POSITIONING *new_P;
  int i, j, min_sz, min_d;

  if (size == P->size && dim == P->dim)
    return 1;

  if (size < 0 && dim < 0) {
    status = BAD_ARGS;
    return 0;
  }
    
  if (size <= P->size)
    min_sz = size;
  else
    min_sz = P->size;
  if (dim <= P->dim)
    min_d = dim;
  else
    min_d = P->dim;

  new_P = new_positioning(size, dim);
  if (new_P == NULL)
    return 0;

  for (i = 1; i <= min_sz; i++)
    for (j = 0; j < min_d; j++)
      PICK(new_P, i, j) = PICK(P, i, j);

  P->size = size;
  P->dim  = dim;

  free(P->pos);
  P->pos  = new_P->pos;
  free(new_P);

  return 1;
}


void
free_positioning(POSITIONING *P)
{
  if (P) {
    free(P->pos);
    free(P);
  }
}


/* --------------------------------------------------------------------	*/
/* The following function reads a graph in Vega format.			*/

#define MAX_LINE_LENGTH 3000

int
readgraph_vega(FILE *fp, GRAPH *G_out, POSITIONING *P_out)
{
  char		line[MAX_LINE_LENGTH];	/* input buffer			*/
  char		*remain_line;		/* remaining line		*/
  int		read_char;		/* number of read characters	*/
  int		lineno;			/* number of current line	*/
  int		alloc;			/* current size of pos and map	*/
  int		tmp;			/* temporary variable		*/

  int		vert;			/* current vertex number	*/
  double	x, y, z;		/* position of current vertex	*/
  int		neighbor;		/* vertex number of neighbor	*/
  EDGE		*edge;			/* the current edge		*/
  EDGE		*prev_edge;		/* the previous edge		*/
  EDGE		*te;			/* temporary edge pointer	*/

  int		nv;			/* number of vertices so far	*/
  int		ne;			/* number of edges so far	*/
  int           dim;			/* dimension of positioning	*/
  GRAPH		*G;			/* the final graph		*/
  POSITIONING	*P;			/* the final positioning	*/

  /* --- initialize --- */

  nv     = 0;
  ne     = 0;
  dim    = 0;
  lineno = 0;
  alloc  = 0;

  prev_edge = NULL;
  G = NULL;
  P = NULL;

  x = y = z = 0.0;

  /* --- read the input one line at a time --- */

  while(fgets(line, MAX_LINE_LENGTH, fp) == line)
  {
    /* --- check if a complete line was read --- */

    if (line[strlen(line)-1] != '\n') {
      status = BAD_INPUT;
      goto fail;
    }

    /* --- keep track of the line number --- */

    lineno++;

    /* --- first line is special --- */

    if (lineno == 1)
    {
      /* --- handle the format tag if present --- */

      sscanf(line, " %n", &read_char);
      if (line[read_char] == '>') {
	if (strncmp(line+read_char, ">>writegraph3d", 14) == 0)
	  dim = 3;
	else if (strncmp(line+read_char, ">>writegraph2d", 14) == 0)
	  dim = 2;
	else {
	  status = BAD_INPUT;
	  goto fail;
	}
	continue;
      }
    }

    /* --- read the vertex number --- */

    if(sscanf(line,"%d %n", &vert, &read_char) !=1)
      continue;
    remain_line = &line[read_char];

    /* --- number 0 indicates end of current graph --- */

    if(vert==0)
      break;

    /* --- complain if not the expected number --- */

    if(vert != ++nv) {
      status = BAD_INPUT;
      goto fail;
    }

    /* --- read the first two coordinates for this vertex --- */

    if(sscanf(remain_line,"%lf %lf %n", &x, &y, &read_char) < 2) {
      status = BAD_INPUT;
      goto fail;
    }
    remain_line += read_char;

    /* --- try to figure out the dimension, if unknown --- */

    if (dim == 0) {
      if(sscanf(remain_line,"%d %n", &tmp, &read_char) < 1)
	dim = 2;
      else if (tmp <= 0 || (remain_line[read_char]
			    && strchr(".eE", remain_line[read_char])))
	dim = 3;
      else
	dim = 2;
    }

    /* --- read the third coordinate if needed --- */

    if (dim == 3) {
      if(sscanf(remain_line,"%lf %n", &z, &read_char) < 1) {
	status = BAD_INPUT;
	goto fail;
      }
      remain_line += read_char;
    }

    /* --- re-allocate map and pos if full --- */

    if (nv >= alloc) {
      if (alloc == 0) {
	alloc = 10;
	G = new_graph(alloc);
	P = new_positioning(alloc, dim);
	if (G == NULL || P == NULL) {
	  status = NO_MEMORY;
	  goto fail;
	}
      }
      else {
	alloc *= 2;
	if (!resize_graph(G, alloc) || !resize_positioning(P, alloc, dim)) {
	  status = NO_MEMORY;
	  goto fail;
	}
      }
    }

    /* --- store coordinate values --- */

    PICK(P, nv, 0) = x;
    PICK(P, nv, 1) = y;
    if (dim == 3)
      PICK(P, nv, 2) = z;

    /* --- read neighbor list and create directed edges --- */

    G->map[nv] = NULL;

    while(sscanf(remain_line, "%d%n", &neighbor, &read_char) == 1) {
      remain_line += read_char;

      edge = (EDGE *) malloc(sizeof(EDGE));
      if (edge == NULL) {
	status = NO_MEMORY;
	goto fail;
      }
      edge->number  = ++ne;
      edge->start   = nv;
      edge->end     = neighbor;
      edge->next    = NULL;
      edge->prev    = NULL;
      edge->inverse = NULL;

      if (G->map[nv] == NULL)
	G->map[nv] = edge;
      else {
	prev_edge->next = edge;
	edge->prev = prev_edge;
      }

      if (neighbor < nv && G->map[neighbor] != NULL) {
	te = G->map[neighbor];
	do {
	  if (te->end == nv) {
	    te->inverse = edge;
	    edge->inverse = te;
	    break;
	  }
	  te = te->next;
	} while (te != G->map[neighbor]);
      }

      prev_edge = edge;
    }

    if (G->map[nv] != NULL) {
      prev_edge->next = G->map[nv];
      G->map[nv]->prev = prev_edge;
    }

    /* --- processing of this input line is done --- */
  }

  /* --- Check if every edge has an inverse --- */

  for (vert = 1; vert <= nv; vert++) {
    te = G->map[vert];
    do {
      if (!te->inverse) {
	status = BAD_INPUT;
	goto fail;
      }
      te = te->next;
    } while (te != G->map[vert]);
  }

  /* --- shrink map and pos to final size --- */

  if (alloc > nv) {
    alloc = nv;
    if (!resize_graph(G, alloc) || !resize_positioning(P, alloc, dim)) {
      status = NO_MEMORY;
      goto fail;
    }
  }

  /* --- set output variables --- */

  if (G_out) {
    if (G_out->map)
      clear_edges(G_out);
    free(G_out->map);
    G_out->size = G->size;
    G_out->map = G->map;
  }
  else if (G)
    free_graph(G);

  if (P_out) {
    free(P_out->pos);
    P_out->size = P->size;
    P_out->dim = P->dim;
    P_out->pos = P->pos;
  }
  else if (P)
    free_positioning(P);

  return 1;

  /* --- clean up in case of error --- */
 fail:
  free_graph(G);
  free_positioning(P);

  return 0;
}


/* --------------------------------------------------------------------	*/
/* The following function writes a graph in Vega format.		*/

int
writegraph_vega(FILE *fp, GRAPH *G, POSITIONING *P)
{
  int dim, i, j;
  double x;
  EDGE *e, *e0;

  if (P)
    dim = P->dim;
  else
    dim = 2;

  if (fprintf(fp, ">>writegraph%dd<<\n", dim) <= 0)
    goto fail;

  for (i = 1; i <= G->size; i++) {
    if (fprintf(fp, "%3d", i) <= 0)
      goto fail;
    for (j = 0; j < dim; j++) {
      if (P)
	x = PICK(P, i, j);
      else
	x = 0.0;
      if (fprintf(fp, " %8.3f", x) <= 0)
	goto fail;
    }

    e = e0 = G->map[i];
    if (e != NULL) {
      do {
	if (fprintf(fp,  " %3d", e->end) <= 0)
	  goto fail;
	e = e->next;
      } while (e != e0);
    }

    if (fprintf(fp, "\n") <= 0)
      goto fail;
  }
  if (fprintf(fp, "  0\n\n") <= 0)
    goto fail;

  if (fflush(fp) != 0)
    goto fail;

  return 1;

 fail:
  status = SYSTEM_ERROR;
  return 0;
}


/* --------------------------------------------------------------------	*/
/* The following function writes a graph in planar code.		*/

int
writegraph_planar(FILE *fp, GRAPH *G)
{
  int  i, n;
  EDGE *e, *e0;
  int large_graph;

  if (G->size > 65535) {
    status = GRAPH_TOO_LARGE;
    return 0;
  }
  else
    large_graph = (G->size > 255);


  if (fprintf(fp, ">>planar_code le<<") <= 0)
    goto fail;
  if (large_graph)
    if (fputc(0, fp) < 0)
      goto fail;

  n = G->size;
  if (fputc(n % 256, fp) < 0)
    goto fail;
  if (large_graph)
    if (fputc(n / 256, fp) < 0)
      goto fail;

  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e != NULL) {
      do {
	n = e->end;
	if (fputc(n % 256, fp) < 0)
	  goto fail;
	if (large_graph)
	  if (fputc(n / 256, fp) < 0)
	    goto fail;
	e = e->next;
      } while (e != e0);
    }

    if (fputc(0, fp) < 0)
      goto fail;
    if (large_graph)
      if (fputc(0, fp) < 0)
	goto fail;
  }

  if (fflush(fp) != 0)
    goto fail;

  return 1;

 fail:
  status = SYSTEM_ERROR;
  return 0;
}

/* --------------------------------------------------------------------	*/
/* The following function writes in Brookhaven pdb format.		*/

int
writegraph_pdb(FILE *fp, GRAPH *G, POSITIONING *P)
{
  int  n, i, success;
  char type;
  EDGE *e, *e0;

  n = G->size;
  for (i = 1; i <= n; i++) {
    if (G->map[i] == G->map[i]->next)
      type = 'H';
    else
      type = 'C';

    success = fprintf(fp, "ATOM  %5d  %c                %8.3f%8.3f%8.3f\n",
		      i, type, PICK(P, i, 0), PICK(P, i, 1), PICK(P, i, 2));
    if (success <= 0)
      goto fail;
  }

  for (i = 1; i <= n; i++) {
    if (fprintf(fp, "CONECT%5d", i) <= 0)
      goto fail;
    e = e0 = G->map[i];
    if (e != NULL) {
      do {
	if (fprintf(fp,  "%5d", e->end) <= 0)
	  goto fail;
	e = e->next;
      } while (e != e0);
    }

    if (fprintf(fp, "\n") <= 0)
      goto fail;
  }
  if (fprintf(fp, "END\n") <= 0)
    goto fail;

  if (fflush(fp) != 0)
    goto fail;

  return 1;

 fail:
  status = SYSTEM_ERROR;
  return 0;
}

/* --------------------------------------------------------------------	*/


int
largest_edge_number(GRAPH *G)
{
  int i, ne;
  EDGE *e, *e0;

  ne = 0;
  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (e->number > ne)
	ne = e->number;
      e = e->next;
    } while (e != e0);
  }

  return ne;
}


int *
make_edge_flags (GRAPH *G, int fill)
{
  int i, max_edge_nr;
  int *res;

  max_edge_nr = largest_edge_number(G);

  res = (int *) calloc(max_edge_nr+1, sizeof(int));
  if (res == NULL) {
    status = NO_MEMORY;
    return NULL;
  }

  for (i = 1; i <= max_edge_nr; i++)
    res[i] = fill;

  return res;
}


int
mark_face(EDGE *e0, int *flags, int val)
{
  EDGE *e = e0;

  do {
    flags[e->number] = val;
    e = e->inverse->prev;
  } while (e != e0);

  return 1;
}


EDGE **
edge_reps (GRAPH *G)
{
  int i, j;
  EDGE *e, *e0;
  EDGE **reps = NULL;
  int *seen = make_edge_flags(G, 0);
  int *is_rep = make_edge_flags(G, 0);
  int nr_edges = 0;

  if (seen == NULL) {
    status = NO_MEMORY;
    goto done;
  }

  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (!seen[e->number]) {
	nr_edges++;
	seen[e->number] = seen[e->inverse->number] = 1;
	is_rep[e->number] = 1;
      }
      e = e->next;
    } while (e != e0);
  }

  reps = (EDGE **) calloc(nr_edges + 1, sizeof(EDGE *));
  if (reps == NULL) {
    status = NO_MEMORY;
    goto done;
  }

  for (i = 1, j = 0; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (is_rep[e->number])
	reps[j++] = e;
      e = e->next;
    } while (e != e0);
  }
  reps[j] = NULL;

 done:
  free(seen);
  free(is_rep);

  return reps;
}


EDGE **
face_reps (GRAPH *G)
{
  int i, j;
  EDGE *e, *e0;
  EDGE **reps = NULL;
  int *seen = make_edge_flags(G, 0);
  int *is_rep = make_edge_flags(G, 0);
  int nr_faces = 0;

  if (is_rep == NULL || seen == NULL)
    goto done;

  for (i = 1; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (!seen[e->number]) {
	nr_faces++;
	is_rep[e->number] = 1;
	mark_face(e, seen, 1);
      }
      e = e->next;
    } while (e != e0);
  }

  reps = (EDGE **) calloc(nr_faces + 1, sizeof(EDGE *));
  if (reps == NULL) {
    status = NO_MEMORY;
    goto done;
  }

  for (i = 1, j = 0; i <= G->size; i++) {
    e = e0 = G->map[i];
    if (e == NULL)
      continue;
    do {
      if (is_rep[e->number])
	reps[j++] = e;
      e = e->next;
    } while (e != e0);
  }
  reps[j] = NULL;

 done:
  free(seen);
  free(is_rep);

  return reps;
}


EDGE *
find_edge(GRAPH *G, int start, int end)
{
  EDGE *e0, *e1, *e2;

  if (start > G->size || end > G->size)
    return NULL;

  e0 = e1 = G->map[start];
  do {
    e2 = e1;
    while (1) {
      if (e2->end == end)
	return e1;
      if (degree_of_vertex(e2->inverse) == 2)
	e2 = e2->inverse->next;
      else
	break;
    }
    e1 = e1->next;
  } while (e1 != e0);

  return NULL;
}


int
find_edge_from_contained_point(GRAPH *G, POSITIONING *P,
			       double px, double py, int *startp, int *endp)
{
  double x1, y1, x2, y2, ye, yemin;
  int vertex, neighbor, edge_below_found = 0;
  EDGE **edges, *edge, *first_edge, *edge_above_found = NULL;

  for (edges = G->map + 1, vertex = 1; vertex <= G->size; ++vertex, ++edges) {
    x1 = PICK(P, vertex, 0);
    y1 = PICK(P, vertex, 1);
    if ((first_edge = edge = *edges) != NULL) {
      do {
	neighbor = edge->end;
	x2 = PICK(P, neighbor, 0);
	y2 = PICK(P, neighbor, 1);
	/* --- consider edges whose x coordinates "include" px --- */
	if (x1 != x2 && ((x1 <= px && x2 >= px) || (x1 >= px && x2 <= px))) {
	  /* --- (px,ye) is the intersection of the edge
	         with a ray extending upwards from (px,py) --- */
	  ye = y1 + (px - x1) * (y2 - y1) / (x2 - x1);
	  if (ye > py) {
	    /* --- the extended edge is above (px,py) --- */
	    if (!edge_above_found || ye < yemin) {
	      /* --- remember this edge --- */
	      yemin = ye;
	      edge_above_found = edge;
	    }
	  } else {
	    edge_below_found = 1;
	  }
	}
	edge = edge->next;
      } while (edge != first_edge);
    }
  }

  if (edge_above_found && edge_below_found) {
    if (PICK(P,edge_above_found->start,0) < PICK(P,edge_above_found->end,0)) {
      vertex = edge_above_found->start;
      neighbor = edge_above_found->end;
    } else {
      neighbor = edge_above_found->start;
      vertex = edge_above_found->end;
    }
    *startp = vertex;
    *endp = neighbor;
    return 1;
  } else {
    return 0;
  }
}


int
degree_of_vertex(EDGE *e)
{
  EDGE *e1;
  int deg;

  e1 = e;
  deg = 0;

  do {
    ++deg;
    e1 = e1->next;
  } while (e1 != e);

  return deg;
}


int
degree_of_face(EDGE *e)
{
  EDGE *e1;
  int deg;

  e1 = e;
  deg = 0;

  do {
    ++deg;
    e1 = e1->inverse->prev;
  } while (e1 != e);

  return deg;
}


int *
vertices_in_face(GRAPH *G, EDGE *e)
{
  EDGE *e1;
  int v, nv;
  int *seen, *verts;

  seen = (int *)calloc(G->size + 1, sizeof(int));
  verts = (int *)calloc(degree_of_face(e) + 1, sizeof(int));
  if (seen == NULL || verts == NULL) {
    status = NO_MEMORY;
    free(seen);
    free(verts);
    return NULL;
  }

  e1 = e;
  nv = 0;
  do {
    v = e1->start;
    if (!seen[v]) {
      seen[v] = 1;
      verts[nv++] = v;
    }
    e1 = e1->inverse->prev;
  } while (e1 != e);

  free(seen);
  return verts;
}


int *
neighbors_of_vertex(GRAPH *G, EDGE *e)
{
  EDGE *e1;
  int nv, v;
  int *seen, *verts;

  seen = (int *)calloc(G->size + 1, sizeof(int));
  verts = (int *)calloc(degree_of_vertex(e) + 1, sizeof(int));
  if (seen == NULL || verts == NULL) {
    status = NO_MEMORY;
    free(seen);
    free(verts);
    return NULL;
  }

  e1 = e;
  nv = 0;
  do {
    v = e1->end;
    if (!seen[v]) {
      seen[v] = 1;
      verts[nv++] = v;
    }
    e1 = e1->next;
  } while (e1 != e);

  free(seen);
  return verts;
}


/* --------------------------------------------------------------------	*/

int
graph_has_symmetry(GRAPH *G, EDGE *a0, EDGE *b0)
{
  int max_edge_nr, v, w, head, tail, res;
  int *map, *queue;
  EDGE *a, *b;
  EDGE **edge_list;

  max_edge_nr = 0;
  for (v = 1; v <= G->size; v++) {
    a = b = G->map[v];
    if (a == NULL)
      continue;
    do {
      if (b->number > max_edge_nr)
	max_edge_nr = b->number;
      b = b->next;
    } while (b != a);
  }

  map = (int *) calloc(max_edge_nr+1, sizeof(int));
  queue = (int *) calloc(max_edge_nr+1, sizeof(int));
  edge_list = (EDGE **) calloc(max_edge_nr+1, sizeof(EDGE *));
  if (map == NULL || queue == NULL || edge_list == NULL) {
    status = NO_MEMORY;
    res = -1;
    goto done;
  }

  for (v = 1; v <= G->size; v++) {
    a = b = G->map[v];
    if (a == NULL)
      continue;
    do {
      edge_list[b->number] = b;
      b = b->next;
    } while (b != a);
  }

  res = 0;

  head = tail = 0;
  map[a0->number] = b0->number;
  queue[head++] = a0->number;
  while (tail < head) {
    v = queue[tail++];
    a = edge_list[v];
    b = edge_list[map[v]];

    v = a->next->number;
    w = b->next->number;
    if (map[v] == 0) {
      map[v] = w;
      queue[head++] = v;
    }
    else if (map[v] != w)
      goto done;

    v = a->inverse->number;
    w = b->inverse->number;
    if (map[v] == 0) {
      map[v] = w;
      queue[head++] = v;
    }
    else if (map[v] != w)
      goto done;
  }
  res = 1;

done:
  free(map);
  free(queue);
  free(edge_list);
  return res;
}


int
symmetry_at_face(GRAPH *G, EDGE *e)
{
  EDGE *k;
  int deg, i, check;

  deg = degree_of_face(e);
  k = e;
  i = 0;

  while (i < deg/2) {
    ++i;
    k = k->next->inverse;
    check = graph_has_symmetry(G, e, k);
    if (check < 0)
      return 0; /* error */
    else if (check > 0)
      return deg / i;
  }

  return 1;
}


EDGE **
spanning_edges_breadth_first(GRAPH *G, EDGE *start, int at_face)
{
  int *seen, *queue;
  int n, v;
  int head, tail, ne;
  EDGE *e0, *e;
  EDGE **edges;

  n = G->size;
  seen = (int *)calloc(n+1, sizeof(int));
  queue = (int *)calloc(n+1, sizeof(int));
  edges = (EDGE **)calloc(n+1, sizeof(EDGE *));

  if (seen == NULL || queue == NULL || edges == NULL) {
    status = NO_MEMORY;
    free(seen);
    free(queue);
    free(edges);
    return NULL;
  }

  head = tail = ne = 0;

  if (at_face) {
    e = e0 = start;
    do {
      v = e->start;
      if (!seen[v]) {
       seen[v] = 1;
       queue[head++] = v;
      }
      e = e->inverse->prev;
    } while (e != e0);
  }
  else {
    v = start->start;
    seen[v] = 1;
    queue[head++] = v;
  }

  while (tail < head) {
    v = queue[tail++];
    e = e0 = G->map[v];
    do {
      v = e->end;
      if (!seen[v]) {
	seen[v] = 1;
	queue[head++] = v;
	edges[ne++] = e;
      }
      e = e->next;
    } while (e != e0);
  }
  edges[ne] = NULL;

  free(seen);
  free(queue);
  return edges;
}


int *
vertices_breadth_first(GRAPH *G, EDGE *start, int at_face, int all)
{
  int *seen, *queue, *verts;
  int n, v;
  int head, tail, nv;
  EDGE *e0, *e;

  n = G->size;
  seen  = (int *)calloc(n+1, sizeof(int));
  queue = (int *)calloc(n+1, sizeof(int));
  verts = (int *)calloc(n+1, sizeof(int));

  if (seen == NULL || queue == NULL || verts == NULL) {
    status = NO_MEMORY;
    free(seen);
    free(queue);
    free(verts);
    return NULL;
  }

  head = tail = nv = 0;

  if (at_face) {
    e = e0 = start;
    do {
      v = e->start;
      if (!seen[v]) {
       seen[v] = 1;
       queue[head++] = v;
       if (all)
	 verts[nv++] = v;
      }
      e = e->inverse->prev;
    } while (e != e0);
  }
  else {
    v = start->start;
    seen[v] = 1;
    queue[head++] = v;
    if (all)
      verts[nv++] = v;
  }

  while (tail < head) {
    v = queue[tail++];
    e = e0 = G->map[v];
    if (!seen[e->end]) {
      do {
	e = e->next;
      } while (!seen[e->end] && e != e0);
      e0 = e;
    }
    do {
      v = e->end;
      if (!seen[v]) {
	seen[v] = 1;
	verts[nv++] = queue[head++] = v;
      }
      e = e->next;
    } while (e != e0);
  }
  verts[nv] = 0;

  free(seen);
  free(queue);
  return verts;
}


int *
vertex_depths_breadth_first(GRAPH *G, EDGE *start, int at_face)
{
  EDGE **edges;
  int *depth;
  int i;

  edges = spanning_edges_breadth_first(G, start, at_face);
  if (edges == NULL)
    return NULL;

  depth = (int *)calloc(G->size+1, sizeof(int));
  if (depth == NULL) {
    status = NO_MEMORY;
    free(edges);
    return NULL;
  }

  for (i = 0; edges[i]; i++)
    depth[edges[i]->end] = depth[edges[i]->start] + 1;

  free(edges);
  return depth;
}


int
maxdepth_breadth_first(GRAPH *G, EDGE *start, int at_face)
{
  int *verts, *depth;
  int i, d;

  verts = vertices_breadth_first(G, start, at_face, 1);
  depth = vertex_depths_breadth_first(G, start, at_face);
  if (verts == NULL || depth == NULL)
    return -1;

  for (i = 0; verts[i]; ++i)
    ;
  d = depth[verts[i-1]];

  free(verts);
  free(depth);
  return d;
}


double
outer_curvature(GRAPH *G, EDGE *e0)
{
  EDGE *e;
  double sum, m;

  sum = 0.0;
  e = e0;
  do {
    m = degree_of_vertex(e);
    if (m == 2)
      sum += 1.0 / 6.0;
    else
      sum += 1.0 / (double)m - 1.0 / 2.0;
    e = e->inverse->prev;
  } while (e != e0);

  return sum;
}


/* -------------------------------------------------------------------- */

int
bfs_renumber_graph(GRAPH *G, EDGE *start, int at_face)
{
  int *verts, *old2new;
  EDGE **map;
  EDGE *e0, *e;
  int n, i;

  n = G->size;

  verts = vertices_breadth_first(G, start, at_face, 1);
  if (verts == NULL)
    return 0;

  old2new = (int *)calloc(n+1, sizeof(int));
  map = (EDGE **)calloc(n+1, sizeof(EDGE *));
  if (old2new == NULL || map == NULL) {
    status = NO_MEMORY;
    free(old2new);
    free(map);
    return 0;
  }

  for (i = 1; i <= n; ++i)
    old2new[verts[i-1]] = i;

  for (i = 1; i <=n; i++) {
    map[i] = G->map[verts[i-1]];
    e0 = e = map[i];
    do {
      e->start = old2new[e->start];
      e->end = old2new[e->end];
      e = e->next;
    } while (e != e0);
  }
  map[1] = start;
  free(G->map);
  G->map = map;

  free(verts);
  free(old2new);
  return 1;
}


EDGE *
new_edge_pair(int start, int end, int n1, int n2,
	      EDGE *prev_at_start, EDGE *prev_at_end)
{
  EDGE *a, *b;

  a = (EDGE *)malloc(sizeof(EDGE));
  b = (EDGE *)malloc(sizeof(EDGE));
  if (a == NULL || b == NULL) {
    status = NO_MEMORY;
    free(a);
    free(b);
    return NULL;
  }

  a->number = n1;
  b->number = n2;
  a->start = b->end = start;
  a->end = b->start = end;
  a->inverse = b;
  b->inverse = a;

  if (prev_at_start) {
    a->prev = prev_at_start;
    a->next = a->prev->next;
    a->next->prev = a;
    a->prev->next = a;
  }
  else
    a->prev = a->next = a;

  if (prev_at_end) {
    b->prev = prev_at_end;
    b->next = b->prev->next;
    b->next->prev = b;
    b->prev->next = b;
  }
  else
    b->prev = b->next = b;

  return a;
}


int
split_edges(GRAPH *G, int *forbidden)
{
  EDGE **ereps;
  EDGE *e, *ep, *k;
  int edge_nr, ne, nv, new_size, i, u, v;

  nv = G->size;
  edge_nr = largest_edge_number(G);

  ereps = edge_reps(G);
  if (ereps == NULL)
    return 0;
  for (ne = 0; ereps[ne]; ne++)
    ;

  new_size = nv + ne;

  if (!resize_graph(G, new_size)) {
    free(ereps);
    return 0;
  }

  for (i = 0, v = nv; i < ne; i++) {
    e = ereps[i];
    if (forbidden && forbidden[e->number])
      continue;

    if (e == e->prev)
      ep = NULL;
    else
      ep = e->prev;
    u = e->start;
    ++v;

    e->start = e->inverse->end = v;
    G->map[v] = e;
    e->prev->next = e->next;
    e->next->prev = e->prev;
    e->prev = e->next = e;

    k = new_edge_pair(u, v, edge_nr+1, edge_nr+2, ep, e);
    edge_nr += 2;
    if (k == NULL) {
      free(ereps);
      return 0;
    }
    if (G->map[u] == e)
      G->map[u] = k;
  }

  if (!resize_graph(G, v)) {
    free(ereps);
    return 0;
  }
  free(ereps);
  return 1;
}


int
triangulate(GRAPH *G, int *forbidden)
{
  EDGE **freps;
  EDGE *e, *k;
  int ne, nv, nf, new_size, i, j, deg, v;

  nv = G->size;
  ne = largest_edge_number(G);

  freps = face_reps(G);
  if (freps == NULL)
    return 0;
  for (nf = 0; freps[nf]; nf++)
    ;

  new_size = nv + nf;

  if (!resize_graph(G, new_size)) {
    free(freps);
    return 0;
  }

  for (i = 0, v = nv; i < nf; i++) {
    e = freps[i];
    if (forbidden && forbidden[e->number])
      continue;

    deg = degree_of_face(e);
    ++v;
    k = NULL;

    for (j = 0; j < deg; j++) {
      k = new_edge_pair(v, e->start, ne+1, ne+2, k, e);
      ne += 2;
      if (k == NULL) {
	free(freps);
	return 0;
      }
      e = e->inverse->prev;
    }
    G->map[v] = k;
  }

  if (!resize_graph(G, v)) {
    free(freps);
    return 0;
  }
  free(freps);
  return 1;
}


/* -------------------------------------------------------------------- */

void
copy_position(POSITIONING *Q, int w, POSITIONING *P, int v)
{
  int dim, i;

  dim = min(P->dim, Q->dim);

  for (i = 0; i < dim; i++)
    PICK(Q, w, i) = PICK(P, v, i);
}


double
squared_dist_positions(POSITIONING *P, int v, POSITIONING *Q, int w)
{
  double d = 0.0, x;
  int dim, i;

  dim = min(P->dim, Q->dim);

  for (i = 0; i < dim; i++) {
    x = PICK(Q,w,i) -  PICK(P,v,i);
    d += x * x;
  }
  return d;
}


double
squared_norm_position(POSITIONING *P, int v)
{
  double d, x;
  int i;

  d = 0.0;
  for (i = 0; i < P->dim; i++) {
    x = PICK(P,v,i);
    d += x * x;
  }
  return d;
}


void
limit_dist_positions(POSITIONING *P, int v, POSITIONING *Q, int w, double t)
{
  double d, f;
  int dim, i;

  d = squared_dist_positions(P, v, Q, w);
  if (d > t) {
    f = 1.0 - t / d;
    dim = min(P->dim, Q->dim);
    for (i = 0; i <= dim; i++)
      PICK(P,v,i) += (PICK(Q,v,i) - PICK(P,v,i)) * f;
  }
}


void
clear_positioning(POSITIONING *P)
{
  int i, v;

  for (v = 1; v <= P->size; v++)
    for (i = 0; i < P->dim; i++)
      PICK(P,v,i) = 0.0;
}


void
copy_positioning(POSITIONING *Q, POSITIONING *P)
{
  int dim, size, i, v;

  dim = min(P->dim, Q->dim);
  size = min(P->size, Q->size);

  for (v = 1; v <= size; v++)
    for (i = 0; i < dim; i++)
      PICK(Q,v,i) = PICK(P,v,i);
}


void
scale_positioning(POSITIONING *P, double f)
{
  int i, v;
  for (v = 1; v <= P->size; v++)
    for (i = 0; i < P->dim; i++)
      PICK(P,v,i) *= f;
}


void
xz_swap_positioning(POSITIONING *P)
{
  int v;
  double t;
  if (P->dim < 3)
    return;
  for (v = 1; v <= P->size; v++) {
    t = PICK(P,v,0);
    PICK(P,v,0) = PICK(P,v,2);
    PICK(P,v,2) = t;
  }
}


void
shift_positioning(POSITIONING *P, double *t, double f)
{
  int i, v;
  for (v = 1; v <= P->size; v++)
    for (i = 0; i < P->dim; i++)
      PICK(P,v,i) += f * t[i];
}


double *
center_of_positioning(POSITIONING *P)
{
  int dim, size, i, v;
  double *c;

  dim = P->dim;
  size = P->size;

  c = (double *)calloc(dim, sizeof(double));
  if (c == NULL) {
    status = NO_MEMORY;
    return NULL;
  }

  for (v = 1; v <= size; v++)
    for (i = 0; i < dim; i++)
      c[i] += PICK(P,v,i);

  for (i = 0; i < dim; i++)
    c[i] /= size;

  return c;
}


int
recenter_positioning(POSITIONING *P)
{
  double *c = center_of_positioning(P);
  if (c) {
    shift_positioning(P, c, -1.0);
    free(c);
    return 1;
  }
  else
    return 0;
}


double
squared_dist_positionings(POSITIONING *P, POSITIONING *Q)
{
  double d, x;
  int dim, size, i, v;

  dim = min(P->dim, Q->dim);
  size = min(P->size, Q->size);

  d = 0.0;
  for (v = 1; v <= size; v++) {
    for (i = 0; i < dim; i++) {
      x = PICK(Q,v,i) -  PICK(P,v,i);
      d += x * x;
    }
  }

  return d;
}


double
average_edge_length(GRAPH *G, POSITIONING *P)
{
  int v, w, ne;
  EDGE *e0, *e;
  double sum;

  sum = 0.0;
  ne = 0;

  for (v = 1; v <= G->size; v++) {
    e = e0 = G->map[v];
    if (e == NULL)
      continue;
    do {
      w = e->end;
      if (w > v) {
	++ne;
	sum += sqrt(squared_dist_positions(P,v,P,w));
      }
      e = e->next;
    } while (e != e0);
  }

  return sum / (double)ne;
}


/* --- EOF graph.c --- */
/* --------------------------------------------------------------------	*
 *	embed.c  				09-aug-2001  by ODF	*
 * --------------------------------------------------------------------	*/

/* CVS: $Id: embed.c,v 1.7 2001/08/16 04:53:16 delgado Exp $ */

/*

   To do list:

   -- 3d: Better init routine for helix mode.

   --     Improve decision procedure of when to use helix mode.

   -- 2d: Allow drawing with one vertex 'at infinity'. Optionally, do
      this automatically if the largest order symmetry centers are all
      at vertices or edges.

*/


#ifndef _h_graph
#include "graph.h"
#endif

#define EPS 1e-15
#define PI 3.14159265358979323846

/* --- */

#define DECLARE_PLACER(name) \
void \
name(GRAPH *G, POSITIONING *P_new, int v, \
     POSITIONING *P_old, int *v_flags, int *e_flags)

typedef DECLARE_PLACER(PLACER);

extern PLACER tutte;
extern PLACER equal_lengths;
extern PLACER equal_lengths_on_sphere;
extern PLACER equal_area;
extern PLACER central_3d;
extern PLACER local_3d;
extern PLACER local_2d;

typedef double TEMPERATURE_FUNCTION(int, int);


/* --- Functions which might later migrate to graph.c ----------------- */

int
has_small_faces(GRAPH *G)
{
  EDGE **freps;
  int i;

  freps = face_reps(G);
  if (freps) {
    for (i = 0; freps[i]; ++i)
      if (degree_of_face(freps[i]) <= 3)
	return 1;
    free(freps);
  }

  return 0;
}


double
radius_of_positioning(POSITIONING *P)
{
  int v;
  double r, x;

  r = 0.0;
  for (v = 1; v <= P->size; ++v) {
    x = squared_norm_position(P, v);
    if (x > r)
      r = x;
  }
  return sqrt(r);
}


int
add_sticks(GRAPH *G)
{
  int ne, v, i, count;
  EDGE *e, *k;
  EDGE **q;

  count = 0;
  for (v = 1; v <= G->size; v++) {
    e = G->map[v];
    if (degree_of_vertex(e) < 3)
      count++;
  }
  if (count < 3)
    return 0;

  q = (EDGE **) calloc(G->size + 1, sizeof(EDGE *));
  if (q == NULL) {
    status = NO_MEMORY;
    return 0;
  }

  for (v = 1, i = 0; v <= G->size; v++) {
    e = G->map[v];
    if (degree_of_vertex(e) == 2) {
      if (degree_of_face(e->next) > degree_of_face(e))
	q[i++] = e->next;
      else
	q[i++] = e;
    }
  }
  q[i] = NULL;

  v = G->size;
  if (!resize_graph(G, v + i)) {
    free(q);
    return 0;
  }

  ne = largest_edge_number(G);

  for (i = 0; q[i]; ++i) {
    e = q[i];
    ++v;
    k = new_edge_pair(e->start, v, ne+1, ne+2, e, NULL);
    ne += 2;
    if (k == NULL) {
      free(q);
      return 0;
    }
    G->map[v] = k->inverse;
  }

  free(q);
  return count;
}


int
close_gaps(GRAPH *G, int min_gap, EDGE **f_outer, int *max_gap)
{
  int count, gap_size, move, ne, v, v0, w;
  EDGE *e, *k, *outer;
  int *seen;

  seen = (int *) calloc(G->size+1, sizeof(int));
  if (seen == NULL) {
    status = NO_MEMORY;
    return 0;
  }

  if (f_outer)
    outer = *f_outer;
  else
    outer = NULL;
  move = count = 0;

  ne = largest_edge_number(G);

  for (v = 1; v <= G->size; v++) {
    if (seen[v])
      continue;
    e = G->map[v];
    if (degree_of_vertex(e) == 1) {
      v0 = w = v;
      gap_size = 0;
      do {
	e = e->inverse->prev;
	++gap_size;
	if (e == outer)
	  move = 1;
	v = e->start;
	seen[v] = 1;

	if (degree_of_vertex(e) == 1 || v == v0) {
	  if (max_gap && gap_size > *max_gap)
	    *max_gap = gap_size;
	  if (gap_size >= min_gap && v != w) {
	    k = new_edge_pair(v, w, ne+1, ne+2, e, G->map[w]);
	    ne += 2;
	    if (k == NULL) {
	      free(seen);
	      return 0;
	    }
	    if (move) {
	      outer = k->inverse;
	      move = 0;
	    }
	    ++count;
	  }
	  w = v;
	  gap_size = 0;
	  move = 0;
	}
      } while (v != v0);
    }
  }

  if (f_outer)
    *f_outer = outer;

  free(seen);
  return count;
}


EDGE *
normalize_graph(GRAPH *G, EDGE *outer, int *max_gap)
{
  EDGE *e;
  int count;
  int old_deg, new_deg;

  if (max_gap)
    *max_gap = 0;

  e = outer;
  old_deg = degree_of_face(e);

  while (1) {
    count = add_sticks(G);                 if (status) return NULL;
    if (count < 3)
      break;
    count = close_gaps(G, 5, &e, max_gap); if (status) return NULL;
    if (count <= 1)
      break;
    new_deg = degree_of_face(e);
    if (new_deg == old_deg)
      break;
    old_deg = new_deg;
  }

  count = add_sticks(G);        if (status) return NULL;
  if (count >= 3)
    close_gaps(G, 3, &e, NULL); if (status) return NULL;
  count = add_sticks(G);        if (status) return NULL;
  if (count >= 3)
    close_gaps(G, 2, &e, NULL); if (status) return NULL;

  return e;
}


int *
classify_edges(GRAPH *G, GRAPH *G_old)
{
  int *flags;
  int v, count_new;
  EDGE *e0, *e, *last_new;

  flags = make_edge_flags(G, 0);
  if (flags == NULL)
    return NULL;
  last_new = NULL;

  for (v = 1; v <= G->size; v++) {
    e = e0 = G->map[v];
    count_new = 0;
    if (e == NULL)
      continue;
    do {
      if (find_edge(G_old, e->start, e->end))
	flags[e->number] = 1;
      else {
	++count_new;
	last_new = e;
      }
      e = e->next;
    } while (e != e0);
    if (count_new == 1) {
      flags[last_new->number] = 2;
      flags[last_new->inverse->number] = 2;
    }
  }

  return flags;
}


int
lift_vertices(GRAPH *G, POSITIONING *P, EDGE *e, double f)
{
  int *depth;
  int i;

  depth = vertex_depths_breadth_first(G, e, 1);
  if (depth == NULL)
    return 0;
  for (i = 1; i <= P->size; ++i)
    PICK(P, i, 2) = f * depth[i];
  free(depth);

  return 1;
}


/* -------------------------------------------------------------------- */

EDGE *
best_outer_face(GRAPH *G)
{
  EDGE **freps;
  EDGE *res;
  int i, best_i, better;
  int degr, depth, symm, best_degr, best_depth, best_symm;

  freps = face_reps(G);
  if (freps == NULL)
    return NULL;

  best_i = 0;
  best_degr = degree_of_face(freps[0]);
  best_depth = maxdepth_breadth_first(G, freps[0], 1);
  best_symm = symmetry_at_face(G, freps[0]);

  for (i = 1; freps[i]; ++i) {
    degr = degree_of_face(freps[i]);
    symm = symmetry_at_face(G, freps[i]);
    depth = maxdepth_breadth_first(G, freps[i], 1);
    if (degr == best_degr) {
      if (symm == best_symm)
	better = (depth < best_depth);
      else
	better = (symm > best_symm);
    }
    else
      better = (degr > best_degr);

    if (better) {
      best_i = i;
      best_degr = degr;
      best_depth = depth;
      best_symm = symm;
    }
  }

  res = freps[best_i];
  free(freps);
  return res;
}


EDGE *
best_outer_face_tubular(GRAPH *G)
{
  EDGE **freps;
  EDGE *res;
  int i, best_i, better;
  int degr, depth, symm, best_degr, best_depth, best_symm;

  freps = face_reps(G);
  if (freps == NULL)
    return NULL;

  best_i = 0;
  best_degr = degree_of_face(freps[0]);
  best_depth = maxdepth_breadth_first(G, freps[0], 1);
  best_symm = symmetry_at_face(G, freps[0]);

  for (i = 1; freps[i]; ++i) {
    depth = maxdepth_breadth_first(G, freps[i], 1);
    degr = degree_of_face(freps[i]);
    symm = symmetry_at_face(G, freps[i]);
    if (depth == best_depth) {
      if (degr == best_degr)
	better = (symm > best_symm);
      else
	better = (degr > best_degr);
    }
    else
      better = (depth > best_depth);

    if (better) {
      best_i = i;
      best_degr = degr;
      best_depth = depth;
      best_symm = symm;
    }
  }

  res = freps[best_i];
  free(freps);
  return res;
}


int
init_positions(GRAPH *G, POSITIONING *P, EDGE *start, int at_face,
	       int dim, int tubular, int winding)
{
  EDGE *e0, *e;
  int *v_list, *depth;
  int d, maxd;
  int i, j, k, n, m, v;
  double arg, f, psi, radius, rot, z;

  v_list = vertices_breadth_first(G, start, at_face, 1);
  depth = vertex_depths_breadth_first(G, start, at_face);
  if (v_list == NULL || depth == NULL) {
    free(v_list);
    free(depth);
    return 0;
  }

  n = G->size;
  maxd = depth[v_list[n-1]];
  if (tubular)
    f = 1.0 / sqrt((double)(G->size));
  else
    f = 1.0;
  rot = z = 0.0;

  i = j = 0;
  while (v_list[j]) {
    d = depth[v_list[j]];

    k = 0;
    if (j > 0) {
      --k;
      e0 = e = G->map[v_list[i]];
      do {
	if (depth[e->end] == d)
	  ++k;
	e = e->next;
      } while (e != e0);
    }

    i = j;
    while (depth[v_list[j]] == d)
      ++j;
    m = j - i;

    if (m > 0)
      arg = 2 * PI / m;
    else
      arg = 0.0;
    rot -= arg * k / 2;

    if (dim < 3)
      radius = 1 - (double)(d+1) / (maxd+1);
    else if (tubular) {
      if (arg)
	radius = 1.0 / arg;
      else
	radius = 0.0;
      z = 0.707 * (d - maxd/2.0);
    }
    else {
      psi = PI * ((double)d/(double)maxd - 1.0);
      z = cos(psi);
      radius = sin(psi);
    }

    for (k = 0; k < m; k++) {
      v = v_list[k+i];
      PICK(P, v, 0) = f * radius * cos(winding * (arg * k + rot));
      PICK(P, v, 1) = f * radius * sin(winding * (arg * k + rot));
      if (dim > 2)
	PICK(P, v, 2) = f * z;
    }
  }

  free(v_list);
  free(depth);
  return 1;
}


/* -------------------------------------------------------------------- */

DECLARE_PLACER(tutte)
{
  double sum, x, y, d;
  EDGE *e0, *e1;
  int w;

  sum = d = 0.1;
  x = d * PICK(P_old, v, 0);
  y = d * PICK(P_old, v, 1);
  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;
  do {
    e1 = e1->next;
    w = e1->end;
    d = 1.0;
    sum += d;
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);
  } while (e1 != e0);
  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum;
    PICK(P_new, v, 1) = y/sum;
  }
}


DECLARE_PLACER(equal_lengths)
{
  double sum, x, y, d;
  EDGE *e0, *e1;
  int w;

  sum = d = 0.1;
  x = d * PICK(P_old, v, 0);
  y = d * PICK(P_old, v, 1);
  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;
  do {
    e1 = e1->next;
    w = e1->end;
    d = sqrt(squared_dist_positions(P_old, v, P_old, w));
    sum += d;
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);
  } while (e1 != e0);
  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum;
    PICK(P_new, v, 1) = y/sum;
  }
}


DECLARE_PLACER(equal_lengths_on_sphere)
{
  double x, y, z, d, r;
  EDGE *e0, *e1;
  int w;

  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;

  x = y = z = 0.0;

  do {
    e1 = e1->next;
    w = e1->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);
    z += d * PICK(P_old, w, 2);
  } while (e1 != e0);

  r = sqrt(x * x + y * y + z * z);
  if (r < EPS)
    r = 1.0;

  PICK(P_new, v, 0) = x/r;
  PICK(P_new, v, 1) = y/r;
  PICK(P_new, v, 2) = z/r;
}


DECLARE_PLACER(equal_area)
{
  double sum, x, y, dx1, dy1, dx2, dy2, d;
  EDGE *e0, *e1;
  int w1, w2;

  sum = x = y = 0.0;
  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;
  w2 = e1->end;
  dx2 = PICK(P_old, w2, 0) - PICK(P_old, v, 0);
  dy2 = PICK(P_old, w2, 1) - PICK(P_old, v, 1);
  do {
    e1 = e1->next;
    w1 = w2;
    dx1 = dx2;
    dy1 = dy2;
    w2 = e1->end;
    dx2 = PICK(P_old, w2, 0) - PICK(P_old, v, 0);
    dy2 = PICK(P_old, w2, 1) - PICK(P_old, v, 1);
    d = dx1 * dy2 - dx2 * dy1;
    d *= d;
    d *= d;
    sum += d;
    x += d * (PICK(P_old, w1, 0) + PICK(P_old, w2, 0));
    y += d * (PICK(P_old, w1, 1) + PICK(P_old, w2, 1));
  } while (e1 != e0);
  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum - PICK(P_old, v, 0);
    PICK(P_new, v, 1) = y/sum - PICK(P_old, v, 1);
  }
}


DECLARE_PLACER(central_3d)
{
  double x, y, z, d, sum;
  EDGE *e0, *e1;
  int w;

  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;

  sum = x = y = z = 0.0;

  do {
    e1 = e1->next;
    w = e1->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    sum += d;
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);
    z += d * PICK(P_old, w, 2);
  } while (e1 != e0);

  d = squared_norm_position(P_old, v);
  if (d > EPS) {
    d = 1.0 / d;
    sum += d;
    x += d * PICK(P_old, v, 0);
    y += d * PICK(P_old, v, 1);
    z += d * PICK(P_old, v, 2);
  }

  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum;
    PICK(P_new, v, 1) = y/sum;
    PICK(P_new, v, 2) = z/sum;
  }
}


DECLARE_PLACER(local_3d)
{
  double x, y, z, d, sum;
  EDGE *e0, *e1;
  int w;

  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;

  sum = x = y = z = 0.0;

  do {
    w = e1->end;
    d = 1.0;
    sum += d;
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);
    z += d * PICK(P_old, w, 2);

    w = e1->inverse->prev->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    if (d > EPS) {
      d = 0.5 / d;
      sum -= d;
      x -= d * PICK(P_old, w, 0);
      y -= d * PICK(P_old, w, 1);
      z -= d * PICK(P_old, w, 2);
    }

    w = e1->inverse->next->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    if (d > EPS) {
      d = 0.5 / d;
      sum -= d;
      x -= d * PICK(P_old, w, 0);
      y -= d * PICK(P_old, w, 1);
      z -= d * PICK(P_old, w, 2);
    }

    e1 = e1->next;
  } while (e1 != e0);

  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum;
    PICK(P_new, v, 1) = y/sum;
    PICK(P_new, v, 2) = z/sum;
  }
}


DECLARE_PLACER(local_2d)
{
  double x, y, d, sum;
  EDGE *e0, *e1;
  int w;

  e0 = e1 = G->map[v];
  if (e0 == NULL)
    return;

  sum = x = y = 0.0;

  do {
    w = e1->end;
    d = 1.0;
    sum += d;
    x += d * PICK(P_old, w, 0);
    y += d * PICK(P_old, w, 1);

    w = e1->inverse->prev->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    if (d > EPS) {
      d = 0.5 / d;
      sum -= d;
      x -= d * PICK(P_old, w, 0);
      y -= d * PICK(P_old, w, 1);
    }

    w = e1->inverse->next->end;
    d = squared_dist_positions(P_old, v, P_old, w);
    if (d > EPS) {
      d = 0.5 / d;
      sum -= d;
      x -= d * PICK(P_old, w, 0);
      y -= d * PICK(P_old, w, 1);
    }

    e1 = e1->next;
  } while (e1 != e0);

  if (sum > EPS) {
    PICK(P_new, v, 0) = x/sum;
    PICK(P_new, v, 1) = y/sum;
  }
}


/* -------------------------------------------------------------------- */

int
iterate_positions(GRAPH *G, POSITIONING *P, int *v_flags, int *e_flags,
		  PLACER *place, int *v_list,
		  TEMPERATURE_FUNCTION *get_temperature,
		  int steps, int in_place, double limit)
{
  POSITIONING *Q, *S;
  int iteration;
  int i, v;
  double t;

  Q = new_positioning(P->size, P->dim);
  if (Q == NULL) {
    status = NO_MEMORY;
    return -1;
  }

  if (in_place)
    S = P;
  else
    S = Q;

  for (iteration = 0; iteration < steps; ++iteration) {
    t = get_temperature(iteration, steps);
    copy_positioning(Q, P);

    for (i = 0; v_list[i]; ++i) {
      v = v_list[i];
      if (v <= G->size) {
	place(G, P, v, S, v_flags, e_flags);
	limit_dist_positions(P, v, Q, v, t);
      }
    }

    if (limit > 0.0 && sqrt(squared_dist_positionings(P, Q)) < limit)
	break;
  }

  free_positioning(Q);
  return iteration;
}


double
fast(int step, int maxstep)
{
  double t;
  t = 1.0 - (double)step/(double)maxstep;
  return 0.1 * t;
}


double
slow(int step, int maxstep)
{
  double t;
  t = 1.0 - (double)step/(double)maxstep;
  return 0.04 * t * t * t;
}


/* -------------------------------------------------------------------- */


void
usage(void)
{
  static char *text[] = {
    "usage: embed [options] <input >output",
    "",
    "the input must be in writegraph format as used in the VEGA project",
    "recognized options:",
    "  -a [+-]     if '+' and there are vertices of degree 2, then",
    "              subdivide some faces to make these stick out.",
    "              (This process is repeated recursively).",
    "  -b n,m      boundary face is clockwise of directed edge n->m",
    "  -c x,y      boundary face contains point x,y",
    "  -d n        dimension of the embedding, either 2 or 3",
    "  -i c        type of initial (non-iterative) embedding,",
    "              where c is one of",
    "              k     keep given embedding",
    "              p     planar",
    "              s     spherical",
    "              t     tubular",
    "  -f x,y,z    multiply default number of iteration steps",
    "              in phases 1,2,3 by factors x,y,z, respectively",
    "  -p c        for dimension 2 only: use force model c in",
    "              second phase, where c is one of",
    "              a     triangle areas",
    "              l     edge lengths",
    "              t     Tutte's method with uniform weights",
    "  -r          renumber vertices on output",
    "  -s [+-]     if '+', work on a triangulation of the graph",
    "  -v          verbose mode",
    "  -w c        write graph in format c, where c is one of",
    "              b     Brookhaven protein data base (3D only)",
    "              n     no output at all",
    "              p     planar code (binary, no coordinates)",
    "              v     writegraph format",
    "  -x          helix mode",
    "",
    "default is '-a+ -d2 -pa -s+ -wv',",
    "with force model other than 'a', the option '-s-' is recommended.",
    "",
    NULL
  };

  int i;

  for (i = 0; text[i]; i++) {
    fprintf(stderr, text[i]);
    fprintf(stderr, "\n");
  }
}


#include <unistd.h>
#include <getopt.h>

void
print_error(char *where, int code) {
  char *code_as_text;

  fprintf(stderr, "Error at \"%s\"", where);
  if (status) {
    switch(status) {
    case BAD_ARGS: code_as_text = "BAD_ARGS"; break;
    case NO_MEMORY: code_as_text = "NO_MEMORY"; break;
    case BAD_INPUT: code_as_text = "BAD_INPUT"; break;
    case VERTEX_IN_USE: code_as_text = "VERTEX_IN_USE"; break;
    case GRAPH_TOO_LARGE: code_as_text = "GRAPH_TOO_LARGE"; break;
    case SYSTEM_ERROR: code_as_text = "SYSTEM_ERROR"; break;
    default: code_as_text = NULL;
    }
    if (code_as_text)
      fprintf(stderr, ", code = %s", code_as_text);
    else
      fprintf(stderr, ", code = %d", code);
  }
  fprintf(stderr, "\n");
}

#define CHECK(expr) \
  if (!(expr)) { \
    print_error(#expr, status); \
    return 1; \
  }

#define ITERATE(G, P, vfl, efl, plc, vls, tpf, fac, stp) { \
  n = iterate_positions((G), (P), (vfl), (efl), (plc), (vls), (tpf), \
			(int)((fac)*(double)(stp)+0.5), 1, 1.0e-4); \
  if (n < 0) { \
    fprintf(stderr, "Error %d in iterate_positions\n", status); \
    return 1; \
  } \
  else if (verbose)  { \
    fprintf(stderr, "used %d iterations\n", n); \
    fprintf(stderr, "average edge length = %f\n", \
	    average_edge_length(G, P)); \
  } \
}

int
main(int argc, char *argv[])
{
  GRAPH *G_in, *G_aug, *G_sub, *G_out;
  EDGE *f_in, *f_aug, *f_sub;
  POSITIONING *P;
  int *forbidden;
  int *v_list;
  int *v_list_sub_0_1;
  int *v_list_sub_1_0;
  int *v_list_aug_1_1;
  int *v_list_in_1_1;
  int c, max_gap, n, steps;

  int     augment            = 1;
  int     dimension          = 2;
  int     end                = 0;
  double  factor1            = 1.0;
  double  factor2            = 1.0;
  double  factor3            = 1.0;
  int     helix_mode         = 0;
  int     helix_winding      = 1;
  char    init_mode          = 0;
  int     output_augmented   = 0;
  char    output_format      = 'v';
  int     output_subdivision = 0;
  int     override_factors   = 0;
  PLACER *placer             = NULL;
  int     renumber           = 0;
  int     start              = 0;
  int     subdivide          = 1;
  int     verbose            = 0;

  double  c_x, c_y;
  int     contained_option   = 0;

  /* --- Parse the command line --- */

  while ((c = getopt(argc, argv, "ASa:b:c:d:f:hi:p:rs:tvw:x:z")) != EOF) {
    switch (c) {
    case 'A':
      output_augmented = 1;
      break;
    case 'S':
      output_subdivision = 1;
      break;
    case 'a':
      switch(optarg[0]) {
      case '+':
	augment = 1;
	break;
      case '-':
	augment = 0;
	break;
      default:
	usage();
	return 1;
      }
      break;
    case 'b':
      if (sscanf(optarg, "%d,%d", &start, &end) == 2) {
        contained_option = 0;
      }
      break;
    case 'c':
      if (sscanf (optarg, "%lf,%lf", &c_x, &c_y) == 2) {
        contained_option = 1;
      }
      break;
    case 'd':
      dimension = atoi(optarg);
      if (dimension != 2 && dimension != 3) {
	usage();
	return 1;
      }
      break;
    case 'f':
      sscanf(optarg, "%lf,%lf,%lf", &factor1, &factor2, &factor3);
      override_factors = 1;
      break;
    case 'h':
      usage();
      return 0;
    case 'i':
      init_mode = optarg[0];
      switch (init_mode) {
      case 'k': /* keep original */
      case 'p':	/* planar */
      case 's':	/* spherical */
      case 't':	/* tubular */
	break;
      default:
	usage();
	return 1;
      }
      break;
    case 'p':
      switch (optarg[0]) {
      case 'a':
	placer = equal_area;
	break;
      case 't':
	placer = tutte;
	break;
      case 'l':
	placer = equal_lengths;
	break;
      default:
	usage();
	return 1;
      }
      break;
    case 'r':
      renumber = 1;
      break;
    case 's':
      switch(optarg[0]) {
      case '+':
	subdivide = 1;
	break;
      case '-':
	subdivide = 0;
	break;
      default:
	usage();
	return 1;
      }
      break;
    case 'v':
      verbose = 1;
      break;
    case 'w':
      output_format = optarg[0];
      switch(output_format) {
      case 'b': /* Brookhaven PDB */
      case 'n': /* no output */
      case 'p':	/* planar code */
      case 'v': /* Vega format */
	break;
      default:
	usage();
	return 1;
      }
      break;
    case 'x':
      helix_mode = 1;
      helix_winding = atoi(optarg);
      break;
    case 'z':
      fprintf(stdout,"%d\n",getpid());  fflush(stdout);
      break;
    default:
      usage();
      return 1;
    }
  }

  /* --- Read the input graph and positioning --- */

  CHECK(G_in = new_graph(0));
  CHECK(P = new_positioning(0,0));
  CHECK(readgraph_vega(stdin, G_in, P));

  /* --- Determine an outer face for the embedding --- */

  if (contained_option) {
    if (! find_edge_from_contained_point(G_in, P, c_x, c_y, &start, &end)) {
      CHECK(! write_result(G_in, P, stdout, output_format));
      free_graph(G_in);
      free_positioning(P);
      return 0;
    }
  }
  if (start && end) {
    if (!(f_in = find_edge(G_in, start, end))) {
      fprintf(stderr, "Start edge %d->%d does not exist\n", start, end);
      return 1;
    }
  }
  else if (init_mode == 't') {
    CHECK(f_in = best_outer_face_tubular(G_in));
  }
  else {
    CHECK(f_in = best_outer_face(G_in));
  }

  /* --- Determine an init mode if none was requested --- */

  if (!init_mode) {
    if (dimension == 3 && outer_curvature(G_in, f_in) <= 0.0)
      init_mode = 's';
    else
      init_mode = 'p';
  }

  /* --- Renumber the graph if necessary --- */

  if (renumber) {
    CHECK(bfs_renumber_graph(G_in, f_in, 1));
    f_in = find_edge(G_in, 1, 2);
  }

  /* --- Make a copy with a more convex outer face --- */

  CHECK(G_aug = copy_of_graph(G_in));
  f_aug = find_edge(G_aug, f_in->start, f_in->end);
  if (augment) {
    CHECK(f_aug = normalize_graph(G_aug, f_aug, &max_gap));
  }

  /* --- Switch on helix mode if necessary --- */

  if (dimension > 2 && max_gap >= 6)
    helix_mode = 1;

  /* --- Make some adjustments for helix mode --- */

  if (helix_mode) {
    init_mode = 'p';
    CHECK(f_in = best_outer_face_tubular(G_in));
  }

  /* --- Make a working copy of the graph --- */

  CHECK(G_sub = copy_of_graph(G_aug));
  f_sub = find_edge(G_sub, f_aug->start, f_aug->end);

  /* --- Triangulate the graph if necessary --- */

  if (subdivide) {
    if (dimension == 3 && has_small_faces(G_aug))
      split_edges(G_sub, NULL);

    if (init_mode == 'p') {
      CHECK(forbidden = make_edge_flags(G_sub, 0));
      mark_face(f_sub, forbidden, 1);
    }
    else
      forbidden = NULL;

    CHECK(triangulate(G_sub, forbidden));
    free(forbidden);

    if (init_mode != 'p')
      f_sub = f_sub->next->inverse;
  }

  /* --- Phase 0: determine an initial positioning --- */

  CHECK(resize_positioning(P, G_sub->size, dimension));

  if (init_mode != 'k') {
    int at_face, dim, tubular, winding;

    clear_positioning(P);

    if (init_mode == 'p') {
      at_face = 1;
      dim = 2;
      tubular = 0;
      if (dimension == 3 && helix_mode)
	winding = helix_winding;
      else
	winding = 1;
    }
    else {
      at_face = 0;
      dim = 3;
      winding = 1;
      tubular = (init_mode == 't');
    }

    CHECK(init_positions(G_sub, P, f_sub, at_face, dim, tubular, winding));
  }

  CHECK(recenter_positioning(P));
  scale_positioning(P, 1.0 / radius_of_positioning(P));

  /* --- Set the default number of iteration steps --- */
  
  if (G_in->size < 200) {
    if (G_in->size < 40)
      steps = 500;
    else
      steps = 20000 / G_in->size;
  }
  else
    steps = G_in->size / 2;

  /* --- Adjust phase specific factors for numbers of steps --- */

  if (init_mode == 't' && !override_factors)
    factor1 = 0.0;
  if (dimension == 2 && !override_factors)
    factor3 = 0.0;

  /* --- Make all vertex lists which might be needed --- */

  CHECK(v_list_sub_1_0 = vertices_breadth_first(G_sub, f_sub, 1, 0));
  CHECK(v_list_sub_0_1 = vertices_breadth_first(G_sub, f_sub, 0, 1));
  CHECK(v_list_in_1_1 = vertices_breadth_first(G_in, f_in, 1, 1));
  CHECK(v_list_aug_1_1 = vertices_breadth_first(G_aug, f_aug, 1, 1));

  /* --- Phase 1: Modified Tutte placement --- */

  if (init_mode == 'p') {
    ITERATE(G_sub, P, NULL, NULL, equal_lengths,
	    v_list_sub_1_0, fast, factor1/4, steps);
  }
  else {
    ITERATE(G_sub, P, NULL, NULL, equal_lengths_on_sphere,
	    v_list_sub_0_1, fast, factor1/2, steps);
  }

  if (dimension == 3 && init_mode == 'p' && !helix_mode) {
    CHECK(lift_vertices(G_sub, P, f_sub, 0.01));
  }

  /* --- Phase 2  --- */

  if (helix_mode) {
    scale_positioning(P, 1.0 / average_edge_length(G_in, P));
    ITERATE(G_in, P, NULL, NULL, local_2d,
	    v_list_in_1_1, slow, factor2, steps);
    CHECK(lift_vertices(G_in, P, f_in, 0.25));
  }
  else {
    if (dimension == 3) {
      placer = central_3d;
      v_list = v_list_sub_0_1;
      scale_positioning(P, 0.1 / average_edge_length(G_in, P));
    }
    else {
      if (!placer) placer = equal_area;
      v_list = v_list_sub_1_0;
    }
    ITERATE(G_sub, P, NULL, NULL, placer, v_list, slow, factor2, steps);
  }

  /* --- Phase 3 --- */

  scale_positioning(P, 1.0 / average_edge_length(G_in, P));

  if (dimension == 3) {
    if (init_mode == 'p')
      v_list = v_list_in_1_1;
    else
      v_list = v_list_aug_1_1;

    ITERATE(G_in, P, NULL, NULL, local_3d, v_list, slow, factor3, steps);
  }
  else
    ITERATE(G_in, P, NULL, NULL, local_2d,
	    v_list_in_1_1, slow, factor3, steps);

  /* --- Determine which version of the graph to write --- */

  if (output_subdivision)
    G_out = G_sub;
  else if (output_augmented)
    G_out = G_aug;
  else
    G_out = G_in;

  /* --- Normalize the final positioning --- */

  CHECK(resize_positioning(P, G_out->size, dimension));
  CHECK(recenter_positioning(P));
  scale_positioning(P, 1.4 / average_edge_length(G_in, P));
  if (init_mode == 't')
    xz_swap_positioning(P);

  /* --- Write the results --- */

  CHECK(! write_result(G_out, P, stdout, output_format));

  /* --- Clean up --- */

  free(v_list_sub_0_1);
  free(v_list_sub_1_0);
  free(v_list_aug_1_1);
  free(v_list_in_1_1);

  free_graph(G_in);
  free_graph(G_aug);
  free_graph(G_sub);
  free_positioning(P);

  return 0;
}

int
write_result(GRAPH *G, POSITIONING *P, FILE *f, char output_format)
{
  switch (output_format) {
  case 'b':
    CHECK(writegraph_pdb(f, G, P));
    break;
  case 'p':
    CHECK(writegraph_planar(f, G));
    break;
  case 'v':
    CHECK(writegraph_vega(f, G, P));
    break;
  }
  return 0;
}


/*
** Local Variables:
** compile-command: "make -k embed"
** End:
*/

