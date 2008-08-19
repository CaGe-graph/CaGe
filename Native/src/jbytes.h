
/*
   Functions that support creating pipes from Java.
*/

# include "jni_patches.h"

# include <jni.h>

# include "dstring.h"


extern char *jbytes2string (JNIEnv *env, jobject bytes);
/*
   "malloc"s and returns a C string with the contents given in the
   Java byte array "bytes", plus a trailing NUL. Must be freed by the user.
*/

extern jbyteArray string2jbytes (JNIEnv *env, char *string);
/*
   Creates a new byte array, fills it with the bytes from "string"
   (excluding the final NUL character) and returns it.
*/

extern jbyteArray dstring2jbytes (JNIEnv *env, dstring string);
/*
   Fills a new byte array with the bytes from dstring.
*/

