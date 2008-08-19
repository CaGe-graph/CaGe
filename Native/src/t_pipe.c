
# include "error_exit.h"

# include "malloc.h"

# include "cmd_vector.h"

# include "jni_patches.h"

# include "NativeCaGePipe.h"

# include "NativeEmbeddableGraph.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


char *cmd_name = "t_pipe";

int debug_native_pipe = 1;
int debug_native_graph = 1, debug_native_embedder = 1;
int debug_read_graphs = 1, debug_pipe = 1;
int debug_dstring = 0, debug_malloc = 1;


FILE *tty;
char response [80];
jlong status, nGraph;

int objects = 0;


void *
ymalloc (size_t size)
{
  void *result;
  int d = debug_malloc;
  debug_malloc = 0;
  result = malloc (size);
  debug_malloc = d;
  return result;
}

void
CallVoidMethod (JNIEnv *env, jobject obj, jmethodID method, ...)
{
  fprintf (stderr, "  CallVoidMethod: %s.%s\n", (char *) obj, (char *) method);
}

void
DeleteLocalRef (JNIEnv *env, jobject obj)
{
  fprintf (stderr, "  DeleteLocalRef: %s\n", (char *) obj);
}

jclass
FindClass (JNIEnv *env, const char *name)
{
  fprintf (stderr, "  FindClass: %s\n", name);
  return (jclass) name;
}

jboolean
GetBooleanField (JNIEnv *env, jobject obj, jfieldID fieldID)
{
  fprintf (stderr,
   "> GetBooleanField: %s.%s > ", (char *) obj, (char *) fieldID);
  fflush (stderr);
  fgets (response, sizeof (response), tty);
  return (jboolean) (response [0] == '1');
}

jint
GetIntField (JNIEnv *env, jobject obj, jfieldID fieldID)
{
  int n;
  fprintf (stderr, "> GetIntField: %s.%s > ", (char *) obj, (char *) fieldID);
  fflush (stderr);
  fgets (response, sizeof (response), tty);
  sscanf (response, "%d", &n);
  return (jint) n;
}

jlong
GetLongField (JNIEnv *env, jobject obj, jfieldID fieldID)
{
  long n;
  fprintf (stderr, "> GetLongField: %s.%s > ", (char *) obj, (char *) fieldID);
  if (! strcmp ((char *) fieldID, "status")) {
      n = (long) status;
      fprintf (stderr, "%ld\n", n);
  } else if (! strcmp ((char *) fieldID, "nGraph")) {
      n = (long) nGraph;
      fprintf (stderr, "%ld\n", n);
  } else {
      fflush (stderr);
      fgets (response, sizeof (response), tty);
      sscanf (response, "%ld", &n);
  }
  return (jlong) n;
}

jobject
GetObjectField (JNIEnv *env, jobject obj, jfieldID fieldID)
{
  char *cp;
  fprintf (stderr, "> GetObjectField: %s.%s > ", (char *) obj, (char *) fieldID);
  fflush (stderr);
  fgets (response, sizeof (response), tty);
  if (*(cp = response + strlen (response) - 1) == '\n') {
      *cp = '\0';
  }
  return (jobject) response;
}

void
SetBooleanField
 (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value)
{
  fprintf (stderr, "< SetBooleanField: %s.%s < %d\n",
   (char *) obj, (char *) fieldID, (int) value);
}

void
SetIntField
 (JNIEnv *env, jobject obj, jfieldID fieldID, jint value)
{
  fprintf (stderr, "< SetIntField: %s.%s < %d\n",
   (char *) obj, (char *) fieldID, (int) value);
}

void
SetLongField
 (JNIEnv *env, jobject obj, jfieldID fieldID, jlong value)
{
  fprintf (stderr, "< SetLongField: %s.%s < %ld\n",
   (char *) obj, (char *) fieldID, (long) value);
  if (! strcmp ((char *) fieldID, "status")) status = value;
  if (! strcmp ((char *) fieldID, "nGraph")) nGraph = value;
}

jfieldID
GetFieldID
 (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  fprintf (stderr, "  GetFieldID: %s.%s - %s\n", (char *) clazz, name, sig);
  return (jfieldID) name;
}

jmethodID
GetMethodID
 (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  fprintf (stderr, "  GetMethodID: %s.%s - %s\n", (char *) clazz, name, sig);
  return (jmethodID) name;
}

jclass
GetObjectClass (JNIEnv *env, jobject obj)
{
  char *class;
  fprintf (stderr, "  GetObjectClass: %s\n", (char *) obj);
  class = ymalloc (strlen ((char *) obj) + 12);
  sprintf (class, "%s.getClass()", (char *) obj);
  return (jclass) class;
}

jobject
NewObject (JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  char *object;
  object = ymalloc (strlen ((char *) clazz) + 8);
  sprintf (object, "%s.Obj%d", (char *) clazz, ++objects);
  fprintf (stderr, "  NewObject: %s > %s\n", (char *) clazz, object);
  return (jobject) object;
}

jfloatArray
NewFloatArray (JNIEnv *env, jsize length)
{
  char *array;
  jfloat *elems;
  array = ymalloc (20);
  sprintf (array, "FloatArray%d", ++objects);
  fprintf (stderr, "  NewFloatArray [%d] > %s\n", (int) length, array);
  elems = malloc (length * sizeof (*elems));
  memcpy (array + strlen (array) + 1, &elems, sizeof (elems));
  return (jfloatArray) array;
}

jfloat *
GetFloatArrayElements
 (JNIEnv *env, jfloatArray array, jboolean *isCopy)
{
  jfloat *elems;
  fprintf (stderr, "  GetFloatArrayElements: %s\n", (char *) array);
  memcpy (&elems, (char *) array + strlen ((char *) array) + 1, sizeof (elems));
  return elems;
}

void
ReleaseFloatArrayElements
 (JNIEnv *env, jfloatArray array, jfloat *elems, jint mode)
{
  int length, i;
  fprintf (stderr, "  ReleaseFloatArrayElements: %s", (char *) array);
  fprintf (stderr, ".length = ");
  fgets (response, sizeof (response), tty);
  sscanf (response, "%d", &length);
  for (i = 0; i < length; ++i)
  {
    fprintf (stderr, "  %s [%d] = %lf\n",
     (char *) array, i, (double) elems [i]);
  }
  free (elems);
}



int
main ()
{
  jobject pipe0 = (jobject) "CaGePipe_0", embedder0 = (jobject) "Embedder_0";
  jobject graph;
  jlong embed, nGraph;

  JNIEnv testJNI;
  testJNI = ymalloc (sizeof (*testJNI));
  testJNI->CallVoidMethod = CallVoidMethod;
  testJNI->DeleteLocalRef = DeleteLocalRef;
  testJNI->FindClass = FindClass;
  testJNI->GetBooleanField = GetBooleanField;
  testJNI->GetFieldID = GetFieldID;
  testJNI->GetFloatArrayElements = GetFloatArrayElements;
  testJNI->GetIntField = GetIntField;
  testJNI->GetLongField = GetLongField;
  testJNI->GetObjectField = GetObjectField;
  testJNI->GetMethodID = GetMethodID;
  testJNI->GetObjectClass = GetObjectClass;
  testJNI->NewFloatArray = NewFloatArray;
  testJNI->NewObject = NewObject;
  testJNI->ReleaseFloatArrayElements = ReleaseFloatArrayElements;
  testJNI->SetBooleanField = SetBooleanField;
  testJNI->SetIntField = SetIntField;
  testJNI->SetLongField = SetLongField;
  tty = fopen ("/dev/tty", "r");

  fprintf (stderr, "** startCaGePipe\n");
  Java_cage_NativeCaGePipe_startCaGePipe (&testJNI, pipe0);
  fprintf (stderr, "** nAdvanceBy(3)\n");
  Java_cage_NativeCaGePipe_nAdvanceBy (&testJNI, pipe0, (jint) 3);
  fprintf (stderr, "** Java_cage_NativeCaGePipe_getGraph\n");
  graph = Java_cage_NativeCaGePipe_getGraph (&testJNI, pipe0);
  if (graph != NULL) {
      nGraph = testJNI->GetLongField (&testJNI, graph,
       testJNI->GetFieldID (&testJNI,
	testJNI->GetObjectClass (&testJNI, graph),
	"nGraph", "J"));
      fprintf (stderr, "Parsed graph:\n---\n");
      debug_malloc = 0;
      write_writegraph_file (jlong2adr (nGraph, void *), stderr, 0, 1, 0, 0);
      debug_malloc = 1;
      fprintf (stderr, "---\n");
      fprintf (stderr, "! embed3D cmd > ");
      fflush (stderr);
      fgets (response, sizeof (response), tty);
      jlong2adr (embed, void *) = make_cmd_vector_string (response, NULL, NULL);
      fprintf (stderr, "** Java_cage_NativeEmbeddableGraph_nEmbed3D\n");
      nGraph
       = Java_cage_NativeEmbedEmbedder_nEmbed3D (&testJNI, embedder0,
	  nGraph, embed, embed);
      if (nGraph != 0) {
	  fprintf (stderr, "Parsed graph:\n---\n");
	  debug_malloc = 0;
	  write_writegraph_file (jlong2adr (nGraph, void *), stderr, 0, 1, 0);
	  debug_malloc = 1;
	  fprintf (stderr, "---\n");
      }
      testJNI->SetLongField (&testJNI, graph,
       testJNI->GetFieldID (&testJNI,
	testJNI->GetObjectClass (&testJNI, graph),
	"nGraph", "J"),
     nGraph);
  } else {
      fprintf (stderr, "** null graph returned\n");
  }
  fprintf (stderr, "** finalizeCaGePipe\n");
  Java_cage_NativeCaGePipe_finalizeCaGePipe (&testJNI, pipe0);
  if (graph != NULL) {
      fprintf (stderr, "** finalizeEmbeddableGraph\n");
      Java_cage_NativeEmbeddableGraph_nFinalize (&testJNI, graph, nGraph);
      free_cmd_vector (jlong2adr (embed, void *));
  }
  fprintf (stderr, "** The End **\n");

  return 0;
}

