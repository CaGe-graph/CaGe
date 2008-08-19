
# include "jni_patches.h"

# include "NativeCMLWriter.h"

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


void write_CML_dstring
 (JNIEnv *env, jobject graph, jobject elementRule,
  void *nGraph, int dimension, dstring *encoding)
{
  char buffer [100], *sep, *element;
  dstring from, to;
  int i, k, n;
  void *iterator;
  g_float **coordinate;
  jclass elementRuleClass;
  jmethodID getElement, getBytes;
  prepare_get_element
   (env, elementRule, &elementRuleClass, &getElement, &getBytes);
  clear_dstring (encoding);
  n = get_graph_size (nGraph);
  add_string (encoding, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  add_string (encoding, "<!DOCTYPE molecule SYSTEM \"cml.dtd\" []>\n");
  add_string (encoding, "<molecule convention=\"MathGraph\">\n");
  add_string (encoding, "  <atomArray>\n");
  add_string (encoding, "    <stringArray builtin=\"id\">");
  sep = "";
  for (i = 1; i <= n; ++i)
  {
    add_string (encoding, sep);
    add_string (encoding, "a");
    sprintf (buffer, "%d", i);
    add_string (encoding, buffer);
    sep = " ";
  }
  add_string (encoding, "</stringArray>\n");
  add_string (encoding, "    <stringArray builtin=\"elementType\">");
  sep = "";
  for (i = 1; i <= n; ++i)
  {
    element = get_element
     (env, elementRule, elementRuleClass, getElement, getBytes, graph, i);
    if (element == NULL) element = "X";
    add_string (encoding, sep);
    add_string (encoding, element);
    sep = " ";
  }
  add_string (encoding, "</stringArray>\n");
  coordinate = malloc (n * sizeof (*coordinate));
  for (i = 0, k = 1; i < n; i = k++)
  {
    coordinate [i] = dimension == 2 ?
     get_2d_coordinates (nGraph, k) : get_3d_coordinates (nGraph, k);
  }
  for (k = 0; k < dimension; ++k)
  {
    add_string (encoding, "    <floatArray builtin=\"");
    add_char (encoding, 'x' + k);
    sprintf (buffer, "%d", dimension);
    add_string (encoding, buffer);
    add_string (encoding, "\">");
    sep = "";
    for (i = 0; i < n; ++i)
    {
      add_string (encoding, sep);
      if (sizeof (g_float) == sizeof (float)) {
	  sprintf (buffer,  "%-g", coordinate [i][k]);
      } else {
	  sprintf (buffer, "%-lg", coordinate [i][k]);
      }
      add_string (encoding, buffer);
      sep = " ";
    }
    add_string (encoding, "</floatArray>\n");
  }
  add_string (encoding, "  </atomArray>\n");
  add_string (encoding, "  <bondArray>\n");
  from = new_dstring;
  to = new_dstring;
  sep = "";
  for (i = 1; i <= n; ++i)
  {
    void *iterator;
    iterator = get_edge_iterator (nGraph, i);
    while (has_next_edge (iterator))
    {
      int j = get_next_edge (iterator);
      if (j <= i) continue;
      sprintf (buffer, "%sa", sep);
      add_string (&from, buffer);
      add_string (&to, buffer);
      sprintf (buffer, "%d", i);
      add_string (&from, buffer);
      sprintf (buffer, "%d", j);
      add_string (&to, buffer);
      sep = " ";
    }
  }
  add_string (encoding, "    <stringArray builtin=\"atomRef\">");
  add_bytes (encoding, from.base, from.length);
  add_string (encoding, "</stringArray>\n");
  add_string (encoding, "    <stringArray builtin=\"atomRef\">");
  add_bytes (encoding, to.base, to.length);
  add_string (encoding, "</stringArray>\n");
  add_string (encoding, "  </bondArray>\n");
  add_string (encoding, "</molecule>\n");
  clear_dstring (&from);
  clear_dstring (&to);
}

JNIEXPORT jbyteArray JNICALL Java_cage_writer_NativeCMLWriter_nEncodeGraph
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
  if (check_graph (graph, (int) dimension, "NativeCMLWriter")) {
    return NULL;
  }
  write_CML_dstring (env, jGraph, elementRule, graph, dimension, &encoding);
  result = dstring2jbytes (env, encoding);
  clear_dstring (&encoding);
  return result;
}

