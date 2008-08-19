
/*
   g_int is the type of integer we will use to count vertices
   and edges. The size of this integer and the amount of
   available memory restrict the size of our graphs.

   g_float is the float type we will use to store coordinates.
*/

typedef  unsigned short  g_int;
typedef  float           g_float;


extern void *get_new_graph (void);
/*
   Creates a new graph.
   The pointer returned is created by malloc.
   It can be freed straight away, but after building up a graph
   using "add_vertex", "add_edge" and "set_?d_coordinates",
   call "clear_graph" first to avoid memory leaks.
*/

extern void clear_graph (void *graph);
/*
   Frees any memory associated with a graph, but not the graph pointer itself.
   The structure pointed to by "graph" is reset to the state created
   by a call to "get_new_graph", so it could be used to build up another graph.
*/

extern void set_graph_comment (void *graph, char *comment);
/*
   Stores a string in a graph that can contain anything and is not
   touched during graph processing.  clear_graph does not do anything
   to the address either, but erases it from the graph structure,
   so take care to free comments yourself before calling clear_graph.
*/

extern char *get_graph_comment (void *graph);
/*
   Returns exactly the address previously stored via set_graph_comment.
*/

extern void set_graph_format (void *graph, int format);
/*
   Stores an integer that is also ignored.
*/

extern int get_graph_format (void *graph);
/*
   Returns the previously stored integer.
*/

extern int add_vertex (void *graph);
/*
   Adds a new vertex to a graph.
   The new vertex is automatically assigned the next available number.
   Vertex numbers start at one, not zero.
*/

extern int add_edge (void *graph, g_int to);
/*
   Adds an edge from the last added vertex to another one.
   Vertex numbers start at one, not zero.
   Of course vertex "to" may not yet have been added.
*/

extern g_int get_graph_size (void *graph);
/*
   Returns the number of vertices in the graph.
*/

extern g_int get_valency (void *graph, g_int from);
/*
   Returns the number of edges starting from vertex "from".
*/

extern void *get_edge_iterator (void *graph, g_int from);
/*
   Returns an address that can be used for subsequent calls
   to has_next_edge and get_next_edge.
   The address can be freed and then discarded.
   Returns NULL if vertex "from" does not exist in the graph.
   Don't change graph "graph" while using an edge_iterator.
*/

extern g_int get_next_edge (void *edge_iterator);
/*
   Returns the number of a vertex that is connected to vertex "from"
   in graph "graph", in the call of "get_edge_iterator"
   which returned "edge_iterator".
   Should only be called if has_next_edge has previously returned 1.
   Subsequent calls step through all edges starting from vertex "from".
*/

extern int has_next_edge (void *edge_iterator);
/*
   Returns 1 if it is ok to call get_next_edge on edge_iterator.
*/

extern int has_2d_coordinates (void *graph);
extern void set_2d_coordinates (void *graph, g_int vertex, g_float *coords);
extern g_float *get_2d_coordinates (void *graph, g_int vertex);
extern g_float *locate_2d_coordinates (void *graph, g_int vertex);
extern void transplant_2d_coordinates (void *dest_graph, void *source_graph);
extern int has_3d_coordinates (void *graph);
extern void set_3d_coordinates (void *graph, g_int vertex, g_float *coords);
extern g_float *get_3d_coordinates (void *graph, g_int vertex);
extern g_float *locate_3d_coordinates (void *graph, g_int vertex);
extern void transplant_3d_coordinates (void *dest_graph, void *source_graph);
/*
   These functions deal with vertex coordinates.

   All g_float pointers point to the first of 2 resp. 3 coordinate values.
   Coordinates are copied from the place pointed to by "coords" to another.
   The address passed to set_?d_coordinates is not remembered or administered,
   and the address returned by get_?d_coordinates will be a different one.
   It is possible to write to 2 resp. 3 g_floats starting from the address
   returned by get_?d_coordinates and locate_?d_coordinates.  The address
   returned by the "get" or "locate" functions is only valid until the next
   call to the corresponding "set" or "locate" function.

   If you call get_?d_coordinates, they must have been set before, or
   an invalid address might be returned.  locate_?d_coordinates always
   returns a valid, writable address, and if coordinates have previously
   been set, they will be at the returned location.

   has_?d_coordinates indicates whether the corresponding set_?d_coordinates
   function has ever been called, it does not give information about the
   state of any particular vertex.  If 1 is returned, it is ensured that no
   memory violation will occur when attempting to get the coordinates of
   vertices 1 up to get_graph_size (graph), although no useful values will
   be returned by the "get" functions if the coordinates for any of these
   vertices have not been set.

   transplant_?d_coordinates disposes of any existing coordinates of the
   indicated dimension in dest_graph, then takes the appropriate coordinates
   address from source_graph and stores it in source_graph.  dest_graph
   loses that coordinate area (important for clear_graph) and behaves
   as if these coordinates were never set.
*/

