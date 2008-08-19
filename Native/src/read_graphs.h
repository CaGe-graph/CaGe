
# include "dstring.h"


/*
   start_reading determines the format of the data coming out a file.
   The format code is returned in *formatp, and if *formatp is initially
   non-zero, no change is made and nothing read.  A header is preserved
   in *header, which is assumed not to contain a previous dstring.
   Returns 0 if *formatp was 0 and a format was recognized from the data,
   1 otherwise.
   If no header is present, output starting with a "space" or "digit"
   character is recognized as some "writegraph" format, every other
   output as planar code.
*/
extern int start_reading (FILE *file, int *formatp, dstring *header);

/*
   For each format produced by our generators, we need a reader and a parser.
   Readers read a byte representation of a graph from a file and store the
   data in a dstring without further analysis.  This segments the data stream
   into graph encodings and remembers the last one.
   Parsers analyse such a dstring and build a graph structure from it.
   Outside the parser, the graph is put into a EmbeddedGraph Java object.

   Each reader or parser returns 0 on success, 1 on failure.

   Which format the current pipe produces is determined by start_reading.
*/

# define  FORMATS              2
# define  FORMAT_PARAM_BITS    2

# define  PARAM_MASK             ((1 << FORMAT_PARAM_BITS) - 1)
# define  get_format(format)     ((format) >> FORMAT_PARAM_BITS)
# define  get_params(format)     ((format) & PARAM_MASK)
# define  set_params(fmt,par)    (((fmt) & ~PARAM_MASK) | (par))
# define  format_equals(f1, f2)  (!(((f1)^(f2)) >> FORMAT_PARAM_BITS))

# define  WRITEGRAPHxD_FORMAT  4
# define  PLANAR_LE_FORMAT     8
# define  PLANAR_BE_FORMAT     9
# define  PLANAR_STD_FORMAT    PLANAR_LE_FORMAT


extern int read_writegraph (FILE *file, int format, dstring *graph_encoding);

extern int read_planar (FILE *file, int format, dstring *graph_encoding);

extern int (*reader [FORMATS])
 (FILE *file, int format, dstring *graph_encoding);

extern int parse_writegraph (dstring encoded_graph, int format, void *graph);

extern int parse_planar (dstring encoded_graph, int format, void *graph);

extern int (*parser [FORMATS])
 (dstring encoded_graph, int format, void *graph);

/*
   write_writegraph_* writes a graph into a file in writegraph format.
   The "dimension" parameter determines whether writegraph_2d or
   writegraph_3d is used.  If "with_coordinates" is non-zero and coordinates
   for the dimension are present in "graph", they are written; otherwise
   zero coordinates are used.  If "header" is non-zero, a header in
   angle brackets format is prepended.

   write_writegraph_dstring writes to a dstring.
   write_writegraph_file writes to a file.
   write_writegraph takes a function "out" which is called each time
   part of the encoding is to be written.  "out" takes a pointer parameter,
   x, which write_writegraph will take from its own "x" parameter and
   pass through to "out" unchanged.
*/
extern void write_writegraph
 (void *graph, void out (void *x, char *string), void *x,
 int dimension, int with_coordinates, int header, int footer);

extern void write_writegraph_file
 (void *graph, FILE *file,
 int dimension, int with_coordinates, int header, int footer);

extern void write_writegraph_dstring
 (void *graph, dstring *ds,
 int dimension, int with_coordinates, int header, int footer);

