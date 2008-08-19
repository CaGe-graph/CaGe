
# include "error_exit.h"

# include "cmd_vector.h"

# include "jbytes.h"

# include "pipe_tools.h"

# include "jni_patches.h"

# include "Pipe.h"

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>


extern int debug_pipe;


JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_startPipe
  (JNIEnv *env, jobject this, jobjectArray cmds,
   jint i_fd, jint o_fd,
   jbyteArray infilename,
   jbyteArray outfilename, jboolean out_append,
   jbyteArray errfilename)
{
  jclass this_class;
  jfieldID field_id, reader_field_id, writer_field_id;
  jbyteArray rundir, path;
  void *cmdv;
  pid_t pipe_pid;
  int writer_fd, reader_fd;

  setJNIEnv (env);
  this_class = (*env)->GetObjectClass (env, this);
  field_id = (*env)->GetFieldID (env, this_class, "runDir", "[B");
  rundir = (*env)->GetObjectField (env, this, field_id);
  field_id = (*env)->GetFieldID (env, this_class, "path", "[B");
  path = (*env)->GetObjectField (env, this, field_id);

  writer_field_id = (*env)->GetFieldID (env, this_class, "writer_fd", "I");
  reader_field_id = (*env)->GetFieldID (env, this_class, "reader_fd", "I");
  writer_fd = (int) (*env)->GetIntField (env, this, writer_field_id);
  close (writer_fd);
  reader_fd = (int) (*env)->GetIntField (env, this, reader_field_id);
  close (reader_fd);

  cmdv = make_cmd_vector (env, cmds, rundir, path);
  establish_pipe (cmdv,
   i_fd, o_fd,
   jbytes2string (env, infilename),
   jbytes2string (env, outfilename), out_append == JNI_TRUE,
   jbytes2string (env, errfilename),
   &pipe_pid, &writer_fd, &reader_fd);
  free_cmd_vector (cmdv);

  if (debug_pipe) fprintf (stderr, "establish_pipe returned PID %d, writer %d, reader %d\n", pipe_pid, writer_fd, reader_fd);
  field_id = (*env)->GetFieldID (env, this_class, "pipe_pid", "I");
  (*env)->SetIntField (env, this, field_id, (jint) pipe_pid);
  (*env)->SetIntField (env, this, writer_field_id, (jint) writer_fd);
  (*env)->SetIntField (env, this, reader_field_id, (jint) reader_fd);
}


JNIEXPORT jint JNICALL Java_lisken_systoolbox_Pipe_checkForExit
  (JNIEnv *env, jobject this)
{
  int status;
  jclass this_class;
  jfieldID field_id;
  pid_t pipe_pid;

  this_class = (*env)->GetObjectClass (env, this);
  field_id = (*env)->GetFieldID (env, this_class, "pipe_pid", "I");
  pipe_pid = (*env)->GetIntField (env, this, field_id);
  status = check_for_exit (pipe_pid);
  collect_any_children ();
  if (status >= 0) {
      pipe_pid = -1;
      (*env)->SetIntField (env, this, field_id, (jint) pipe_pid);
  }
  return (jint) status;
}


JNIEXPORT jint JNICALL Java_lisken_systoolbox_Pipe_waitForExit
  (JNIEnv *env, jobject this)
{
  int status;
  jclass this_class;
  jfieldID field_id;
  pid_t pipe_pid;

  this_class = (*env)->GetObjectClass (env, this);
  field_id = (*env)->GetFieldID (env, this_class, "pipe_pid", "I");
  pipe_pid = (*env)->GetIntField (env, this, field_id);
  status = wait_for_exit (pipe_pid);
  collect_any_children ();
  pipe_pid = -1;
  (*env)->SetIntField (env, this, field_id, (jint) pipe_pid);
  return (jint) status;
}


void close_fd
 (JNIEnv *env, jobject this, jclass this_class, char *fd_field_name)
{
  jfieldID field_id;
  int fd;
  field_id = (*env)->GetFieldID (env, this_class, fd_field_name, "I");
  fd = (int) (*env)->GetIntField (env, this, field_id);
  close (fd);
  (*env)->SetIntField (env, this, field_id, (jint) -1);
}


void stop_pipe
 (JNIEnv *env, jobject this, jclass this_class)
{
  jfieldID field_id;
  pid_t pipe_pid;
  close_fd (env, this, this_class, "writer_fd");
  field_id = (*env)->GetFieldID (env, this_class, "pipe_pid", "I");
  pipe_pid = (*env)->GetIntField (env, this, field_id);
  if (pipe_pid > 0) {
      if (debug_pipe) fprintf (stderr, "Stopping pipe process %d\n", pipe_pid);
      kill (pipe_pid, SIGTERM);
      pipe_pid = -1;
      (*env)->SetIntField (env, this, field_id, (jint) pipe_pid);
  }
}


JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_stop
  (JNIEnv *env, jobject this)
{
  jclass this_class;
  this_class = (*env)->GetObjectClass (env, this);
  stop_pipe (env, this, this_class);
}


JNIEXPORT void JNICALL Java_lisken_systoolbox_Pipe_finalizePipe
  (JNIEnv *env, jobject this)
{
  jclass this_class;
  this_class = (*env)->GetObjectClass (env, this);
  stop_pipe (env, this, this_class);
  close_fd (env, this, this_class, "reader_fd");
  collect_any_children ();
}

