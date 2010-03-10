
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <stdio.h>

# include "malloc.h"

# include "dstring.h"

# include "graph.h"


extern int debug_read_graphs;


/*
   For each format produced by our generators, we need a reader and a parser.
   Readers read a byte representation of a graph from a file and store the
   data in a dstring without further analysis.  This segments the data stream
   into graph encodings and remembers the last one.
   Parsers analyse such a dstring and build a graph structure from it.
   Outside the parser, the graph is put into a EmbeddedGraph Java object.

   Each reader or parser returns 0 on success, 1 on failure.
   In case of success, readers should try to detect if the graph encoding
   just read is the last in the file, e.g. by trying to read on to the
   first character of the next graph and then ungetc'ing it.

   Reader parameters are a file to read from, a format code (required
   to pass on format parameters) and a dstring which will contain the
   character encoding of the graph (which will be cleared before use).

   Parser parameters are the dstring produced by a successful reader call,
   the format code, and a graph which is assumed to be newly obtained
   by a call to get_new_graph.

   Format codes can be obtained by calling start_reading at the beginning
   of the output (below).
*/

# define  FORMATS              2
# define  FORMAT_PARAM_BITS    2

# define  PARAM_MASK             ((1 << FORMAT_PARAM_BITS) - 1)
# define  get_format(format)     ((format) >> FORMAT_PARAM_BITS)
# define  get_params(format)     ((format) & PARAM_MASK)
# define  set_params(fmt,par)    (((fmt) & ~PARAM_MASK) | (par))
# define  format_equals(f1, f2)  (!(((f1)^(f2)) >> FORMAT_PARAM_BITS))

# define  WRITEGRAPHxD_FORMAT  4
# define  PLANAR_XE_FORMAT     8
# define  PLANAR_BE_FORMAT     9
# define  PLANAR_LE_FORMAT     10


int read_writegraph (FILE *file, int format, dstring *graph_encoding)
{
  dstring new_encoding;
  int c, status = 0;
  if (debug_read_graphs) fprintf (stderr, "reading writegraph\n");
  new_encoding = new_dstring;
  while ((c = getc (file)) != EOF)
  {
    /* if (c == '\0') break; */
    add_char (&new_encoding, c);
    switch (status)
    {
      case 0:
        if (isdigit (c)) status = 1;
	break;
      case 1:
        if (c == '\n') status = 2;
	break;
      case 2:
        if (c == '0') status = 3;
	else if (! isspace (c)) status = 1;
	break;
      case 3:
        if (c == '\n') {
	    status = 4;
	    break;
	} else if (! isspace (c)) status = 1;
	break;
    }
    if (status == 4) {
	while ((c = getc (file)) != EOF)
	{
	  if (isdigit (c)) {
	      ungetc (c, file);
	      break;
	  }
	}
        break;
    }
  }
  if (status == 2 || status == 4) {
      add_char (&new_encoding, '\0');
      clear_dstring (graph_encoding);
      *graph_encoding = new_encoding;
      return 0;
  } else {
      clear_dstring (&new_encoding);
      return 1;
  }
}

/*
   There are two versions of planar code, single-byte and two-byte.
   (Which byte order is used for two-byte is given in the format parameter.)
   We read all versions, but before storing in graph_encoding we convert
   to two-byte code in native byte order, for easier parsing.

   The next macro decodes a 2-byte word, given a char pointer to it,
   and returning a word (short) value in machine order.  If format is
   odd (e.g. 1), reads a big-endian word, if even (2), a little-endian one.
*/
# define  decode_word(adr,format)  (((adr) [(format)&1]) | (((unsigned short) (adr) [1 ^ ((format) & 1)]) << 8))

int read_planar (FILE *file, int format, dstring *graph_encoding)
{
  dstring new_encoding;
  int c, single_byte, codelen, state;
  unsigned short vertices, n;
  if (debug_read_graphs) fprintf (stderr, "reading planar\n");
  new_encoding = new_dstring;
  if ((c = getc (file)) == EOF) return 1;
  single_byte = (c != '\0');
  state = 0;
  codelen = single_byte ? 1 : 2;
  if (single_byte) {
      ungetc (c, file);
  }
  while (fread (&c, codelen, 1, file))
  {
    if (single_byte) {
        n = (unsigned short) * (unsigned char *) &c;
    } else {
        n = decode_word ((unsigned char *) &c, format);
    }
    add_bytes (&new_encoding, (char *) &n, sizeof (n));
    switch (state)
    {
      case 0:
        vertices = n;
	if (! vertices) {
	    clear_dstring (graph_encoding);
	    *graph_encoding = new_encoding;
	    if ((c = getc (file)) != EOF) ungetc (c, file);
	    return 0;
	}
	state = 1;
	break;
      case 1:
        if (n) continue;
	if (--vertices) continue;
	clear_dstring (graph_encoding);
	*graph_encoding = new_encoding;
	if ((c = getc (file)) != EOF) ungetc (c, file);
	return 0;
    }
  }
  clear_dstring (&new_encoding);
  return 1;
}

int (*reader [FORMATS]) (FILE *file, int format, dstring *graph_encoding)
 = { read_writegraph, read_planar };


/*
   strxpbrk does the same as strpbrk(3), only considers the
   trailing null byte part of the "accept" string
*/
char *strxpbrk (char *string, char *accept)
{
  char found [256], *c;
  memset (found, 0, sizeof (found));
  found [0] = (char) 1;
  for (c = accept; *c; ++c)
    found [*c] = (char) 1;
  for (c = string; ; ++c)
    if (found [*c]) return c;
}

int parse_writegraph (dstring encoded_graph, int format, void *graph)
{
  int dimension, vertex, v2, i, p;
  char *this_line, *next_line, *end_of_graph;
  dstring coord_string;
  double coord;
  g_float *g_coord;

  if (debug_read_graphs) fprintf (stderr, "parsing writegraph\n");

  dimension = get_params (format);
  coord_string = new_dstring;
  /* coord_string will serve as a dynamic double array for coordinates */

  vertex = 0;
  next_line = encoded_graph.base;
  end_of_graph = next_line + encoded_graph.length - 1;
  while ((this_line = next_line) < end_of_graph)
  {
    /* find end of line, overwrite eol characters with NULs, set next_line */
    next_line = strxpbrk (this_line, "\r\n");
    while (next_line < end_of_graph)
    {
      *next_line = '\0';
      if (! strchr ("\r\n", *++next_line)) break;
    }

    /* parse vertex number */
    if (sscanf (this_line, "%d %n", &i, &p) < 1) {
        continue;
    } else if (i == 0) {
        break;
    } else if (i != vertex + 1) {
        continue;
    }
    this_line += p;
    vertex = i;
    /* parse coordinates, find dimension if initially unknown */
    if (dimension) {
	set_length (&coord_string, dimension * sizeof (double));
        for (i = 0; i < dimension; ++i)
	{
	  if (sscanf (this_line, "%lf %n", &coord, &p) < 1) {
	       break;
	  }
	  this_line += p;
	  ((double *) coord_string.base) [i] = coord;
	}
	if (i < dimension) {
	    clear_dstring (&coord_string);
	    return 1;
	}
    } else {
	/* dimension yet unknown (will only work on the first line) */
	int p1;
        for (i = 0; ; ++i)
	{
	  /* parse a float */
	  if (sscanf (this_line, "%lf %n", &coord, &p) < 1) {
	      if (debug_read_graphs) {
		  fprintf (stderr, "no more float");
		  fprintf (stderr, " - dimension is %d\n", i);
	      }
	      break;
	  }
	  /* if we can parse the same string into a vertex number, finish */
	  if (coord > 1.5) {
	      if (sscanf (this_line, "%d %n", &v2, &p1) > 0) {
	          if (p1 == p && (int) (coord + 0.5) == v2) {
		      if (debug_read_graphs) {
			  fprintf (stderr, "non-coordinate float: %lf", coord);
			  fprintf (stderr, " - dimension is %d\n", i);
		      }
		      break;
		  }
	      }
	  }
	  this_line += p;
	  set_length (&coord_string, (i+1) * sizeof (double));
	  ((double *) coord_string.base) [i] = coord;
	}
	dimension = i;
	if (dimension <= 3) format = set_params (format, dimension);
    }
    add_vertex (graph);
    /* transfer coordinates into graph */
    if (dimension >= 2 && dimension <= 3) {
	if (dimension == 2) {
	    g_coord = locate_2d_coordinates (graph, vertex);
	} else {
	    g_coord = locate_3d_coordinates (graph, vertex);
	}
	for (i = dimension-1; i >= 0; --i)
	{
	  coord = ((double *) coord_string.base) [i];
	  g_coord [i] = (g_float) coord;
	}
    }
    /* read connection list */
    while (sscanf (this_line, "%d %n", &v2, &p) > 0)
    {
      this_line += p;
      add_edge (graph, v2);
      if (debug_read_graphs) fprintf (stderr, "%d -> %d\n", vertex, v2);
    }
  }
  clear_dstring (&coord_string);
  set_graph_format (graph, format);
  return 0;
}

int parse_planar (dstring encoded_graph, int format, void *graph)
{
  unsigned short *n;
  int vertices, v1, v2;

  if (debug_read_graphs) fprintf (stderr, "parsing planar\n");

  n = (unsigned short *) encoded_graph.base;
  vertices = *n;
  for (v1 = 1; v1 <= vertices; ++v1)
  {
    add_vertex (graph);
    while (v2 = *++n)
    {
      add_edge (graph, (g_int) v2);
      if (debug_read_graphs) fprintf (stderr, "%d -> %d\n", v1, v2);
    }
  }
  set_graph_format (graph, format);
  return 0;
}

int (*parser [FORMATS])
 (dstring encoded_graph, int format, void *graph)
 = { parse_writegraph, parse_planar };



/*
   start_reading determines the format of the data coming out of a file.
   The format code is returned in *formatp, and if *formatp is initially
   non-zero, no change is made and nothing read.  A header is preserved
   in *header, which is cleared before use.
   Returns 0 if *formatp was 0 and a format was recognized from the data,
   1 otherwise.
   If no header is present, output starting with a "space" or "digit"
   character is recognized as some "writegraph" format, every other
   output as planar code.
*/
int start_reading (FILE *file, int *formatp, dstring *header)
{
  /* for unmarked planar code, determine this machine's endian-ness -
     my_endianness is 1 for big-endian ("big end first"), 2 for little-endian */
  unsigned short word = 0x0102;
  char my_endianness = * (char *) &word;

  int c, n;
  char *cp;

  if (*formatp) return 1;

  if ((c = getc (file)) == EOF) return 1;
  
  if (isspace (c) || isdigit (c)) {
      *formatp = WRITEGRAPHxD_FORMAT;
  } else if (c != '>') {
      *formatp = PLANAR_XE_FORMAT + my_endianness;
  }
  if (*formatp) {
      ungetc (c, file);
      return 0;
  }

  if ((c = getc (file)) == EOF) return 1;
  if (c != '>') return 1;

  clear_dstring (header);
  *header = new_dstring;
  add_string (header, ">>");
  while ((c = getc (file)) != '<')
  {
    if (c != '\0' && c != EOF) {
	add_char (header, c);
    } else {
	clear_dstring (header);
	return 1;
    }
  }
  if ((c = getc (file)) != '<') {
      clear_dstring (header);
      return 1;
  }
  add_string (header, "<<");

  add_char (header, '\0');
  if (debug_read_graphs)
      fprintf (stderr, "header: %s\n", header->base);
  cp = (char *) &c;
  n = 0;
  if (! n) sscanf (header->base + 2, " writegraph%[23]d%*1[ <]%n", cp, &n);
  if (! n) sscanf (header->base + 2, " writegraph_%[23]d%*1[ <]%n", cp, &n);
  if (n) {
      *formatp = WRITEGRAPHxD_FORMAT + (*cp - '0');
      header->base [header->length - 1] = '\n';
      return 0;
  }
  if (! n) sscanf (header->base + 2, " planar_code%n", &n);
  if (! n) sscanf (header->base + 2, " embed_code%n", &n);
  if (n) {
      c = 0;
      /*  */ if (sscanf (header->base + 2 + n,    "%*1[ <]%n", &c), c) {
	  *formatp = PLANAR_XE_FORMAT + my_endianness;
      } else if (sscanf (header->base + 2 + n, "_le%*1[ <]%n", &c), c) {
	  *formatp = PLANAR_LE_FORMAT;
      } else if (sscanf (header->base + 2 + n, "_be%*1[ <]%n", &c), c) {
	  *formatp = PLANAR_BE_FORMAT;
      }
  }
  set_length (header, header->length - 1);

  return (*formatp == 0);
}


void write_writegraph (void *graph, void out (void *x, char *string), void *x,
 int dimension, int with_coordinates, int header, int footer)
{
  int format, has_2d, has_3d, use_2d, use_3d, i;
  g_int size, vertex;
  g_float *coord;
  void *iter;
  char buffer [20];
  has_2d = has_2d_coordinates (graph);
  has_3d = has_3d_coordinates (graph);
  if (with_coordinates) {
      use_2d = dimension == 0 ? has_2d : dimension == 2;
      use_3d = dimension == 0 ? has_3d : dimension == 3;
  } else {
      use_2d = use_3d = dimension = 0;
  }
  if (header) {
      if (dimension < 2 || dimension > 3) {
	  out (x, ">>writegraph<<\n");
      } else {
	  sprintf (buffer, ">>writegraph%dd<<\n", dimension);
	  out (x, buffer);
      }
  }
  size = get_graph_size (graph);
  for (vertex = 1; vertex <= size; ++vertex)
  {
    sprintf (buffer, "%4ld", (long) vertex);
    out (x, buffer);
    if (use_2d) {
	out (x, "\t");
	if (has_2d) coord = get_2d_coordinates (graph, vertex);
	for (i = 0; i < 2; ++i)
	{
	  if (sizeof (g_float) == sizeof (float)) {
	      sprintf (buffer,  "\t% -6g", has_2d ? coord [i] : 0.0f);
	  } else {
	      sprintf (buffer, "\t% -6lg", has_2d ? coord [i] : 0.0);
	  }
	  out (x, buffer);
	}
    }
    if (use_3d) {
	out (x, "\t");
	if (has_3d) coord = get_3d_coordinates (graph, vertex);
	for (i = 0; i < 3; ++i)
	{
	  if (sizeof (g_float) == sizeof (float)) {
	      sprintf (buffer,  "\t% -6g", has_3d ? coord [i] : 0.0f);
	  } else {
	      sprintf (buffer, "\t% -6lg", has_3d ? coord [i] : 0.0);
	  }
	  out (x, buffer);
	}
    }
    out (x, "\t\t");
    iter = get_edge_iterator (graph, vertex);
    while (has_next_edge (iter))
    {
      sprintf (buffer, "  %ld", (long) get_next_edge (iter));
      out (x, buffer);
    }
    free (iter);
    out (x, "\n");
  }
  if (footer) out (x, "0\n");
}

static void fout (void *file, char *string)
{
  (void) fputs (string, (FILE *) file);
}

void write_writegraph_file (void *graph, FILE *file,
 int dimension, int with_coordinates, int header, int footer)
{
  write_writegraph (graph, fout, file,
   dimension, with_coordinates, header, footer);
}

static void dsout (void *ds, char *string)
{
  add_string ((dstring *) ds, string);
}

void write_writegraph_dstring (void *graph, dstring *ds,
 int dimension, int with_coordinates, int header, int footer)
{
  write_writegraph (graph, dsout, ds,
   dimension, with_coordinates, header, footer);
}

