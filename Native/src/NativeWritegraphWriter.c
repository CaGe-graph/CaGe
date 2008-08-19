
# include "jni_patches.h"

# include "NativeWritegraphWriter.h"

# include "malloc.h"

# include "dstring.h"

# include "graph.h"

# include "read_graphs.h"

# include "check_graph.h"

# include "jbytes.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativeWritegraphWriter_nEncodeGraph
  (JNIEnv *env, jobject this, jobject jGraph, jint dimension)
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
  if (check_graph (graph, (int) dimension, "NativeWritegraphWriter")) {
    return NULL;
  }
  write_writegraph_dstring (graph, &encoding, (int) dimension, 1, 0, 1);
  result = dstring2jbytes (env, encoding);
  clear_dstring (&encoding);
  return result;
}

JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativeWritegraphWriter_header
  (JNIEnv *env, jobject this, jint dimension)
{
  char *header;
  jbyteArray result;
  header = malloc (20);
  sprintf (header, ">>writegraph%dd<<\n", (int) dimension);
  result = string2jbytes (env, header);
  free (header);
  return result;
}

