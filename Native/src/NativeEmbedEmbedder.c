
# include <stdlib.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <string.h>
# include "jni_patches.h"

# include <jni.h>

# include "error_exit.h"

# include "NativeEmbedEmbedder.h"

# include "read_graphs.h"

# include "graph.h"

# include "cmd_vector.h"

# include "pipe_tools.h"

# include "jbytes.h"


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


extern int debug_native_embedder;


JNIEXPORT jlong JNICALL Java_cage_NativeEmbedEmbedder_nCompileCommands
  (JNIEnv *env, jobject this,
   jobjectArray cmds, jbyteArray rundir, jbyteArray path)
{
  void *cmdv;
  jlong cmd_adr;
  cmdv = make_cmd_vector (env, cmds, rundir, path);
  if (debug_native_embedder) fprintf (stderr, "creating %p\n", cmdv);
  jlong2adr (cmd_adr, void *) = cmdv;
  return cmd_adr;
}


JNIEXPORT void JNICALL Java_cage_NativeEmbedEmbedder_nFinalize
  (JNIEnv *env, jobject this, jlong cmds)
{
  if (debug_native_embedder) fprintf (stderr, "freeing %p\n", jlong2adr (cmds, void *));
  free_cmd_vector (jlong2adr (cmds, void *));
}


char *
getErrFilename (JNIEnv *env, jobject this, jclass this_class, char *dir)
{
  int i, success;
  char *name;
  struct stat buf;
  jfieldID name_field;

  success = 0;
  name = malloc (strlen (dir) + 15);
  (*env)->MonitorEnter (env, this_class);
  for (i = 1; i < 10000; ++i)
  {
    sprintf (name, "%s/embed%.4d.log", dir, i);
    if (stat (name, &buf) != 0) {
        success = 1;
	break;
    }
  }
  if (success) {
      success = errno == ENOENT;
  }
  if (success) {
      int fd = open (name, O_CREAT | O_WRONLY | O_TRUNC, 00644);
      if (fd >= 0) {
          close (fd);
      } else {
          success = 0;
      }
  }
  (*env)->MonitorExit (env, this_class);
  if (! success) {
      free (name);
      name = NULL;
  }
  name_field = (*env)->GetFieldID (env, this_class, "errFilenameBytes", "[B");
  (*env)->SetObjectField (env, this, name_field, string2jbytes (env, name));
  return name;
}

int
finish_errfile (char **err_filename)
{
  struct stat err_stat;
  int output_found = 0;
  if (*err_filename != NULL) {
      stat (*err_filename, &err_stat);
      if (debug_native_embedder) fprintf (stderr, "embedder errfile: %s, size %ld\n", *err_filename, (long)err_stat.st_size);
      if (err_stat.st_size == 0) {
	  unlink (*err_filename);
      } else {
	  output_found = 1;
	  if (debug_native_embedder) {
	      int c;
	      FILE *err_file;
	      err_file = fopen (*err_filename, "rb");
	      if (err_file == NULL) {
		  fprintf (stderr, "can't read errfile contents\n");
	      } else {
		  fprintf (stderr, "errfile contents:\n");
		  while ((c = getc (err_file)) != EOF) {
		    putc (c, stderr);
		  }
		  fclose (err_file);
		  fprintf (stderr, " * end of errfile\n");
	      }
	  }
      }
      free (*err_filename);
      *err_filename = NULL;
  }
  return output_found;
}

void *
embed_graph
 (JNIEnv *env, jobject this, void *graph, void *embed_cmdv, int dimension)
{
  void *new_graph;
  int pipeinfd, pipeoutfd;
  FILE *pipeinfile, *pipeoutfile;
  pid_t embed_pid;
  char *err_filename;
  int new_format, fmt;
  dstring new_encoding, new_header;
  jclass this_class;
  jfieldID pid_field;

  extern int debug_pipe;

  if (debug_native_embedder) fprintf (stderr, "{ embed_graph\n");
  this_class = (*env)->GetObjectClass (env, this);
  err_filename = getErrFilename (env, this, this_class,
   ((struct cmd_vector *) embed_cmdv)->rundir);

  establish_pipe (embed_cmdv, -1, -1,
   NULL, NULL, 0, err_filename == NULL ? "/dev/null" : err_filename,
   &embed_pid, &pipeinfd, &pipeoutfd);
  pid_field = (*env)->GetFieldID (env, this_class, "nEmbedPID", "J");
  (*env)->SetLongField (env, this, pid_field, (jlong) embed_pid);
  pipeinfile = fdopen (pipeinfd, "w");
  pipeoutfile = fdopen (pipeoutfd, "r");
  if (debug_native_embedder) {
      struct cmd_vector *cmdv;
      char **argp, *sep;
      int i;
      cmdv = embed_cmdv;
      sep = "embedder: ";
      for (i = 0; i < cmdv->length; ++i)
      {
	fprintf (stderr, "%s", sep);
	sep = "";
        for (argp = cmdv->cmd [i]; *argp; ++argp)
	{
	  fprintf (stderr, "%s'%s'", sep, *argp);
	  sep = " ";
	}
	sep = " | ";
      }
      fprintf (stderr, "\nWriting graph to embedder (fildes %d)\n", pipeinfd);
  }
  if (debug_native_embedder) {
      write_writegraph_file (graph, stderr, dimension, 1, 0, 0);
  }
  write_writegraph_file (graph, pipeinfile, dimension, 1, 1, 1);
  fclose (pipeinfile);
  if (debug_native_embedder) fprintf (stderr, "Finished writing\n");
  if (debug_native_embedder) fprintf (stderr, "reading embedding (fildes %d)\n", pipeoutfd);
  new_format = 0;
  new_header = new_dstring;
  if (start_reading (pipeoutfile, &new_format, &new_header)) {
      int status;
      if (debug_native_embedder) fprintf (stderr, "start_reading failed\n");
      fclose (pipeoutfile);
      status = wait_for_exit (embed_pid);
      if (debug_native_embedder) fprintf (stderr, "embedder pipe finished, status %d\n", status);
      finish_errfile (&err_filename);
      clear_dstring (&new_header);
      error_exit ("embedder problem: no output or unknown output format (start_reading)", "io/IOException");
      return NULL;
  }
  if (debug_native_embedder) fprintf (stderr, "start_reading successful\n");
  clear_dstring (&new_header);
  new_encoding = new_dstring;
  fmt = get_format (new_format);
  if (reader [fmt-1] (pipeoutfile, new_format, &new_encoding)) {
      if (debug_native_embedder) fprintf (stderr, "reader failed\n");
      clear_dstring (&new_encoding);
      fclose (pipeoutfile);
      wait_for_exit (embed_pid);
      finish_errfile (&err_filename);
      error_exit ("embedder problem: graph data ends early or doesn't match format (reader)", "io/IOException");
      return NULL;
  }
  if (debug_native_embedder) fprintf (stderr, "reader successful\n");
  new_graph = get_new_graph ();
  if (parser [fmt-1] (new_encoding, new_format, new_graph)) {
      if (debug_native_embedder) fprintf (stderr, "parser failed\n");
      clear_dstring (&new_encoding);
      fclose (pipeoutfile);
      wait_for_exit (embed_pid);
      finish_errfile (&err_filename);
      clear_graph (new_graph);
      free (new_graph);
      error_exit ("embedder problem: graph data doesn't match format (parser)", "io/IOException");
      return NULL;
  }
  if (debug_native_embedder) fprintf (stderr, "parser successful\n");
  clear_dstring (&new_encoding);
  fclose (pipeoutfile);
  wait_for_exit (embed_pid);
/*
  if (wait_for_exit (embed_pid) > 0 | finish_errfile (&err_filename)) {
      finish_errfile (&err_filename);
      clear_graph (new_graph);
      free (new_graph);
      error_exit ("embedder reports error (exit value or stderr output)", "io/IOException");
      return NULL;
  }
*/
  finish_errfile (&err_filename);
  if (debug_native_embedder) fprintf (stderr, "} embed_graph\n");
  return new_graph;
}


JNIEXPORT void JNICALL Java_cage_NativeEmbedEmbedder_nEmbed2D
  (JNIEnv *env, jobject this, jlong nGraph, jlong embed2D)
{
  void *embed2d_cmdv, *graph, *new_graph;
  if (debug_native_embedder) fprintf (stderr, "{ nEmbed2D\n");
  if (embed2D != 0) {
      setJNIEnv (env);
      graph = jlong2adr (nGraph, void *);
      embed2d_cmdv = jlong2adr (embed2D, void *);
      new_graph = embed_graph (env, this, graph, embed2d_cmdv, 2);
      if (new_graph != NULL) {
	  transplant_2d_coordinates (graph, new_graph);
	  clear_graph (new_graph);
	  free (new_graph);
      }
  } else if (debug_native_embedder) {
      if (debug_native_embedder) fprintf (stderr, "embedding skipped.\n");
  }
  if (debug_native_embedder) fprintf (stderr, "} nEmbed2D\n");
}


JNIEXPORT void JNICALL Java_cage_NativeEmbedEmbedder_nEmbed3D
  (JNIEnv *env, jobject this,
   jlong nGraph, jlong embed3DNew, jlong embed3DRefine)
{
  jlong embed3D;
  void *embed3d_cmdv, *graph, *new_graph;
  if (debug_native_embedder) fprintf (stderr, "{ nEmbed3D\n");
  setJNIEnv (env);
  graph = jlong2adr (nGraph, void *);
  if (embed3DNew != embed3DRefine) {
      if (debug_native_embedder) fprintf (stderr, "has 3D coordinates? %s\n", has_3d_coordinates (graph) ? "yes" : "no");
      embed3D = has_3d_coordinates (graph) ? embed3DRefine : embed3DNew;
  } else {
      embed3D = embed3DNew;
  }
  if (embed3D != 0) {
      embed3d_cmdv = jlong2adr (embed3D, void *);
      new_graph = embed_graph (env, this, graph, embed3d_cmdv, 3);
      if (new_graph != NULL) {
	  transplant_3d_coordinates (graph, new_graph);
	  clear_graph (new_graph);
	  free (new_graph);
      }
  } else if (debug_native_embedder) {
      if (debug_native_embedder) fprintf (stderr, "embedding skipped.\n");
  }
  if (debug_native_embedder) fprintf (stderr, "} nEmbed3D\n");
}


JNIEXPORT void JNICALL Java_cage_NativeEmbedEmbedder_nStop
  (JNIEnv *env, jobject this, jlong pid)
{
  if (debug_native_embedder) fprintf (stderr, "stopping embedder pipe %d\n", (int) pid);
  kill ((pid_t) pid, SIGTERM);
}

