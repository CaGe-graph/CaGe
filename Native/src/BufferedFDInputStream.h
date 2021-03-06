/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class lisken_systoolbox_BufferedFDInputStream */

#ifndef _Included_lisken_systoolbox_BufferedFDInputStream
#define _Included_lisken_systoolbox_BufferedFDInputStream
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    init
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDInputStream_init
  (JNIEnv *, jobject, jint);

/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    openFile
 * Signature: ([B)J
 */
JNIEXPORT jlong JNICALL Java_lisken_systoolbox_BufferedFDInputStream_openFile
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    nRead
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nRead
  (JNIEnv *, jobject, jlong);

/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    nUnread
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nUnread
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    nNextByte
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nNextByte
  (JNIEnv *, jobject, jlong);

/*
 * Class:     lisken_systoolbox_BufferedFDInputStream
 * Method:    nClose
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_lisken_systoolbox_BufferedFDInputStream_nClose
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
