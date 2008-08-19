
# include "jni_patches.h"

# include "BufferedFDOutputStream.h"

# include "error_exit.h"

# include "jbytes.h"

# include <string.h>
# include <errno.h>


/*
   This macro takes an lvalue of type jlong and converts it
   into an lvalue of a given pointer type.
*/
# define  jlong2adr(jlong, ptrtype)  (* (ptrtype *) &(jlong))


JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDOutputStream_init
  (JNIEnv *env, jobject this, jint fildes)
{
  FILE *file;
  jlong file_adr;
  setJNIEnv (env);
  if ((file = fdopen ((int) fildes, "w")) == NULL) {
      error_exit (strerror (errno), "io/IOException");
  }
  jlong2adr (file_adr, FILE *) = file;
  return file_adr;
}

JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDOutputStream_openFile
  (JNIEnv *env, jobject this, jbyteArray filenameBytes, jboolean append)
{
  FILE *file;
  jlong file_adr;
  char *filename;
  jclass this_class;
  jfieldID fd_field;
  setJNIEnv (env);
  filename = jbytes2string (env, filenameBytes);
  file = fopen (filename, append == JNI_TRUE ? "ab" : "wb");
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

JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDOutputStream_nWrite
  (JNIEnv *env, jobject this, jlong file_adr, jint byte)
{
  FILE *file;
  int  c;
  if (file_adr == 0) {
      error_exit (strerror (EBADF), "io/IOException");
      return;
  }
  file = jlong2adr (file_adr, FILE *);
  if (putc ((int) byte, file) == EOF) {
      error_exit (strerror (errno), "io/IOException");
  }
}

JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDOutputStream_nFlush
  (JNIEnv *env, jobject this, jlong file_adr)
{
  FILE *file;
  file = jlong2adr (file_adr, FILE *);
  fflush (file);
}

JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDOutputStream_nClose
  (JNIEnv *env, jobject this, jlong file_adr)
{
  FILE *file;
  if (file_adr == 0) return;
  file = jlong2adr (file_adr, FILE *);
  fclose (file);
}

