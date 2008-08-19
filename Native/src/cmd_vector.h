
/*
   Functions that support creating pipes from Java.
*/

# include "jni_patches.h"

# include <jni.h>

# include "dstring.h"


struct cmd_vector
{
  int  length;
  char ***cmd;
  char *rundir;
  char *path;
};


extern void *make_cmd_vector (JNIEnv *env, jobjectArray cmds,
 jobjectArray rundir, jobjectArray path);
/*
   "cmds" is a byte[][][] object in Java.  Each Element is a "command".
   A command, in turn, is a byte[][] object.  Each element of a command
   is a byte[] representing a single command argument.  Therefore, all
   segmentation of commands into arguments has to take place elsewhere.
   "make_cmd_vector" creates a structure, via malloc, that represents
   the "cmds" object as a dynamic array of standard *argv[]s in C.
   The structure also stores the byte[] arguments "rundir", representing
   a directory to change into before running the commands, and "path",
   a string which contains a search path to be used in place of the
   PATH environment variable, as a restriction for command execution.
   To enforce that restriction, commands must not contain slashes or
   backslashes if path is set.  The two fields are assumed to be set
   only of they are both non-null and non-empty byte[] arrays (or strings,
   for make_cmd_vector_string).  Calling free_cmd_vector gets rid of
   all memory allocated by a command vector.
*/

extern void *make_cmd_vector_string (char *string, char *rundir, char *path);
/*
   Makes a command vector from a string, which will be written into.
   '|' characters separate commands, spaces separate arguments.
   Use strdup(3) if you want to pass static strings to make_cmd_vector_string,
   for any of the three arguments.
*/

extern void free_cmd_vector (void *cmd_vector);
/*
   Frees any memory allocated within a command vector, and the
   vector address itself.  The rundir and path members are freed
   as well.  Use strdup(3) if you want to pass static strings to
   make_cmd_vector_string.
*/

