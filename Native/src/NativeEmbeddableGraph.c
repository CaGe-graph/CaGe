
# include <stdlib.h>
# include <stdio.h>

# include "jni_patches.h"

# include <jni.h>

# include "error_exit.h"

# include "NativeEmbeddableGraph.h"

# include "graph.h"

# include "jbytes.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


extern int debug_native_graph;


JNIEXPORT jlong JNICALL Java_cage_NativeEmbeddableGraph_newNGraph
  (JNIEnv *env, jobject this)
{
  jlong graph_adr;
  jlong2adr (graph_adr, void *) = get_new_graph();
  return graph_adr;
}


jfloatArray
gfloats_to_new_jfloats
 (JNIEnv *env, g_float *value, int length)
{
  int i;
  jfloatArray array;
  jfloat *element;
  array = (*env)->NewFloatArray (env, (jsize) length);
  element = (*env)->GetFloatArrayElements (env, array, NULL);
  for (i = length-1; i >= 0; --i)
  {
    element [i] = value [i];
  }
  (*env)->ReleaseFloatArrayElements (env, array, element, JNI_COMMIT);
  return array;
}

void
jfloats_to_gfloats
 (JNIEnv *env, jfloatArray array, g_float *element, int length)
{
  int i;
  jfloat *value;
  value = (*env)->GetFloatArrayElements (env, array, NULL);
  for (i = length-1; i >= 0; --i)
  {
    element [i] = value [i];
  }
  (*env)->ReleaseFloatArrayElements (env, array, value, JNI_ABORT);
}


JNIEXPORT jbyteArray JNICALL Java_cage_NativeEmbeddableGraph_nGetComment
  (JNIEnv *env, jobject this, jlong nGraph)
{
  return string2jbytes (env, get_graph_comment (jlong2adr (nGraph, void *)));
}

JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nSetComment
  (JNIEnv *env, jobject this, jlong nGraph, jbyteArray comment)
{
  set_graph_comment (jlong2adr (nGraph, void *), jbytes2string (env, comment));
}


JNIEXPORT jint JNICALL Java_cage_NativeEmbeddableGraph_nGetFormat
  (JNIEnv *env, jobject this, jlong nGraph)
{
  return (jint) get_graph_format (jlong2adr (nGraph, void *));
}

JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nSetFormat
  (JNIEnv *env, jobject this, jlong nGraph, jint format)
{
  set_graph_format (jlong2adr (nGraph, void *), format);
}


JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nAddVertex
  (JNIEnv *env, jobject this, jlong nGraph)
{
  add_vertex (jlong2adr (nGraph, void *));
}

JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nAddEdge
  (JNIEnv *env, jobject this, jlong nGraph, jint to)
{
  add_edge (jlong2adr (nGraph, void *), (g_int) to);
}


JNIEXPORT jint JNICALL Java_cage_NativeEmbeddableGraph_nGetSize
  (JNIEnv *env, jobject this, jlong nGraph)
{
  return (jint) get_graph_size (jlong2adr (nGraph, void *));
}

JNIEXPORT jint JNICALL Java_cage_NativeEmbeddableGraph_nGetValency
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex)
{
  return (jint) get_valency (jlong2adr (nGraph, void *), (g_int) vertex);
}


JNIEXPORT jobject JNICALL Java_cage_NativeEmbeddableGraph_nGetEdgeIterator
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex)
{
  void *iter;
  jclass iter_class;
  jobject iter_obj;
  jlong iter_adr;
  jfieldID iter_field;
  if (debug_native_graph) fprintf (stderr, "{ nGetEdgeIterator\n");
  iter = get_edge_iterator (jlong2adr (nGraph, void *), vertex);
  iter_class = (*env)->FindClass (env, "cage/NativeEdgeIterator");
  iter_obj = (*env)->AllocObject (env, iter_class);
  jlong2adr (iter_adr, void *) = iter;
  iter_field = (*env)->GetFieldID (env, iter_class, "nIter", "J");
  (*env)->SetLongField (env, iter_obj, iter_field, iter_adr);
  if (debug_native_graph) fprintf (stderr, "} nGetEdgeIterator\n");
  return iter_obj;
}


JNIEXPORT jboolean JNICALL Java_cage_NativeEmbeddableGraph_nHas2DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph)
{
  return has_2d_coordinates (jlong2adr (nGraph, void *)) ?
          JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jfloatArray JNICALL Java_cage_NativeEmbeddableGraph_nGet2DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex)
{
  return gfloats_to_new_jfloats (env,
   get_2d_coordinates (jlong2adr (nGraph, void *), (g_int) vertex), 2);
}

JNIEXPORT jobjectArray JNICALL Java_cage_NativeEmbeddableGraph_nGetAll2DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph)
{
  void *graph;
  g_int size;
  g_float (*coords) [2];
  jobjectArray array;
  jclass floatArrayClass;
  jfloatArray *element;
  int i;
  graph = jlong2adr (nGraph, void *);
  size = get_graph_size (graph);
  coords = (g_float (*) [2]) locate_2d_coordinates (graph, (g_int) 1);
  floatArrayClass = (*env)->FindClass (env, "[F");
  array = (*env)->NewObjectArray (env, (jsize) size, floatArrayClass, NULL);
  for (i = 0; i < size; ++i)
  {
    (*env)->SetObjectArrayElement (env, array, i,
     gfloats_to_new_jfloats (env, *(coords++), 2));
  }
  return array;
}


JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nSet2DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex, jfloatArray coords)
{
  jfloats_to_gfloats (env, coords,
   locate_2d_coordinates (jlong2adr (nGraph, void *), (g_int) vertex), 2);
}


JNIEXPORT jboolean JNICALL Java_cage_NativeEmbeddableGraph_nHas3DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph)
{
  return has_3d_coordinates (jlong2adr (nGraph, void *)) ?
          JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jfloatArray JNICALL Java_cage_NativeEmbeddableGraph_nGet3DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex)
{
  return gfloats_to_new_jfloats (env,
   get_3d_coordinates (jlong2adr (nGraph, void *), (g_int) vertex), 3);
}

JNIEXPORT jobjectArray JNICALL Java_cage_NativeEmbeddableGraph_nGetAll3DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph)
{
  void *graph;
  g_int size;
  g_float (*coords) [3];
  jobjectArray array;
  jclass floatArrayClass;
  jfloatArray *element;
  int i;
  graph = jlong2adr (nGraph, void *);
  size = get_graph_size (graph);
  coords = (g_float (*) [3]) locate_3d_coordinates (graph, (g_int) 1);
  floatArrayClass = (*env)->FindClass (env, "[F");
  array = (*env)->NewObjectArray (env, (jsize) size, floatArrayClass, NULL);
  for (i = 0; i < size; ++i)
  {
    (*env)->SetObjectArrayElement (env, array, i,
     gfloats_to_new_jfloats (env, *(coords++), 3));
  }
  return array;
}


JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nSet3DCoordinates
  (JNIEnv *env, jobject this, jlong nGraph, jint vertex, jfloatArray coords)
{
  jfloats_to_gfloats (env, coords,
   locate_3d_coordinates (jlong2adr (nGraph, void *), (g_int) vertex), 3);
}


JNIEXPORT jbyteArray JNICALL Java_cage_NativeEmbeddableGraph_toBytes
  (JNIEnv *env, jobject this, jlong nGraph)
{
  jbyteArray result;
  dstring encoding = new_dstring;
  if (debug_native_graph) fprintf (stderr, "{ toBytes\n");
  write_writegraph_dstring (jlong2adr (nGraph, void *), &encoding, 0, 1, 0, 0);
  result = dstring2jbytes (env, encoding);
  clear_dstring (&encoding);
  if (debug_native_graph) fprintf (stderr, "} toBytes\n");
  return result;
}


JNIEXPORT void JNICALL Java_cage_NativeEmbeddableGraph_nFinalize
  (JNIEnv *env, jobject this, jlong nGraph)
{
  void *graph;
  if (debug_native_graph) fprintf (stderr, "{ nFinalize /* NGraph */\n");
  graph = jlong2adr (nGraph, void *);
  clear_graph (graph);
  free (graph);
  if (debug_native_graph) fprintf (stderr, "} nFinalize /* NGraph */\n");
}

