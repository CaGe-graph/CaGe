
# include <errno.h>
# include <unistd.h>
# include <string.h>
# include "jni_patches.h"

# include <jni.h>

# include "malloc.h"


JNIEnv *env;


void
error_exit (char *msg, char *error_name)
{
  jclass error_class;
  char *class_name;
  if (error_name == NULL) error_name = "lang/Exception";
  class_name = malloc (strlen (error_name) + 6);
  sprintf (class_name, "java/%s", error_name);
  error_class = (*env)->FindClass (env, class_name);
  free (class_name);
  (*env)->ThrowNew (env, error_class, msg);
}

void
error_exit_subproc (char *msg, char *error_name)
{
  fprintf (stderr, "fatal error (%s): %s\n", error_name, msg);
  if (errno > 0) {
      fprintf (stderr, "errno is %d: %s\n", errno, strerror (errno));
  }
  exit (2);
}

void
setJNIEnv (JNIEnv *e)
{
  env = e;
}

