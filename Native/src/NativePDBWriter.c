
# include "jni_patches.h"

# include "NativePDBWriter.h"

# include "malloc.h"

# include "dstring.h"

# include "graph.h"

# include "jbytes.h"

# include "check_graph.h"

# include "get_element.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


void write_PDB_dstring
 (JNIEnv *env, jobject graph, jobject elementRule,
  void *nGraph, int dimension, dstring *encoding)
{
  char buffer [100], *element;
  int i, size;
  void *iterator;
  g_float *coord;
  jclass elementRuleClass;
  jmethodID getElement, getBytes;
  prepare_get_element
   (env, elementRule, &elementRuleClass, &getElement, &getBytes);
  clear_dstring (encoding);
  size = get_graph_size (nGraph);
  for (i = 1; i <= size; ++i)
  {
    coord = get_3d_coordinates (nGraph, i);
    element = get_element
     (env, elementRule, elementRuleClass, getElement, getBytes, graph, i);
    if (element == NULL) element = "X";
    sprintf (buffer,
     "ATOM  %5d  %3s              %8.3f%8.3f%8.3f                          \n",
     i, element, coord [0], coord [1], coord [2]);
    add_string (encoding, buffer);
  }
  for (i = 1; i <= size; ++i)
  {
    void *iterator;
    int k = 0;
    iterator = get_edge_iterator (nGraph, i);
    while (has_next_edge (iterator))
    {
      int j;
      if (k % 6 == 0) {
	  if (k > 0) add_char (encoding, '\n');
          add_string (encoding, "CONECT");
	  sprintf (buffer, "%5d", i);
	  add_string (encoding, buffer);
      }
      ++k;
      j = get_next_edge (iterator);
      sprintf (buffer, "%5d", j);
      add_string (encoding, buffer);
    }
    free (iterator);
    add_char (encoding, '\n');
  }
  add_string (encoding, "END\n");
}

JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativePDBWriter_nEncodeGraph
  (JNIEnv *env, jobject this, jobject jGraph, jobject elementRule, jint dimension)
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
  if (check_graph (graph, (int) dimension, "NativePDBWriter")) {
    return NULL;
  }
  write_PDB_dstring (env, jGraph, elementRule, graph, dimension, &encoding);
  result = dstring2jbytes (env, encoding);
  clear_dstring (&encoding);
  return result;
}

