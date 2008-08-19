
# include "jni_patches.h"

# include "NativePlanarWriter.h"

# include "malloc.h"

# include "dstring.h"

# include "graph.h"

# include "check_graph.h"

# include "jbytes.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


void write_embed_dstring (void *graph, dstring *encoding)
{
  int i, size;
  void *iterator;
  clear_dstring (encoding);
  size = get_graph_size (graph);
  if (size < 0x0100) {
      int code = size;
      add_char (encoding, code);
      for (i = 1; i <= size; ++i)
      {
        iterator = get_edge_iterator (graph, i);
	while (has_next_edge (iterator))
	{
	  code = get_next_edge (iterator);
	  add_char (encoding, code);
	}
	free (iterator);
	code = 0;
	add_char (encoding, code);
      }
  } else {
      unsigned short code = size;
      add_char (encoding, 0);
      add_bytes (encoding, (char *) &code, 2);
      for (i = 1; i <= size; ++i)
      {
        iterator = get_edge_iterator (graph, i);
	while (has_next_edge (iterator))
	{
	  code = get_next_edge (iterator);
	  add_bytes (encoding, (char *) &code, 2);
	}
	free (iterator);
	code = 0;
	add_bytes (encoding, (char *) &code, 2);
      }
  }
}

JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativePlanarWriter_nEncodeGraph
  (JNIEnv *env, jobject this, jobject jGraph)
{
  void *graph;
  jclass class;
  jfieldID field;
  jlong graph_adr;
  jbyteArray result;
  dstring encoding = new_dstring;
  setJNIEnv (env);
  class = (*env)->GetObjectClass (env, jGraph);
  field = (*env)->GetFieldID (env, class, "nGraph", "J");
  graph_adr = (*env)->GetLongField (env, jGraph, field);
  graph = jlong2adr (graph_adr, void *);
  if (check_graph (graph, 0, "NativePlanarWriter")) {
    return NULL;
  }
  write_embed_dstring (graph, &encoding);
  result = dstring2jbytes (env, encoding);
  clear_dstring (&encoding);
  return result;
}

JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativePlanarWriter_header
  (JNIEnv *env, jobject this)
{
  /*
     the macro 'my_endianness' is a char value of 'b' for big-endian
     and 'l' for little-endian machines
  */
  unsigned short word = (((unsigned short) 'b') << 8) | 'l';

  # define  my_endianness  (* (char *) &word)

  char *header;
  jbyteArray result;
  header = malloc (20);
  sprintf (header, ">>planar_code %ce<<", (int) my_endianness);
  result = string2jbytes (env, header);
  free (header);
  return result;
}

