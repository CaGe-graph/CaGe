
# include "jni_patches.h"

# include "BufferedFDInputStream.h"

# include "file_tools.h"

# include "error_exit.h"

# include "jbytes.h"

# include <string.h>
# include <errno.h>
# include <stdlib.h>

/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDInputStream_init
  (JNIEnv *env, jobject this, jint fildes)
{
  FILE *file;
  jlong file_adr;
  setJNIEnv (env);
  if ((file = fdopen ((int) fildes, "r")) == NULL) {
      error_exit (strerror (errno), "io/IOException");
  }
  jlong2adr (file_adr, FILE *) = file;
  return file_adr;
}

JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDInputStream_openFile
  (JNIEnv *env, jobject this, jbyteArray filenameBytes)
{
  FILE *file;
  jlong file_adr;
  char *filename;
  jclass this_class;
  jfieldID fd_field;
  setJNIEnv (env);
  filename = jbytes2string (env, filenameBytes);
  file = fopen (filename, "rb");
  free (filename);
  if (file == NULL) {
      error_exit (strerror (errno), "io/IOException");
  } else {
      this_class = (*env)->GetObjectClass (env, this);
      fd_field = (*env)->GetFieldID (env, this_class, "fd", "I");
      (*env)->SetIntField (env, this, fd_field, (jint) fileno (file));
  }
  jlong2adr (file_adr, FILE *) = file;
  return file_adr;
}

JNIEXPORT jint JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nRead
  (JNIEnv *env, jobject this, jlong file_adr)
{
  FILE *file;
  int  c;
  file = jlong2adr (file_adr, FILE *);
  if (file_adr == 0) {
      error_exit (strerror (EBADF), "io/IOException");
      return (jint) -1;
  }
  errno = 0;
  if ((c = getc (file)) == EOF) {
      if (errno > 0) {
	  error_exit (strerror (errno), "io/IOException");
      }
      return (jint) -1;
  }
  return (jint) c;
}

JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nUnread
  (JNIEnv *env, jobject this, jlong file_adr, jint byte)
{
  FILE *file;
  file = jlong2adr (file_adr, FILE *);
  if (ungetc ((int) byte, file) == EOF) {
      error_exit ("unread into BufferedFDInputStream failed", "io/IOException");
  }
}

JNIEXPORT jint JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nNextByte
  (JNIEnv *env, jobject this, jlong file_adr)
{
  FILE *file;
  file = jlong2adr (file_adr, FILE *);
  return next_byte (file);
}

JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nClose
  (JNIEnv *env, jobject this, jlong file_adr)
{
  FILE *file;
  if (file_adr == 0) return;
  file = jlong2adr (file_adr, FILE *);
  fclose (file);
}

