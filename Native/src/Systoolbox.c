
# include <limits.h>
# include <unistd.h>
# include <stdio.h>

# include "jni_patches.h"

# include <jni.h>

# include "jbytes.h"

# include "malloc.h"


/*
JNIEXPORT jbyteArray JNICALL Java_lisken_systoolbox_Systoolbox_nGetCWD
  (JNIEnv *env, jclass this_class)
{
  char *buffer;
  jbyteArray result;
  buffer = malloc (PATH_MAX);
  if (getcwd (buffer, PATH_MAX) == NULL) {
      error_exit ("getcwd wants too much", "lang/OutOfMemoryException");
  }
  result = string2jbytes (env, buffer);
  free (buffer);
  return result;
}
*/

JNIEXPORT jbyteArray JNICALL Java_lisken_systoolbox_Systoolbox_nGetEnv
  (JNIEnv *env, jclass this_class, jbyteArray nameBytes)
{
  jbyteArray result;
  char *name;
  char *value;
  name = jbytes2string (env, nameBytes);
  value = getenv (name);
  result = string2jbytes (env, value);
  free (name);
  return result;
}

