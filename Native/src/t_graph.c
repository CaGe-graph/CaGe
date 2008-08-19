
# include <stdio.h>

# include "error_exit.h"

# include "graph.h"


char *cmd_name = "t_graph";

int debug_graph = 1, debug_malloc = 1;


int main ()
{
  void *graph;
  void *iter;
  g_float c [3];
  const g_float *d;
  g_int n, i;
  graph = get_new_graph ();
  add_vertex (graph);
  add_edge (graph, 2);
  add_vertex (graph);
  add_edge (graph, 1);
  c [0] = 0.1;
  c [1] = 0.2;
  c [2] = 0.3;
  set_2d_coordinates (graph, 1, c+1);
  set_3d_coordinates (graph, 1, c);
  c [0] = 0.4;
  c [1] = 0.5;
  c [2] = 0.6;
  set_2d_coordinates (graph, 2, c);
  set_3d_coordinates (graph, 2, c);
  n = get_graph_size (graph);
  for (i = 1; i <= n; ++i)
  {
    printf ("%d  ", i);
    d = get_2d_coordinates (graph, i);
    printf ("(%lg, %lg) ", (double) d [0], (double) d [1]);
    d = get_3d_coordinates (graph, i);
    printf ("(%lg, %lg, %lg) ", (double) d [0], (double) d [1], (double) d [2]);
    if ((iter = get_edge_iterator (graph, i)) == NULL) {
	printf ("\n");
        fprintf (stderr, "Iterator is NULL\n");
	exit (2);
    }
    printf (" -> ");
    while (has_next_edge (iter))
    {
      printf (" %d", get_next_edge (iter));
    }
    free (iter);
    printf ("\n");
  }
  clear_graph (graph);
  free (graph);
  return 0;
}

