
# include <string.h>

# include "jni_patches.h"

# include <jni.h>

# include "dstring.h"

# include "malloc.h"


char *
jbytes2string (JNIEnv *env, jobject bytes)
{
  char *result;
  int  length;
  char *elements;
  if (bytes == NULL) return NULL;
  length = (*env)->GetArrayLength (env, bytes);
  result = malloc (length + 1);
  elements = (char*)(*env)->GetByteArrayElements (env, bytes, NULL);
  strncpy (result, elements, length);
  strcpy (& result[length], "");
  (*env)->ReleaseByteArrayElements (env, bytes, (signed char *)elements, JNI_ABORT);
  return result;
}

jbyteArray
mem2jbytes (JNIEnv *env, char *base, int length)
{
  jbyteArray result;
  char *elements;
  if (base == NULL) return NULL;
  result = (*env)->NewByteArray (env, length);
  elements = (char*)(*env)->GetByteArrayElements (env, result, NULL);
  memcpy (elements, base, length);
  (*env)->ReleaseByteArrayElements (env, result,(signed char *)elements, JNI_COMMIT);
  return result;
}

jbyteArray
string2jbytes (JNIEnv *env, char *string)
{
  return mem2jbytes (env, string, string == NULL ? 0 : strlen (string));
}

jbyteArray
dstring2jbytes (JNIEnv *env, dstring string)
{
  return mem2jbytes (env, string.base, string.length);
}

