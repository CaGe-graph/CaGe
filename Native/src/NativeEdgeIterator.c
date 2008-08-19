
# include "jni_patches.h"

# include <jni.h>

# include "graph.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


JNIEXPORT jboolean JNICALL Java_cage_NativeEdgeIterator_nHasNextEdge
  (JNIEnv *env, jobject this, jlong nIter, jclass exceptionClass)
{
  return has_next_edge (jlong2adr (nIter, void *)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL Java_cage_NativeEdgeIterator_nGetNextEdge
  (JNIEnv *env, jobject this, jlong nIter)
{
  void *iter;
  jclass exceptionClass;
  iter = jlong2adr (nIter, void *);
  if (has_next_edge (iter)) {
      return (jint) get_next_edge (iter);
  } else {
      exceptionClass = (*env)->FindClass (env,
       "com/sun/java/util/collections/NoSuchElementException");
      (*env)->ThrowNew (env, exceptionClass,
       "No more edges in EmbeddableGraph");
      return 0;
  }
}

JNIEXPORT void JNICALL Java_cage_NativeEdgeIterator_nFinalize
  (JNIEnv *env, jobject this, jlong nIter)
{
  free (jlong2adr (nIter, void *));
}

