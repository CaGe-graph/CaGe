
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <stdio.h>

# include "malloc.h"

# include "dstring.h"

# include "graph.h"

# include "read_graphs.h" 

# include "file_tools.h"

# include "pipe_tools.h"

# include "jni_patches.h"

# include "NativeCaGePipe.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


extern int debug_native_pipe;


/* increase "flowing" graphs counter at "chunks" of the following size */
# define  DEFAULT_GRAPHNO_FIRE_INTERVAL  100


/*
   The "pipe_status" structure holds all information that we
   want to store for each NativeCaGePipe object - i.e. values
   that we want to survive between different native method
   calls from Java - and other bits we want to pass between
   C functions within one native method call (bundled into
   a struct to reduce parameter lists).
   There is one issue - how do we store the adress of one of
   these structures, obtained via malloc, in the Java object?
   The current implementation hopes that the address, as stored
   in a C pointer, can be passed into and out of a jlong without
   change (most importantly, the jlong needs to be big enough
   to hold the pointer value).
*/
struct pipe_status
{
  FILE *file;
  dstring header;
  int format, graphno_fire_interval, advanced1;
  dstring current_graph_encoding, last_graph_encoding;
  jclass class;
  jfieldID status_field, graphno_field, flowing_field, running_field;
  jfieldID reader_field;
  jmethodID graphno_changed, flowing_changed, running_changed;
  jint advance_target;
};
typedef struct pipe_status pipe_status;


/*
   Attempts to read the next graph from the pipe.
   Returns 0 on success.
*/
int read_next (pipe_status *pipestat)
{
  int format;
  format = get_format (pipestat->format);
  if (format > 0 && format <= FORMATS) {
      return reader [format-1]
       (pipestat->file, pipestat->format, & pipestat->current_graph_encoding);
  } else {
      return 1;
  }
}


/*
   These functions provide for the transfer of the pipe status
   between the JNI funtions and the Java object for which they work.
   set_status stores the structure's address in the object.
   get_status retrieves it.
*/

pipe_status *get_status (JNIEnv *env, jobject this)
{
  jclass this_class;
  jfieldID status_field;
  pipe_status *pipestat;
  jlong x;
  setJNIEnv (env);
  this_class = (*env)->GetObjectClass (env, this);
  status_field = (*env)->GetFieldID (env, this_class, "status", "J");
  x = (*env)->GetLongField (env, this, status_field);
  pipestat = jlong2adr (x, pipe_status *);
  if (pipestat == NULL) {
      error_exit ("status information lost", "io/IOException");
      return NULL;
  }
  return pipestat;
}

/*
int check_status (JNIEnv *env, jobject this, pipe_status *last_status)
{
  pipe_status *pipestat;
  jlong x;

  x = (*env)->GetLongField (env, this, last_status->status_field);
  pipestat = jlong2adr (x, pipe_status *);
  return pipestat == last_status;
}
*/


void set_status (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  jclass this_class;
  jfieldID status_field;
  jlong x;
  jlong2adr (x, pipe_status *) = pipestat;
  if (pipestat != NULL) {
      status_field = pipestat->status_field;
  } else {
      this_class = (*env)->GetObjectClass (env, this);
      status_field = (*env)->GetFieldID (env, this_class, "status", "J");
  }
  (*env)->SetLongField (env, this, status_field, x);
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_setAdvanceTarget
  (JNIEnv *env, jobject this, jint advanceTarget)
{
  pipe_status *pipestat;
  pipestat = get_status (env, this);
  pipestat->advance_target = advanceTarget;
}

JNIEXPORT jint JNICALL Java_cage_NativeCaGePipe_getAdvanceTarget
  (JNIEnv *env, jobject this)
{
  pipe_status *pipestat;
  pipestat = get_status (env, this);
  return pipestat->advance_target;
}

jint get_advance_target (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  jint target;
  (*env)->MonitorEnter (env, this);
  target = pipestat->advance_target;
  (*env)->MonitorExit (env, this);
  return target;
}

int short_of_advance_target
  (JNIEnv *env, jobject this, int graphno, pipe_status *pipestat)
{
  int target = (int) get_advance_target (env, this, pipestat);
  return target < 0 ? 1 : graphno < target;
}


int get_graphno (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  return (*env)->GetIntField (env, this, pipestat->graphno_field);
}


void set_graphno (JNIEnv *env, jobject this, pipe_status *pipestat, int n)
{
  (*env)->SetIntField (env, this, pipestat->graphno_field, n);
}


void fire_graphno_changed (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  (*env)->CallVoidMethod (env, this, pipestat->graphno_changed);
}


int get_flowing (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  jboolean flowing;
  (*env)->MonitorEnter (env, this);
  flowing = (*env)->GetBooleanField (env, this, pipestat->flowing_field);
  (*env)->MonitorExit (env, this);
  return flowing == JNI_TRUE;
}

void set_flowing (JNIEnv *env, jobject this, pipe_status *pipestat, int b)
{
  jclass this_class;
  (*env)->MonitorEnter (env, this);
  (*env)->SetBooleanField (env, this, pipestat->flowing_field, (jboolean) b);
  (*env)->MonitorExit (env, this);
  (*env)->CallVoidMethod (env, this, pipestat->flowing_changed);
}


void set_running (JNIEnv *env, jobject this, pipe_status *pipestat, int b)
{
  if (debug_native_pipe) fprintf (stderr, "set running: %d\n", b);
  (*env)->MonitorEnter (env, this);
  (*env)->SetBooleanField (env, this, pipestat->running_field, (jboolean) b);
  (*env)->MonitorExit (env, this);
}

void fire_running_changed (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  (*env)->CallVoidMethod (env, this, pipestat->running_changed);
}

int get_running (JNIEnv *env, jobject this, pipe_status *pipestat)
{
  jboolean running;
  (*env)->MonitorEnter (env, this);
  running = (*env)->GetBooleanField (env, this, pipestat->running_field);
  (*env)->MonitorExit (env, this);
  return running == JNI_TRUE;
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_initCaGePipe
  (JNIEnv *env, jobject this)
{
  jclass this_class;
  pipe_status *pipestat;

  setJNIEnv (env);
  if (debug_native_pipe) fprintf (stderr, "{ initCaGePipe\n");
  if (sizeof (pipe_status *) > sizeof (jlong)) {
      error_exit ("Can't fit a pointer into a jlong. Contact the authors.",
       "lang/InstantiationException");
/*
   If this error occurs, we are not able to fit the address of one
   "pipe_status" structure into the space provided by a "jlong"
   (a long variable in Java, 64 bit).  There doesn't seem to be any
   larger primitive type available in Java.  Therefore, in case of
   this error occuring, we would have to distribute the pointer
   across 2 "jlong"s (or even more) and define and use as many
   status fields in a NativeCaGePipe object.
*/
      return;
  }
  this_class = (*env)->GetObjectClass (env, this);
  pipestat = malloc (sizeof (pipe_status));
  pipestat->graphno_fire_interval = DEFAULT_GRAPHNO_FIRE_INTERVAL;
  pipestat->class = (*env)->NewGlobalRef (env, this_class);
  pipestat->status_field =
   (*env)->GetFieldID (env, this_class, "status", "J");
  pipestat->graphno_field =
   (*env)->GetFieldID (env, this_class, "graphNo", "I");
  pipestat->flowing_field =
   (*env)->GetFieldID (env, this_class, "flowing", "Z");
  pipestat->running_field =
   (*env)->GetFieldID (env, this_class, "running", "Z");
  pipestat->reader_field =
   (*env)->GetFieldID (env, this_class, "reader_fd", "I");
  pipestat->graphno_changed =
   (*env)->GetMethodID (env, this_class, "fireGraphNoChanged", "()V");
  pipestat->flowing_changed =
   (*env)->GetMethodID (env, this_class, "fireFlowingChanged", "()V");
  pipestat->running_changed =
   (*env)->GetMethodID (env, this_class, "fireRunningChanged", "()V");
  pipestat->file = NULL;
  pipestat->current_graph_encoding = new_dstring;
  pipestat->last_graph_encoding = new_dstring;
  pipestat->header = new_dstring;
  set_status (env, this, pipestat);
  if (debug_native_pipe) fprintf (stderr, "} initCaGePipe\n");
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_startCaGePipe
  (JNIEnv *env, jobject this)
{
  pipe_status *pipestat;
  int outfd;
  setJNIEnv (env);
  if (debug_native_pipe) fprintf (stderr, "{ startCaGePipe\n");
  pipestat = get_status (env, this);
  if (pipestat->file != NULL) {
      if (debug_native_pipe) fprintf (stderr, "closing old output file (%d)\n", fileno (pipestat->file));
      fclose (pipestat->file);
      pipestat->file = NULL;
  }
  clear_dstring (& pipestat->header);
  clear_dstring (& pipestat->last_graph_encoding);
  clear_dstring (& pipestat->current_graph_encoding);
  pipestat->format = 0;
  pipestat->advanced1 = 0;
  outfd = (int) (*env)->GetIntField (env, this, pipestat->reader_field);
  if (debug_native_pipe) fprintf (stderr, "outfd = %d\n", outfd);
  if (outfd >= 0) {
      pipestat->file = fdopen (outfd, "rb");
      (*env)->SetIntField (env, this, pipestat->reader_field, (jint) -1);
      if (pipestat->file == NULL) {
          error_exit ("can't open pipe output for reading (fdopen)", "io/IOException");
      }
  } else {
      error_exit ("can't open pipe output for reading (no fd)", "io/IOException");
  }
  if (debug_native_pipe) fprintf (stderr, "} startCaGePipe\n");
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_finalizeCaGePipe
  (JNIEnv *env, jobject this)
{
  pipe_status *pipestat;
  if (debug_native_pipe) fprintf (stderr, "{ finalizeCaGePipe\n");
  pipestat = get_status (env, this);
  if (pipestat->file != NULL) {
      if (debug_native_pipe) fprintf (stderr, "closing output file (%d)\n", fileno (pipestat->file));
      fclose (pipestat->file);
      pipestat->file = NULL;
  }
  clear_dstring (& pipestat->header);
  clear_dstring (& pipestat->current_graph_encoding);
  clear_dstring (& pipestat->last_graph_encoding);
  (*env)->DeleteGlobalRef (env, pipestat->class);
  free (pipestat);
  pipestat = NULL;
  set_status (env, this, pipestat);
  if (debug_native_pipe) fprintf (stderr, "} finalizeCaGePipe\n");
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_nStartAdvancing
  (JNIEnv *env, jobject this)
{
  pipe_status *pipestat;
  int graphno, last_graphno, remainder, c;
  int read_error, reset_flow, graphno_fire_interval;

  setJNIEnv (env);
  pipestat = get_status (env, this);
  if (debug_native_pipe) fprintf (stderr, "{ nStartAdvancing (%d)\n", (int) get_advance_target (env, this, pipestat));
  graphno = get_graphno (env, this, pipestat);
  if (pipestat->file == NULL) {
      if (debug_native_pipe) fprintf (stderr, "no file\n");
      set_flowing (env, this, pipestat, JNI_FALSE);
      set_graphno (env, this, pipestat, graphno);
      fire_graphno_changed (env, this, pipestat);
      return;
  }
  if (! get_running (env, this, pipestat)) return;
  if (! pipestat->format) {
      /* avoid throwing the following exception for an empty input */
      if ((c = getc (pipestat->file)) != EOF) {
          ungetc (c, pipestat->file);
	  /* there is input, we should be able to determine the format */
	  if (start_reading (pipestat->file,
	       & pipestat->format, & pipestat->header)) {
	      error_exit ("generator problem: no output or unknown output format (start_reading)", "io/IOException");
	      return;
	  }
      }
  }

  graphno_fire_interval = pipestat->graphno_fire_interval;
  last_graphno = graphno;
  remainder = -1;
  read_error = 0;
  reset_flow = 1;

  while (short_of_advance_target (env, this, graphno, pipestat))
  {
    if (! pipestat->advanced1) {
	if (read_next (pipestat)) {
	    read_error = 1;
	    break;
	}
	pipestat->advanced1 = 1;
    }
    if (! get_flowing (env, this, pipestat)) {
	if (debug_native_pipe) fprintf (stderr, "out of flow while advancing\n");
	reset_flow = 0;
	break;
    }
    (*env)->MonitorEnter (env, this);
    pipestat->advanced1 = 0;
    clear_dstring (& pipestat->last_graph_encoding);
    pipestat->last_graph_encoding = pipestat->current_graph_encoding;
    pipestat->current_graph_encoding = new_dstring;
    ++graphno;
    set_graphno (env, this, pipestat, graphno);
    (*env)->MonitorExit (env, this);
    if (graphno_fire_interval > 0
     && (remainder = graphno % graphno_fire_interval) == 0) {
	fire_graphno_changed (env, this, pipestat);
    }
  }

  if (debug_native_pipe) fprintf (stderr, "remainder = %d, reset_flow = %d, eof = %d, running = %d\n", remainder, reset_flow, feof (pipestat->file), get_running (env, this, pipestat));
  if (! get_running (env, this, pipestat)) return;
  if (feof (pipestat->file) && ! pipestat->advanced1) {
      set_running (env, this, pipestat, JNI_FALSE);
      if (debug_native_pipe) fprintf (stderr, "closing output file (%d)\n", fileno (pipestat->file));
      fclose (pipestat->file);
      pipestat->file = NULL;
  }
  if (reset_flow) {
      set_flowing (env, this, pipestat, JNI_FALSE);
  }
  if (graphno > last_graphno && ! pipestat->advanced1) {
      fire_graphno_changed (env, this, pipestat);
  }
  if (pipestat->file == NULL) {
      fire_running_changed (env, this, pipestat);
  } else if (read_error) {
      error_exit ("generator problem: graph data doesn't match format (reader)", "io/IOException");
  }
  if (debug_native_pipe)
      fprintf (stderr, "graph no. %d, %d byte(s)\n", graphno, pipestat->current_graph_encoding.length);
  if (debug_native_pipe) fprintf (stderr, "} nAdvanceBy\n");
}


/*
void set_ptr_field
 (JNIEnv *env, jobject obj, jclass clazz, char *field_name, void *ptr)
{
  jfieldID field_id;
  jlong adr;
  jlong2adr (adr, void *) = ptr;
  field_id = (*env)->GetFieldID (env, clazz, field_name, "J");
  (*env)->SetLongField (env, obj, field_id, adr);
}
*/

JNIEXPORT jobject JNICALL Java_cage_NativeCaGePipe_getGraph
  (JNIEnv *env, jobject this)
{
  int format;
  pipe_status *pipestat;
  int is_flowing;
  dstring new_graph_encoding;
  void *new_graph;
  jlong new_graph_adr;
  jclass graph_class;
  jmethodID graph_constructor;
  jobject new_jgraph;

  if (debug_native_pipe) fprintf (stderr, "{ getGraph\n");
  pipestat = get_status (env, this);
  (*env)->MonitorEnter (env, this);
  if ((is_flowing = get_flowing (env, this, pipestat))) {
      new_graph_encoding = new_dstring;
  } else {
      new_graph_encoding = pipestat->last_graph_encoding;
      pipestat->last_graph_encoding = new_dstring;
  }
  if (debug_native_pipe) fprintf (stderr, "got encoding: %d bytes, flowing = %d\n", new_graph_encoding.length, is_flowing);
  (*env)->MonitorExit (env, this);
  if (is_flowing) {
      error_exit ("Don't retrieve a graph while flowing", "io/IOException");
      return NULL;
  } else if (new_graph_encoding.length == 0) {
      clear_dstring (&new_graph_encoding);
      error_exit ("No new graph to hand out (yet)", "io/IOException");
      return NULL;
  }
  format = get_format (pipestat->format);
  if (format <= 0 || format > FORMATS) {
      clear_dstring (&new_graph_encoding);
      error_exit ("generator problem: illegal format after reading", "io/IOException");
      return NULL;
  } else if (parser [format-1]
	     (new_graph_encoding, pipestat->format,
	      (new_graph = get_new_graph()))) {
      if (debug_native_pipe) fprintf (stderr, "parser failed\n");
      clear_graph (new_graph);
      free (new_graph);
      clear_dstring (&new_graph_encoding);
      error_exit ("generator problem: graph data doesn't match format (parser)", "io/IOException");
      return NULL;
  }
  if (debug_native_pipe) {
      fprintf (stderr, "graph parsed:\n");
      write_writegraph_file (new_graph, stderr, 0, 1, 0, 0);
  }

  jlong2adr (new_graph_adr, void *) = new_graph;
  graph_class = (*env)->FindClass (env, "cage/NativeEmbeddableGraph");
  graph_constructor = (*env)->GetMethodID (env, graph_class, "<init>", "(J)V");
  new_jgraph = (*env)->NewObject (env, graph_class, graph_constructor, new_graph_adr);
  if (debug_native_pipe) fprintf (stderr, "} getGraph\n");
  return new_jgraph;
}


JNIEXPORT jboolean JNICALL Java_cage_NativeCaGePipe_wouldBlock
  (JNIEnv *env, jobject this)
{
  pipe_status *pipestat;
  pipestat = get_status (env, this);
  return next_byte (pipestat->file) < 0 ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT void JNICALL Java_cage_NativeCaGePipe_setGraphNoFireInterval
  (JNIEnv *env, jobject this, jint interval)
{
  pipe_status *pipestat;
  pipestat = get_status (env, this);
  pipestat->graphno_fire_interval = interval ? interval :
   DEFAULT_GRAPHNO_FIRE_INTERVAL;
}

