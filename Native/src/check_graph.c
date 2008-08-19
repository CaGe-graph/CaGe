
# include <stdio.h>

# include "error_exit.h"

# include "graph.h"


int
check_coordinates (void *graph, int dimension)
{
  return 
   dimension == 2 ? (! has_2d_coordinates (graph)) :
   dimension == 3 ? (! has_3d_coordinates (graph)) :
   0;
}

int
check_graph (void *graph, int dimension, char *function_name)
{
  char msg [80];
  if (graph == NULL) {
      sprintf (msg, "%s received NULL graph", function_name);
      error_exit (msg, "io/IOException");
      return 1;
  } else if (check_coordinates (graph, dimension)) {
      sprintf (msg, "%s received graph with no coordinates", function_name);
      error_exit (msg, "io/IOException");
      return 1;
  }
  return 0;
}

