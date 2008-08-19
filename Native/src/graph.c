
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# include "malloc.h"

# include "graph.h"
/*
   These functions implement the interface defined in "graph.h".
   Most comments are in that file.
*/


struct embedded_graph
{
  char     *comment;
  int      format;
  g_int    vertices, edges;
  struct vertex
  {
    g_int  valency;
    g_int  edgeind;
  }        *vertex;
  g_int    *edgespace;
  g_float  (*embedding_2d) [2], (*embedding_3d) [3];
};
typedef struct embedded_graph embedded_graph;
const embedded_graph new_graph = { NULL, 0, 0, 0, NULL, NULL, NULL, NULL };


void *
get_new_graph (void)
{
  embedded_graph *result = malloc (sizeof (embedded_graph));
  *result = new_graph;
  return result;
}


void
clear_graph (void *g)
{
  embedded_graph *graph;
  graph = g;
  free (graph->vertex);
  free (graph->edgespace);
  free (graph->embedding_2d);
  free (graph->embedding_3d);
  *graph = new_graph;
}


char *
get_graph_comment (void *graph)
{
  return ((embedded_graph *) graph)->comment;
}


void
set_graph_comment (void *graph, char *comment)
{
  ((embedded_graph *) graph)->comment = comment;
}


int
get_graph_format (void *graph)
{
  return ((embedded_graph *) graph)->format;
}


void
set_graph_format (void *graph, int format)
{
  ((embedded_graph *) graph)->format = format;
}


int
add_vertex (void *g)
{
  embedded_graph *graph;
  g_int v;
  graph = g;
  v = graph->vertices++;
  graph->vertex = realloc (graph->vertex,
   graph->vertices * sizeof (struct vertex));
  graph->vertex [v].valency = 0;
  graph->vertex [v].edgeind = graph->edges;
  return 0;
}


int
add_edge (void *g, g_int to)
{
  embedded_graph *graph;
  g_int v;
  graph = g;
  v = graph->vertices - 1;
  ++graph->edges;
  graph->edgespace = realloc (graph->edgespace, graph->edges * sizeof (g_int));
  graph->edgespace [graph->vertex [v].edgeind + graph->vertex [v].valency++]
   = to;
  return 0;
}


g_int
get_graph_size (void *g)
{
  return ((embedded_graph *) g)->vertices;
}

g_int
get_valency (void *g, g_int from)
{
  return ((embedded_graph *) g)->vertex [from-1].valency;
}


struct edge_iterator
{
  g_int  *edge;
  g_int  rest;
};

void *
get_edge_iterator (void *g, g_int f)
{
  embedded_graph *graph;
  g_int from;
  struct edge_iterator *i;
  graph = g;
  from = f-1;
  if (from < graph->vertices) {
      i = malloc (sizeof (struct edge_iterator));
      i->edge = graph->edgespace + graph->vertex [from].edgeind;
      i->rest = graph->vertex [from].valency;
      return i;
  }
  return NULL;
}

int
has_next_edge (void *edge_iterator)
{
  return (((struct edge_iterator *) edge_iterator)->rest > 0);
}

g_int
get_next_edge (void *edge_iterator)
{
  struct edge_iterator *i;
  i = edge_iterator;
  --i->rest;
  return *(i->edge++);
}


int has_2d_coordinates (void *g)
{
  embedded_graph *graph;
  graph = g;
  if (graph->embedding_2d != NULL) {
      (void) locate_2d_coordinates (graph, graph->vertices);
      return 1;
  } else {
      return 0;
  }
}

g_float *
locate_2d_coordinates (void *g, g_int vertex)
{
  embedded_graph *graph;
  int coord_size;
  graph = g;
  coord_size = vertex > graph->vertices ? vertex : graph->vertices;
  graph->embedding_2d = realloc (graph->embedding_2d,
   coord_size * sizeof (* graph->embedding_2d));
  return ((embedded_graph *) graph)->embedding_2d [vertex - 1];
}

void
set_2d_coordinates (void *g, g_int vertex, g_float *coords)
{
  memcpy (locate_2d_coordinates (g, vertex), coords, 2 * sizeof (*coords));
}

g_float *
get_2d_coordinates (void *graph, g_int vertex)
{
  return ((embedded_graph *) graph)->embedding_2d [vertex - 1];
}

void
transplant_2d_coordinates (void *dest_graph, void *source_graph)
{
  g_float (**dest_coords) [2], (**source_coords) [2];
  dest_coords = & ((embedded_graph *) dest_graph)->embedding_2d;
  source_coords = & ((embedded_graph *) source_graph)->embedding_2d;
  free (*dest_coords);
  *dest_coords = *source_coords;
  *source_coords = NULL;
}


int has_3d_coordinates (void *g)
{
  embedded_graph *graph;
  graph = g;
  if (graph->embedding_3d != NULL) {
      (void) locate_3d_coordinates (graph, graph->vertices);
      return 1;
  } else {
      return 0;
  }
}

g_float *
locate_3d_coordinates (void *g, g_int vertex)
{
  embedded_graph *graph;
  int coord_size;
  graph = g;
  coord_size = vertex > graph->vertices ? vertex : graph->vertices;
  graph->embedding_3d = realloc (graph->embedding_3d,
   coord_size * sizeof (* graph->embedding_3d));
  return ((embedded_graph *) graph)->embedding_3d [vertex - 1];
}

void
set_3d_coordinates (void *g, g_int vertex, g_float *coords)
{
  memcpy (locate_3d_coordinates (g, vertex), coords, 3 * sizeof (*coords));
}

g_float *
get_3d_coordinates (void *graph, g_int vertex)
{
  return ((embedded_graph *) graph)->embedding_3d [vertex - 1];
}

void
transplant_3d_coordinates (void *dest_graph, void *source_graph)
{
  g_float (**dest_coords) [3], (**source_coords) [3];
  dest_coords = & ((embedded_graph *) dest_graph)->embedding_3d;
  source_coords = & ((embedded_graph *) source_graph)->embedding_3d;
  free (*dest_coords);
  *dest_coords = *source_coords;
  *source_coords = NULL;
}

